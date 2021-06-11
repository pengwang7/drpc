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

#include "logger.hpp"
#include "socket.hpp"
#include "udp_server.hpp"

namespace drpc {

UdpServer::UdpServer(EventLoop* event_loop, Endpoint* endpoint, std::string id)
    : event_loop_(event_loop), unique_id_(id) {
    int fd = CreateUDPServer(endpoint);
    DASSERT(fd != -1, "UdpServer create socket failed.");

    async_socket_.reset(new AsyncSocket(event_loop_, fd, kNoneEvent));
    DASSERT(async_socket_ != nullptr, "UdpServer async_socket_ is nil.");
}

UdpServer::~UdpServer() {
    DDEBUG("UdpServer destroy.");
}

void UdpServer::Start() {
    async_socket_->ModifyIOEvents(true, false);
    if (async_socket_->IsNone()) {
        DERROR("UdpServer attach to event loop failed, events is none.");
        return;
    }

    DASSERT(async_socket_->IsAttached(), "UdpServer socket not attach.");
}

void UdpServer::Stop() {
    auto fn = [&]() {
        async_socket_->DisableAllIOEvents();
        async_socket_->Close();
        DDEBUG("Stop UdpServer id={}.", unique_id_.c_str());
    };

    event_loop_->QueueInLoop(fn);
}

UdpServerManager::UdpServerManager()
    : event_loop_(new EventLoop()) {
    DASSERT(event_loop_ != nullptr, "event_loop_ is nil.");
}

UdpServerManager::~UdpServerManager() {
    delete event_loop_;
    event_loop_ = nullptr;
}

void UdpServerManager::Start() {
    if (loop_thread_ != nullptr) {
        return;
    }

    loop_thread_.reset(new std::thread(std::bind(&UdpServerManager::Cycle, this)));
}

void UdpServerManager::Stop() {
    event_loop_->Stop();
}

void UdpServerManager::AddUdpServer(UdpServer* udp_server) {
    if (!udp_server) {
        DERROR("AddUdpServer udp_server is nil.");
        return;
    }

    auto fn = [&, udp_server]() {
        DDEBUG("Add it={}.", udp_server->unique_id());
        udp_server->Start();
        udp_servers_.push_back(udp_server);
    };

    event_loop_->RunInLoop(fn);
}

void UdpServerManager::DelUdpServer(UdpServer* udp_server) {
    if (!udp_server) {
        return;
    }

    DDEBUG("Call DelUdpServer.");

    auto fn = [&]() {
        udp_server->Stop();

        DDEBUG("Del id={}.", udp_server->unique_id());

        auto iter = std::find(udp_servers_.begin(), udp_servers_.end(), udp_server);
        if (iter == udp_servers_.end()) {
            return;
        }

        delete *iter;
        udp_servers_.erase(iter);
    };

    event_loop_->RunInLoop(fn);
}

UdpServer* UdpServerManager::FindUdpServerById(std::string id) {
    UdpServer* result = nullptr;
    for (auto iter = udp_servers_.begin(); iter != udp_servers_.end(); ++ iter) {
        if ((*iter)->unique_id() == id) {
            result = *iter;
            break;
        }
    }

    return result;
}

void UdpServerManager::Cycle() {
    DDEBUG("before runing");
    event_loop_->Run();
    DDEBUG("after running");
}

} // namespace drpc
