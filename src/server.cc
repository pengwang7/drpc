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
#include "channel.hpp"
#include "rpc_channel.hpp"
#include "server.hpp"

namespace drpc {

Server::Server() {
    DTRACE("Create Server.");
}

Server::~Server() {
    DTRACE("Destroy Server.");
}

bool Server::Start(Options* options) {
    options_ = options;

    Init();

    status_.store(kStarting);

    group_->Run();

    if (!listener_->Start()) {
        return false;
    }

    status_.store(kRunning);

    event_loop_->Run();

    return true;
}

bool Server::Stop() {
    DASSERT(status_ == kRunning, "Server status error.");

    status_.store(kStopping);
    substatus_.store(kStoppingListener);

    event_loop_->RunInLoop(std::bind(&Server::StopInLoop, this, DoneCallback()));

    return true;
}

bool Server::AddService(google::protobuf::Service* service) {
    auto* desc = service->GetDescriptor();
    if (services_.find(desc->full_name()) != services_.end()) {
        DWARNING("AddService failed, already had this service({}).", desc->full_name());
        return false;
    }

    services_.insert({desc->full_name(), service});

    DDEBUG("Add service({}) to RPC success.", desc->full_name());

    return true;
}

void Server::Init() {
    DASSERT(status_ == kNull, "Server status error.");

    next_channel_id_ = 0;

    endpoint_.reset(new Endpoint(options_->address.c_str(), options_->port));

    event_loop_.reset(new EventLoop());

    listener_.reset(new TcpNetworkListener(event_loop_.get(), endpoint_.get()));

    listener_->SetNewConnCallback(
        std::bind(&Server::HandleNewChannel,
                  this,
                  std::placeholders::_1,
                  std::placeholders::_2));

    if (options_->threads <= 0) {
        options_->threads = get_nprocs();
    }

    group_.reset(new EventLoopGroup(options_->threads, "group1"));

    status_.store(kInitialized);
}

void Server::StopLoopGroup() {
    DASSERT(event_loop_->IsConsistent(), "StopLoopGroup error.");
    DASSERT(IsStopping(), "Server status error.");

    substatus_.store(kStoppingThreadPool);

    group_->Stop();
    group_->Wait();
    group_.reset();

    substatus_.store(kSubStatusNull);
}

void Server::StopInLoop(DoneCallback cb) {
    DASSERT(event_loop_->IsConsistent(), "StopInLoop error.");

    listener_->Stop();
    listener_.reset();

    if (channels_.empty()) {
        StopLoopGroup();
        event_loop_->Stop();
        status_.store(kStopped);
        return;
    }

    for (auto& c : channels_) {
        if (c.second->IsConnected()) {
            DTRACE("Close channel id={}, fd={}.", c.second->id(), c.second->fd());
            c.second->Close();
        }
    }
}

void Server::RemoveChannel(const ChannelPtr& channel) {
    DTRACE("The fd={}, channels_.size()={}.", channel->fd(), channels_.size());

    auto fn = [this, channel]() -> void {
        DASSERT(event_loop_->IsConsistent(), "RemoveChannel error.");
        channels_.erase(channel->id());

        if (IsStopping() && channels_.empty()) {
            StopLoopGroup();
            event_loop_->Stop();
            status_.store(kStopped);
        }
    };

    event_loop_->RunInLoop(fn);
}

void Server::HandleNewChannel(int fd, const std::string& raddr) {
    DASSERT(event_loop_->IsConsistent(), "HandleNewChannel error.");

    if (IsStopping()) {
        DWARNING("The server is at stopping status. Discard fd={}, remote_addr={}.", fd, raddr);
        close(fd);
        return;
    }

    DASSERT(IsRunning(), "Server status error.");

    EventLoop* io_event_loop = GetNextEventLoop();

    ++ next_channel_id_;

    ChannelPtr channel(new Channel(io_event_loop, fd, next_channel_id_));
    channel->SetChannelCallback(std::bind(&Server::OnChannel, this, std::placeholders::_1));
    channel->SetCloseCallback(std::bind(&Server::RemoveChannel, this, std::placeholders::_1));

    channels_[channel->id()] = channel;

    io_event_loop->RunInLoop(std::bind(&Channel::Attach, channel));
}

void Server::OnChannel(const ChannelPtr& channel) {
    if (channel->IsConnected()) {
        RpcChannelPtr rpc_channel(new RpcChannel(channel));
        rpc_channel->SetServiceMap(&services_);
        channel->SetMessageCallback(
            std::bind(&RpcChannel::OnRpcMessage,
                      get_raw_pointer(rpc_channel), std::placeholders::_1,
                      std::placeholders::_2));
        channel->SetContext(0, rpc_channel);
    } else {
        channel->SetContext(0, RpcChannelPtr());
    }
}

EventLoop* Server::GetNextEventLoop() {
    return group_->GetNextEventLoop();
}

} // namespace drpc
