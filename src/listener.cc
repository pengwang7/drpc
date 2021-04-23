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

#include "logger.h"
#include "socket.h"
#include "async_socket.h"
#include "event_loop.h"
#include "listener.h"

namespace drpc {

NetworkListener::NetworkListener(
    EventLoop* event_loop, ServerOptions* options)
    : event_loop_(event_loop), options_(options) {
    DTRACE("Construt NetworkListener.");
}

NetworkListener::~NetworkListener() {
    DTRACE("Destruct NetworkListener.");
}

void NetworkListener::AcceptHandle() {
    DASSERT(event_loop_->IsConsistent(), "NetworkListener error.");

    std::string peer_addr;

    int sock = AcceptSocket(listen_socket_->fd2(), peer_addr);
    if (sock < 0) {
        if (errno != EAGAIN && errno != EINTR) {
            DERROR("NetworkListener accept failed: {}.", std::strerror(errno));
        }
        return;
    }

    if (SetSocketOptions(sock, O_NONBLOCK) ||
            SetSocketOptions(sock, TCP_NODELAY) ||
            SetSocketOptions(sock, TCP_QUICKACK)) {
        close(sock);
        return;
    }

    if (!new_conn_cb_) {
        close(sock);
        DERROR("NetworkListener new conn cb is nil.");
        return;
    }

    new_conn_cb_(sock, peer_addr);
}

BasicNetworkListener::BasicNetworkListener(
    EventLoop* event_loop, ServerOptions* options)
    : NetworkListener(event_loop, options) {
    DTRACE("Construt BasicNetworkListener.");
}

BasicNetworkListener::~BasicNetworkListener() {
    DTRACE("Destruct BasicNetworkListener.");
}

bool BasicNetworkListener::Start() {
    int fd = CreateSocket(AF_UNIX, true);
    if (fd < 0) {
        DERROR("Start failed.");
        return false;
    }

    SetSocketOptions(fd, SO_REUSEADDR);
    SetSocketOptions(fd, TCP_DEFER_ACCEPT);

    if (options_->server_mode == ServerMode::OLPT_REUSE_PORT) {
        SetSocketOptions(fd, SO_REUSEPORT);
    }

    DASSERT(BindSocket(fd, options_->address), "Start failed.");
    DASSERT(ListenSocket(fd, 128), "Start failed.");

    listen_socket_.reset(new AsyncSocket(event_loop_, fd, kReadEvent));
    if (!listen_socket_) {
        return false;
    }

    listen_socket_->SetReadCallback(std::bind(&BasicNetworkListener::AcceptHandle, this));

    event_loop_->RunInLoop(std::bind(&AsyncSocket::Attach, listen_socket_.get()));

    return true;
}

bool BasicNetworkListener::Stop() {
    DASSERT(event_loop_->IsConsistent(), "Stop error.");

    listen_socket_->Close();

    return true;
}

TcpNetworkListener::TcpNetworkListener(
    EventLoop* event_loop, ServerOptions* options)
    : NetworkListener(event_loop, options) {
    DTRACE("Construt TcpNetworkListener.");
}

TcpNetworkListener::~TcpNetworkListener() {
    DTRACE("Destruct TcpNetworkListener.");
}

bool TcpNetworkListener::Start() {
    IPAddress ip;
    DASSERT(IPFromString(options_->address, &ip), "Start failed.");
    DASSERT(ip.family() == AF_INET, "TcpNetworkListener not support IPV6 address.");

    int fd = CreateSocket(ip.family(), true);
    if (fd < 0) {
        DERROR("Start failed.");
        return false;
    }

    if (SetSocketOptions(fd, SO_REUSEADDR) ||
            SetSocketOptions(fd, TCP_DEFER_ACCEPT)) {
        DERROR("Start failed.");
        return false;
    }

    if (options_->server_mode == ServerMode::OLPT_REUSE_PORT) {
        SetSocketOptions(fd, SO_REUSEPORT);
    }

    DASSERT(BindSocket(fd, ip, options_->port), "Start failed.");
    DASSERT(ListenSocket(fd, 128), "Start failed.");

    listen_socket_.reset(new AsyncSocket(event_loop_, fd, kReadEvent));
    if (!listen_socket_) {
        return false;
    }

    listen_socket_->SetReadCallback(std::bind(&TcpNetworkListener::AcceptHandle, this));

    event_loop_->RunInLoop(std::bind(&AsyncSocket::Attach, listen_socket_.get()));

    return true;
}

bool TcpNetworkListener::Stop() {
    //DASSERT(event_loop_->IsConsistent(), "Stop error.");

    event_loop_->RunInLoop(std::bind(&AsyncSocket::Close, listen_socket_.get()));

    //listen_socket_->Close();

    return true;
}

} // namespace drpc