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
#include "event.hpp"
#include "event_loop.hpp"
#include "async_socket.hpp"

namespace drpc {

static_assert(kReadEvent == EV_READ, "CHECK DRPC READ EVENT.");
static_assert(kWriteEvent == EV_WRITE, "CHECK DRPC WRITE EVENT.");

AsyncSocket::AsyncSocket(EventLoop* event_loop, int fd, int io_events)
    : event_loop_(event_loop), io_(nullptr), fd_(fd), events_(io_events), attached_(false) {
    DTRACE("Create AsyncSocket fd={}.", fd_);
    DASSERT(event_loop_ = event_loop, "AsyncSocket event loop is nil.");
    DASSERT(fd_ > 0, "AsyncSocket fd is invalid.");
    DASSERT(io_ = new(std::nothrow) event, "AsyncSocket new io failed.");

    memset(io_, 0, sizeof(*io_));
}

AsyncSocket::~AsyncSocket() {
    DTRACE("Destroy AsyncSocket fd={}.", fd_);
    DASSERT(io_ == nullptr, "Memory leak.");
}

bool AsyncSocket::Attach() {
    DASSERT(event_loop_->IsConsistent(), "Attach error.");
    DASSERT(!IsNone(), "Attach error.");

    if (attached_) {
        DASSERT(Detach(), "Attach error.");
    }

    event_assign(io_, event_loop_->event_loop(), fd_,
                 events_ | EV_PERSIST, &AsyncSocket::IOEventHandle, this);

    if (event_add(io_, nullptr) == 0) {
        attached_ = true;
        return true;
    }

    DERROR("Attach to event loop failed, fd={}.", fd_);

    return false;
}

bool AsyncSocket::Detach() {
    DASSERT(event_loop_->IsConsistent(), "Detach error.");
    DASSERT(attached_, "Detach error.");

    if (event_del(io_) == 0) {
        attached_ = false;

        return true;
    }

    DERROR("Detach from event loop failed, fd={}.", fd_);

    return false;
}

void AsyncSocket::Close() {
    DTRACE("Close AsyncSocket fd={}.", fd_);

    DASSERT(io_, "Close error.");
    DASSERT(!attached_, "Close error.");

    OBJECT_SAFE_DESTROY(io_, delete);

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

    if (old_events == events_) {
        return;
    }

    if (IsNone()) {
        DASSERT(Detach(), "AsyncSocket detach failed.");
    } else {
        DASSERT(Attach(), "AsyncSocket attach failed.");
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

std::string AsyncSocket::EventsToString() const {
    std::string s;

    if (events_ & kReadEvent) {
        s = "kReadEvent";
    }

    if (events_ & kWriteEvent) {
        if (!s.empty()) {
            s += "|";
        }

        s += "kWriteEvent";
    }

    return s;
}

EventLoop* AsyncSocket::event_loop() const {
    return event_loop_;
}

int AsyncSocket::fd() const {
    return fd_;
}

void AsyncSocket::IOEventHandle(int fd, short which) {
    DASSERT(fd == fd_, "IOEventHandle error.");

    if ((which & kReadEvent) && read_cb_) {
        read_cb_();
    }

    if ((which & kWriteEvent) && write_cb_) {
        write_cb_();
    }
}

void AsyncSocket::IOEventHandle(int fd, short which, void* args) {
    AsyncSocket* async_socket = static_cast<AsyncSocket*>(args);
    if (async_socket) {
        async_socket->IOEventHandle(fd, which);
    }
}

} // namespace drpc
