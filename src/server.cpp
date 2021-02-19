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

#include "logger.hpp"
#include "async_socket.hpp"
#include "listener.hpp"
#include "channel.hpp"
#include "timing_wheel.hpp"
#include "rpc_channel.hpp"
#include "event_loop.hpp"
#include "server.hpp"

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

    if (options_->enable_check_timeout) {
        listener_event_loop_->StartChannelTimeoutCheck(1, [&]() {
            timing_wheel_->push_bucket(Bucket());
            DDEBUG("Push empty bucket.");
        });
    }

    group_->Run();

    if (!listener_->Start()) {
        DERROR("Server start error.");
        return false;
    }

    listener_event_loop_->Run();

    return true;
}

bool Server::Stop() {
    listener_->Stop();

    group_->Stop();

    listener_event_loop_->Stop();

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
    if (!options) {
        DERROR("Server init failed, options is nil.");
        return false;
    }

    options_ = options;

    if (options_->threads <= 0) {
        options_->threads = get_nprocs();
    }

    chan_table_.reset(new ChannelHashTable);
    DASSERT(chan_table_, "Server init failed, channel table is nil.");

    group_.reset(new EventLoopGroup(options_, "event-loop-group"));
    if (!group_) {
        DERROR("Server init failed, group is nil.");
        return false;
    }

    listener_event_loop_.reset(new EventLoop(options_->fd_limits));
    if (!listener_event_loop_) {
        DERROR("Server init failed, listener event loop is nil.");
        return false;
    }

    listener_.reset(new Listener(listener_event_loop_.get(), options_));
    if (!listener_) {
        DERROR("Server init failed, listener is nil.");
        return false;
    }

    listener_->SetNewConnCallback(std::bind(&Server::BuildChannel,
                this, std::placeholders::_1, std::placeholders::_2));

    if (options_->enable_check_timeout || options_->timeout <= 0) {
        timing_wheel_.reset(new TimingWheel(options_->timeout, 1));
        if (!timing_wheel_) {
            DERROR("Server init failed, timing wheel is nil.");
            return false;
        }
    } else {
        DDEBUG("Server not enable channel timeout check.");
    }

    return true;
}

void Server::BuildChannel(int fd, std::string& peer_addr) {
    DTRACE("BuildChannel fd: %d, peer address: %s.", fd, peer_addr.c_str());

    EventLoop* event_loop = group_->event_loop();
    DASSERT(event_loop, "BuildChannel error.");

    AsyncSocket* ast = new AsyncSocket(event_loop, fd, kNoneEvent);
    if (!ast) {
        DERROR("AsyncSocket new failed.");
        return;
    }

    channel_ptr chan(new Channel(ast));
    if (!chan) {
        DERROR("BuildChannel error.");
        return;
    }

    chan->Init();

    chan->SetNewChannelCallback(std::bind(&Server::OnNewChannel,
                this, std::placeholders::_1));
    chan->SetCloseCallback(std::bind(&Server::OnCloseChannel,
                this, std::placeholders::_1));
    chan->SetTimedoutCallback(std::bind(&Server::OnTimeoutChannel,
            this, std::placeholders::_1));
//    chan->SetRecvMessageCallback(std::bind(&Server::OnMessage,
//                this, std::placeholders::_1, std::placeholders::_2));

    event_loop->SendToQueue(std::bind(&Channel::Attach, chan));
}

void Server::OnNewChannel(const channel_ptr& chan) {
    DTRACE("OnNewChannel csid: %s.", chan->csid().c_str());

    std::shared_ptr<RpcChannel> rpc_chan(new RpcChannel(chan, &service_map_));
    rpc_chan->SetRefreshCallback(std::bind(&Server::OnRefreshChannel, this, std::placeholders::_1));
    rpc_chan->SetAnyContext(this);

    chan->SetRecvMessageCallback(std::bind(&RpcChannel::OnRpcMessage,
                rpc_chan.get(), std::placeholders::_1, std::placeholders::_2));
    chan->SetAnyContext(rpc_chan);
}

void Server::OnRefreshChannel(const channel_ptr& chan) {
    DTRACE("OnRefreshChannel...........");
    sleep(2);
    //WeakEntryPtr weak_entry(any_cast<WeakEntryPtr>(chan->GetAnyContext()));
    EntryPtr entry(new Entry(chan));

    if (entry) {
        std::lock_guard<std::mutex> lock(mutex_);
        timing_wheel_->push_back(entry);
    } else {
        DWARNING("OnRefreshChannel entry is nil.");
    }
}

void Server::OnCloseChannel(const channel_ptr& chan) {

}

void Server::OnTimeoutChannel(const channel_ptr& chan) {

}

void Server::OnMessage(const channel_ptr& chan, Buffer& buffer) {

}

} /* end namespace drpc */
