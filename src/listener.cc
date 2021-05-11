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
#include "async_socket.hpp"
#include "event_loop.hpp"
#include "listener.hpp"

namespace drpc {

NetworkListener::NetworkListener(
    EventLoop* event_loop, Endpoint* endpoint)
    : event_loop_(event_loop), endpoint_(endpoint) {
    DTRACE("Construt NetworkListener.");
}

NetworkListener::~NetworkListener() {
    DTRACE("Destruct NetworkListener.");
}

void NetworkListener::SetListenerSocketOptions(int fd) {
    SetSocketOptions(fd, SO_REUSEADDR);
    SetSocketOptions(fd, TCP_DEFER_ACCEPT);
    SetSocketOptions(fd, SO_REUSEPORT);
}

void NetworkListener::AcceptHandle() {
    DASSERT(event_loop_->IsConsistent(), "NetworkListener error.");

    std::string peer_addr;

    int fd = AcceptSocket(socket_->fd(), peer_addr);
    if (fd < 0) {
        if (errno != EAGAIN && errno != EINTR) {
            DERROR("NetworkListener accept failed: {}.", std::strerror(errno));
        }
        return;
    }

    if (!new_conn_cb_) {
        close(fd);
        DERROR("NetworkListener new conn cb is nil.");
        return;
    }

    new_conn_cb_(fd, peer_addr);
}

BasicNetworkListener::BasicNetworkListener(
    EventLoop* event_loop, Endpoint* endpoint)
    : NetworkListener(event_loop, endpoint) {
    DTRACE("Construt BasicNetworkListener.");
}

BasicNetworkListener::~BasicNetworkListener() {
    DTRACE("Destruct BasicNetworkListener.");
}

bool BasicNetworkListener::Start() {
    int fd = CreateSocket(AF_UNIX, true);
    if (fd < 0) {
        return false;
    }

    SetListenerSocketOptions(fd);

    DASSERT(BindSocket(fd, std::string("/tmp/none")), "");
    DASSERT(ListenSocket(fd, 128), "");

    socket_.reset(new AsyncSocket(event_loop_, fd, kReadEvent));
    if (!socket_) {
        return false;
    }

    socket_->SetReadCallback(std::bind(&BasicNetworkListener::AcceptHandle, this));

    event_loop_->RunInLoop(std::bind(&AsyncSocket::Attach, socket_.get()));

    return true;
}

bool BasicNetworkListener::Stop() {
    DASSERT(event_loop_->IsConsistent(), "Stop error.");

    socket_->DisableAllIOEvents();
    socket_->Close();

    return true;
}

TcpNetworkListener::TcpNetworkListener(
    EventLoop* event_loop, Endpoint* endpoint)
    : NetworkListener(event_loop, endpoint) {
    DTRACE("Construt TcpNetworkListener.");
}

TcpNetworkListener::~TcpNetworkListener() {
    DTRACE("Destruct TcpNetworkListener.");
}

bool TcpNetworkListener::Start() {
    DASSERT(endpoint_->family() == AF_INET, "TcpNetworkListener not support IPV6 address.");

    int fd = CreateSocket(endpoint_->family(), true);
    if (fd < 0) {
        return false;
    }

    SetListenerSocketOptions(fd);

    DASSERT(BindSocket(fd, endpoint_), "");
    DASSERT(ListenSocket(fd, 128), "");

    socket_.reset(new AsyncSocket(event_loop_, fd, kReadEvent));
    if (!socket_) {
        return false;
    }

    socket_->SetReadCallback(std::bind(&TcpNetworkListener::AcceptHandle, this));

    event_loop_->RunInLoop(std::bind(&AsyncSocket::Attach, socket_.get()));

    return true;
}

bool TcpNetworkListener::Stop() {
    DASSERT(event_loop_->IsConsistent(), "Stop error.");

    socket_->DisableAllIOEvents();
    socket_->Close();

    return true;
}

} // namespace drpc
