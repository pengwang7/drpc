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
#include "utility.hpp"
#include "event.hpp"
#include "event_loop.hpp"
#include "async_watcher.hpp"

namespace drpc {

AsyncWatcher::AsyncWatcher(struct event_base* event_loop, AsyncWatcherFunctor&& handle)
    : event_loop_(event_loop), io_(nullptr), attached_(false),
      task_handle_(std::move(handle)), cancel_handle_(nullptr)  {

    DTRACE("Create AsyncWatcher.");

    io_ = new ::event;
    DASSERT(io_, "Create AsyncWatcher error.");

    memset(io_, 0, sizeof(struct event));
}

AsyncWatcher::~AsyncWatcher() {
    DTRACE("Destroy AsyncWatcher.");
    FreeEvent();
    Close();
}

bool AsyncWatcher::Init() {
    if (!DoInitImpl()) {
        goto failed;
    }

    event_add(io_, nullptr);

    return true;

failed:
    Close();

    return false;
}

void AsyncWatcher::Cancel() {
    DASSERT(io_, "AsyncWatcher error.");

    FreeEvent();

    // After the cancel_handle_ executed, AsyncWatcher will be destroyed.
    if (cancel_handle_) {
        cancel_handle_();
    }
}

bool AsyncWatcher::Watching(Duration duration) {
    struct timeval tv;
    struct timeval* timeoutval = nullptr;
    if (duration.Nanoseconds() > 0) {
        duration.To(&tv);
        timeoutval = &tv;
    }

    if (attached_) {
        event_del(io_);
        attached_ = false;
    }

    if (event_add(io_, timeoutval) != 0) {
        DERROR("AsyncWatcher event_add failed, fd={}.", io_->ev_fd);
        return false;
    }

    attached_ = true;

    return true;
}

void AsyncWatcher::Close() {
    DoCloseImpl();
}

void AsyncWatcher::FreeEvent() {
    if (io_) {
        if (attached_) {
            event_del(io_);
            attached_ = false;
        }

        delete io_;
        io_ = nullptr;
    }
}

EventfdWatcher::EventfdWatcher(EventLoop* event_loop, AsyncWatcherFunctor&& handle)
    : AsyncWatcher(event_loop->event_loop(), std::move(handle)) {

}

EventfdWatcher::~EventfdWatcher() {
    Close();
}

bool EventfdWatcher::AsyncWait() {
    return Watching(Duration());
}

void EventfdWatcher::Notify() {
    uint64_t data = 3399;
    if (write(event_fd_, &data, sizeof(data)) != sizeof(uint64_t)) {
        DERROR("EventfdWatcher notify failed, error={}.", std::strerror(errno));
        return;
    }
}

bool EventfdWatcher::DoInitImpl() {
    event_fd_ = eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
    if (event_fd_ < 0) {
        DERROR("EventfdWatcher initialize error: {}.", std::strerror(errno));
        goto failed;
    }

    event_assign(this->io_, event_loop_, event_fd_,
                 EV_READ | EV_PERSIST,
                 &EventfdWatcher::NotifyHandle, this);

    return true;

failed:
    Close();

    return false;
}

void EventfdWatcher::DoCloseImpl() {
    close(event_fd_);
    event_fd_ = -1;
}

void EventfdWatcher::NotifyHandle(int fd, short events, void* args) {
    uint64_t data = 0;

    if (events & EV_CLOSED) {
        DERROR("EventfdWatcher receive error: {}.", events);
        return;
    }

    EventfdWatcher* fd_watcher = static_cast<EventfdWatcher*>(args);
    if (!fd_watcher) {
        DERROR("EventfdWatcher watcher is nil.");
        return;
    }

    if (read(fd_watcher->event_fd_, &data, sizeof(data)) != sizeof(uint64_t)) {
        DERROR("EventfdWatcher read event fd error: {}.", std::strerror(errno));
        return;
    }

    fd_watcher->task_handle_();
}

} // namespace drpc
