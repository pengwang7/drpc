/*
 * MIT License
 *
 * Copyright (c) 2020 pengwang7
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include <google/protobuf/service.h>
#include <google/protobuf/descriptor.h>

#include <sys/sysinfo.h>

#include "logger.h"
#include "async_socket.h"
#include "channel.h"
//#include "timing_wheel.h"
#include "rpc_channel.h"
#include "event_loop.h"
#include "server.h"

namespace drpc {

Server::Server() {
    DTRACE("Create Server: %p.", this);
}

Server::~Server() {
    DTRACE("Destroy Server: %p.", this);
}

bool Server::Start(ServerOptions* options) {
    if (!DoInit(options)) {
        DERROR("Server start error.");
        return false;
    }

    status_.store(kStarting);

    group_->Run();

    if (!network_listener_->Start()) {
        DERROR("Server start error.");
        return false;
    }

    status_.store(kRunning);

    listener_event_loop_->Run();

    return true;
}

bool Server::Stop() {
    DASSERT(status_ == kRunning, "Server status error.");

    status_.store(kStopping);

    network_listener_->Stop();

    group_->Stop();
    group_->Wait();

    listener_event_loop_->Stop();

    status_.store(kStopped);

    return true;
}

bool Server::AddService(::google::protobuf::Service* service) {
    const google::protobuf::ServiceDescriptor* desc = service->GetDescriptor();
    if (service_map_.find(desc->full_name()) != service_map_.end()) {
        DWARNING("AddService failed, already had same name.");
        return false;
    }

    service_map_.insert({desc->full_name(), service});

    DDEBUG("The service add to rpc success(%s).", desc->full_name().c_str());

    return true;
}

ServerOptions* Server::GetServerOptions() {
    return options_;
}

bool Server::DoInit(ServerOptions* options) {
    status_.store(kInitializing);

    if (!options) {
        DERROR("Server init failed, options is nil.");
        return false;
    }

    options_ = options;

    next_channel_id_ = 0;

    if (options_->threads <= 0) {
        options_->threads = get_nprocs();
    }

    group_.reset(new EventLoopGroup(options_, "event-loop-group"));

    DASSERT(group_, "Server init failed, group is nil.");

    listener_event_loop_.reset(new EventLoop());

    DASSERT(listener_event_loop_, "Server init failed, listener event loop is nil.");

    if (options_->listener_mode != ListenerMode::LISTEN_ETH_NET) {
        network_listener_.reset(
            new BasicNetworkListener(listener_event_loop_.get(), options_));
    } else {
        network_listener_.reset(
            new TcpNetworkListener(listener_event_loop_.get(), options_));
    }

    DASSERT(network_listener_, "Server init failed, network listener is nil.");

    network_listener_->SetNewConnCallback(std::bind(&Server::BuildChannel,
                                          this, std::placeholders::_1, std::placeholders::_2));

    status_.store(kInitialized);

    return true;
}

void Server::BuildChannel(int fd, std::string& peer_addr) {
    DASSERT(listener_event_loop_->IsConsistent(), "Listener event loop check failed.");

    if (IsStopping()) {
        DWARNING("The server is at stopping status, discard socket fd=%d", fd);
        close(fd);
        return;
    }

    DTRACE("BuildChannel fd: %d, peer address: %s.", fd, peer_addr.c_str());

    EventLoop* event_loop = group_->event_loop();
    DASSERT(event_loop, "BuildChannel error.");

    AsyncSocket* ast = new AsyncSocket(event_loop, fd, kReadEvent);
    if (!ast) {
        DERROR("AsyncSocket new failed.");
        close(fd);
        return;
    }

    channel_ptr chan(new Channel(ast, ++ next_channel_id_));
    if (!chan) {
        DERROR("BuildChannel error.");
        close(fd);
        return;
    }

    chan->Init();

    chan->SetNewChannelCallback(std::bind(&Server::OnNewChannel,
                                          this, std::placeholders::_1));
    chan->SetCloseCallback(std::bind(&Server::OnCloseChannel,
                                     this, std::placeholders::_1));
    chan->SetTimedoutCallback(std::bind(&Server::OnTimeoutChannel,
                                        this, std::placeholders::_1));

    event_loop->RunInLoop(std::bind(&Channel::Attach, chan));
}

void Server::OnNewChannel(const channel_ptr& chan) {
    DTRACE("OnNewChannel csid: %d.", chan->csid());

    rpc_channel_ptr rpc_chan(new RpcChannel(chan, &service_map_));
    rpc_chan->SetAnyContext(this);

//    if (options_->enable_check_timeout &&
//        options_->timeout > 0 && timing_wheel_) {
//        rpc_chan->SetRefreshCallback(std::bind(&Server::OnRefreshChannel, this, std::placeholders::_1));
//
//        EntryPtr entry(new Entry(chan));
//        WeakEntryPtr weak_entry(entry);
//
//        std::lock_guard<std::mutex> lock(mutex_);
//        timing_wheel_->push_back(entry);
//
//        // Save the weak entry to channel any context.
//        chan->SetAnyContext(weak_entry);
//    }

    chan->SetRecvMessageCallback(std::bind(&RpcChannel::OnRpcMessage,
            rpc_chan.get(), std::placeholders::_1, std::placeholders::_2));

    std::lock_guard<std::mutex> guard(hash_mutex_);
    hash_table_[chan->csid()] = rpc_chan;
}

void Server::OnRefreshChannel(const channel_ptr& chan) {
//    DASSERT(!chan->GetAnyContext().empty(), "The channel any context is nil.");
//
//    if (options_->enable_check_timeout &&
//        options_->timeout > 0 && timing_wheel_) {
//        // Don't create a new entry, if we do this(create a new entry),
//        // the channel will voluntarily close, regardless of whether there
//        // is a message refresh.
//        WeakEntryPtr weak_entry(any_cast<WeakEntryPtr>(chan->GetAnyContext()));
//        EntryPtr entry(weak_entry);
//
//        if (entry) {
//            std::lock_guard<std::mutex> lock(mutex_);
//            timing_wheel_->push_back(entry);
//        } else {
//            DWARNING("OnRefreshChannel entry is nil.");
//        }
//    }
}

void Server::OnCloseChannel(const channel_ptr& chan) {
    DTRACE("The rpc channel(%d) removed from container.", chan->csid());

    {
        std::lock_guard<std::mutex> lock(hash_mutex_);
        hash_table_.erase(chan->csid());
    }
}

void Server::OnTimeoutChannel(const channel_ptr& chan) {
    //OnCloseChannel(chan);
}

} // namespace drpc
