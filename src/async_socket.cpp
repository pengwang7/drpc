#include "logger.hpp"
#include "event_loop.hpp"
#include "async_socket.hpp"

namespace drpc {

static_assert(DRPC_NONE_EVENT == EV_NONE, "CHECK DRPC NONE EVENT.");
static_assert(DRPC_READ_EVENT == EV_READ, "CHECK DRPC READ EVENT.");
static_assert(DRPC_WRITE_EVENT == EV_WRITE, "CHECK DRPC WRITE EVENT.");

#define ASYNC_SOCKET_DESTROY(_obj, _destroy_fn) do { \
                if ((_obj)) { \
                    close(_obj->fd); \
                    _destroy_fn((_obj)); \
                    (_obj) = nullptr; \
                } \
    } while (0)

AsyncSocket::AsyncSocket(EventLoop* event_loop, int fd, unsigned short io_events) {
    DASSERT(event_loop_ = event_loop, "AsyncSocket event loop is nil.");
    DASSERT(io_ = new(std::nothrow) ev_io, "AsyncSocket new ev_io failed.")

    events_ = io_events;
    attached_ = false;

    ev_io_init(io_, IOEventHandle, fd, events_);

    DTRACE("Create async socket: %p.", this);
}

AsyncSocket::~AsyncSocket() {
    DTRACE("Destroy async socket: %p.", this);
}

bool AsyncSocket::Attach() {
    if (IsNone() || attached_) {
        return false;
    }

    io_->data = (void*)this;

    ev_io_start(event_loop_->event_loop(), io_);

    attached_ = true;

    return true;
}

bool AsyncSocket::Detach() {
    if (IsNone() || !attached_) {
        return false;
    }

    ev_io_stop(event_loop_->event_loop(), io_);

    attached_ = false;

    return true;
}

void AsyncSocket::Close() {
    if (!io_) {
        return;
    }

    if (!Detach()) {
        DERROR("AsyncSocket detach failed.");
    }

    ASYNC_SOCKET_DESTROY(io_, delete);

    read_cb_ = AsyncSocketCallback();
    write_cb_ = AsyncSocketCallback();
}

void AsyncSocket::SetReadCallback(const AsyncSocketCallback& cb) {
    read_cb_ = cb;
}

void AsyncSocket::SetWriteCallback(const AsyncSocketCallback& cb) {
    write_cb_ = cb;
}

void AsyncSocket::ModifyIOEvents(bool readable, bool writable) {
    int old_events = events_;

    if (readable) {
        events_ |= DRPC_READ_EVENT;
    }

    if (writable) {
        events_ |= DRPC_WRITE_EVENT;
    }

    if (IsNone()) {
        DASSERT(!Detach(), "AsyncSocket update none events failed.");
        return;
    }

    if (events_ != old_events) {
        ev_io_modify(io_, events_);
    }
}

void AsyncSocket::DisableAllIOEvents() {
    ModifyIOEvents(false, false);
}

bool AsyncSocket::IsAttached() const {
    return attached_;
}

bool AsyncSocket::IsNone() const {
    return (events_ == DRPC_NONE_EVENT);
}

bool AsyncSocket::IsReadable() const {
    return (events_ & DRPC_READ_EVENT) != 0;
}

bool AsyncSocket::IsWritable() const {
    return (events_ & DRPC_WRITE_EVENT) != 0;
}

int AsyncSocket::fd2() const {
    if (!io_) {
        DERROR("AsyncSocket io watcher is nil.");
        return -1;
    }

    return io_->fd;
}

void AsyncSocket::IOEventHandle(int events) {
    if ((events & DRPC_READ_EVENT) && read_cb_) {
        read_cb_();
    }

    if ((events & DRPC_WRITE_EVENT) && write_cb_) {
        write_cb_();
    }
}

void AsyncSocket::IOEventHandle(struct ev_loop* loop, struct ev_io* io, int events) {
    if (events & EV_ERROR) {
        DERROR("AsyncSocket callback error.");
        return;
    }

    if (!io->data) {
        DERROR("AsyncSocket callback instance is nil.");
        return;
    }

    AsyncSocket* async_socket = static_cast<AsyncSocket*>(io->data);
    if (!async_socket) {
        DERROR("AsyncSocket callback static_cast ret is nil.");
        return;
    }

    async_socket->IOEventHandle(events);
}

} /* end namespace drpc */
