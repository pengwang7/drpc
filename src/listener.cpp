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
    DASSERT(ListenSocket(fd, 1024), "Start failed.");

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
        listen_socket_->Detach();
        listen_socket_->Close();
    } else {
        event_loop_->SendToQueue(std::bind(&AsyncSocket::Detach, listen_socket_.get()));
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

    DTRACE("Listener accept socket: %d, peer adddress: %s.", sock, peer_addr);

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