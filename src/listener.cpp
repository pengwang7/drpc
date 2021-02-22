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

Listener::Listener(EventLoop* event_loop, ServerOptions* options)
    : event_loop_(event_loop), options_(options) {
    DASSERT(event_loop_, "Listener event_loop is nil.");
    DASSERT(options_, "Listener server options is nil.");
}

Listener::~Listener() {
    new_conn_cb_ = nullptr;
}

bool Listener::Start() {
    IPAddress ip;
    DASSERT(IPFromString(options_->address, &ip), "Start failed.");
    DASSERT(ip.family() == AF_INET, "Listener not support IPV6 address.");

    int fd = CreateSocket(ip.family(), true);
    if (fd < 0) {
        DERROR("Start listener failed.");
        return false;
    }

    SetSocketOptions(fd, SO_REUSEADDR);
    SetSocketOptions(fd, TCP_DEFER_ACCEPT);

    if (options_->server_mode == ServerMode::OLPT_REUSE_PORT) {
        SetSocketOptions(fd, SO_REUSEPORT);
    }

    DASSERT(BindSocket(fd, ip, options_->port), "Start failed.");
    DASSERT(ListenSocket(fd, 128), "Start failed.");

    listen_socket_.reset(new AsyncSocket(event_loop_, fd, kReadEvent));
    if (!listen_socket_) {
        return false;
    }

    listen_socket_->SetReadCallback(std::bind(&Listener::AcceptHandle, this));

    if (event_loop_->IsConsistent()) {
        listen_socket_->Attach();
    } else {
        event_loop_->SendToQueue(std::bind(&AsyncSocket::Attach, listen_socket_.get()));
    }

    return true;
}

bool Listener::Stop() {
    if (event_loop_->IsConsistent()) {
        listen_socket_->Close();
    } else {
        event_loop_->SendToQueue(std::bind(&AsyncSocket::Close, listen_socket_.get()));
    }

    return true;
}

void Listener::SetNewConnCallback(NewConnCallback cb) {
    new_conn_cb_ = cb;
}

void Listener::AcceptHandle() {
    DASSERT(event_loop_->IsConsistent(), "Listener error.");

    std::string peer_addr;

    int sock = AcceptSocket(listen_socket_->fd2(), peer_addr);
    if (sock < 0) {
        if (errno != EAGAIN && errno != EINTR) {
            DERROR("Listener accept failed: %s", strerror(errno));
        }
        return;
    }

    SetSocketOptions(sock, O_NONBLOCK);
    SetSocketOptions(sock, TCP_NODELAY);
    SetSocketOptions(sock, TCP_QUICKACK);

    if (!new_conn_cb_) {
        DERROR("Listener new conn cb is nil.");
        return;
    }

    new_conn_cb_(sock, peer_addr);
}

} /* end namespace drpc */
