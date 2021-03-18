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
#include "event_loop.h"
#include "async_socket.h"

namespace drpc {

static_assert(kNoneEvent == EV_NONE, "CHECK DRPC NONE EVENT.");
static_assert(kReadEvent == EV_READ, "CHECK DRPC READ EVENT.");
static_assert(kWriteEvent == EV_WRITE, "CHECK DRPC WRITE EVENT.");

#define ASYNC_SOCKET_DESTROY(_obj, _destroy_fn) do { \
                if ((_obj)) { \
                    close(_obj->fd); \
                    _destroy_fn((_obj)); \
                    (_obj) = nullptr; \
                } \
    } while (0)

AsyncSocket::AsyncSocket(EventLoop* event_loop, int fd, int io_events) {
    DASSERT(event_loop_ = event_loop, "AsyncSocket event loop is nil.");
    DASSERT(io_ = new(std::nothrow) ev_io, "AsyncSocket new ev_io failed.")

    events_ = io_events;
    attached_ = false;

    if (fd <= 0) {
        DERROR("The fd:%d", fd);
    }

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
    if (!attached_) {
        return false;
    }

    ev_io_stop(event_loop_->event_loop(), io_);

    attached_ = false;

    return true;
}

void AsyncSocket::Close() {
    // Need check io_ is nullptr.
    if (!io_) {
        return;
    }

    if (!Detach()) {
        DDEBUG("AsyncSocket already detach.");
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
        events_ |= kReadEvent;
    } else {
        events_ &= (~kReadEvent);
    }

    if (writable) {
        events_ |= kWriteEvent;
    } else {
        events_ &= (~kWriteEvent);
    }

    if (IsNone() && attached_) {
        return;
    }

    // When the async_socket_ is already attach, must call Deatch
    // before ev_io_modify and then call Attach.
    if (events_ != old_events) {
        if (attached_) {
            DASSERT(Detach(), "AsyncSocket detach failed.");
            ev_io_modify(io_, events_);
            DASSERT(Attach(), "AsyncSocket attach failed.");
        } else {
            ev_io_modify(io_, events_);
        }
    }
}

void AsyncSocket::DisableAllIOEvents() {
    ModifyIOEvents(false, false);
}

bool AsyncSocket::IsAttached() const {
    return attached_;
}

bool AsyncSocket::IsNone() const {
    return (events_ == kNoneEvent);
}

bool AsyncSocket::IsReadable() const {
    return (events_ & kReadEvent) != 0;
}

bool AsyncSocket::IsWritable() const {
    return (events_ & kWriteEvent) != 0;
}

EventLoop* AsyncSocket::event_loop() const {
    return event_loop_;
}

int AsyncSocket::fd2() const {
    if (!io_) {
        DERROR("AsyncSocket io watcher is nil.");
        return -1;
    }

    return io_->fd;
}

void AsyncSocket::IOEventHandle(int events) {
    if ((events & kReadEvent) && read_cb_) {
        read_cb_();
    }

    if ((events & kWriteEvent) && write_cb_) {
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

} // namespace drpc
