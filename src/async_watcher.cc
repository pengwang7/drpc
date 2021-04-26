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

#include <ev.h>

#include "logger.h"
#include "utility.h"
#include "event_loop.h"
#include "async_watcher.h"

namespace drpc {

AsyncWatcher::AsyncWatcher(struct ev_loop* event_loop,
                           AsyncWatcherTaskFunctor&& handle, AsyncWatcherType type)
    : event_loop_(event_loop), io_(NULL), timer_(NULL), type_(type),
      task_handle_(std::move(handle)), attached_(false) {

    if (type_ == AsyncWatcherType::IO) {
        io_ = static_cast<ev_io*>(calloc(1, sizeof(*io_)));
        memset(io_, 0, sizeof(*io_));
    } else {
        timer_ = static_cast<ev_timer*>(calloc(1, sizeof(*timer_)));
        memset(timer_, 0, sizeof(*timer_));
    }

    DASSERT(type_ == AsyncWatcherType::IO || type_ == AsyncWatcherType::TIMER, "Invalid AsyncWatcher type.");

    DTRACE("Create AsyncWatcher.");
}

AsyncWatcher::~AsyncWatcher() {
    DASSERT(!io_ && !timer_ && !attached_, "AsyncWatcher memory leak.");

    DTRACE("Destroy AsyncWatcher.");
}

bool AsyncWatcher::Init() {
    if (!DoInitImpl()) {
        DoTerminateImpl();
        DERROR("AsyncWatcher initialize failed.");

        return false;
    }

    return true;
}

bool AsyncWatcher::Watching() {
    if (attached_ && io_) {
        ev_io_stop(event_loop_, io_);
    }

    if (attached_ && timer_) {
        ev_timer_stop(event_loop_, timer_);
    }

    attached_ = false;

    if (type_ == AsyncWatcherType::IO) {
        ev_io_start(event_loop_, io_);
    } else {
        ev_timer_start(event_loop_, timer_);
    }

    attached_ = true;

    return true;
}

void AsyncWatcher::Cancel() {
    if (attached_) {
        if (type_ == AsyncWatcherType::IO) {
            ev_io_stop(event_loop_, io_);
        } else {
            ev_timer_stop(event_loop_, timer_);
        }

        attached_ = false;
    }

    OBJECT_SAFE_DESTROY(io_, free);
    OBJECT_SAFE_DESTROY(timer_, free);

    // After the cancel_handle_ executed, AsyncWatcher will be destroyed.
    if (cancel_handle_) {
        cancel_handle_();
    }
}

void AsyncWatcher::Terminate() {
    DoTerminateImpl();
}

EventfdWatcher::EventfdWatcher(EventLoop* event_loop,
                               AsyncWatcherTaskFunctor&& handle)
    : AsyncWatcher(event_loop->event_loop(), std::move(handle)) {

}

EventfdWatcher::~EventfdWatcher() {
    DoTerminateImpl();
}

void EventfdWatcher::Notify() {
    uint64_t data = 3399;
    if (write(event_fd_, &data, sizeof(data)) != sizeof(uint64_t)) {
        DERROR("EventfdWatcher notify failed.");
        return;
    }
}

bool EventfdWatcher::DoInitImpl() {
    event_fd_ = eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
    if (event_fd_ < 0) {
        DoTerminateImpl();
        DERROR("EventfdWatcher initialize error: {}.", std::strerror(errno));
        return false;
    }

    io_->data = static_cast<void*>(this);
    ev_io_init(io_, EventfdWatcher::NotifyHandle, event_fd_, EV_READ);

    return true;
}

void EventfdWatcher::DoTerminateImpl() {
    DTRACE("EventfdWatcher closed event fd.");
    close(event_fd_);
    event_fd_ = -1;
}

void EventfdWatcher::NotifyHandle(struct ev_loop* event_loop, struct ev_io* io, int events) {
    uint64_t data = 0;

    if (events & EV_ERROR) {
        DERROR("EventfdWatcher receive error: {}.", events);
        return;
    }

    EventfdWatcher* fd_watcher = static_cast<EventfdWatcher*>(io->data);
    if (!fd_watcher) {
        DERROR("EventfdWatcher watcher is nil.");
        return;
    }

    if (read(fd_watcher->event_fd_, &data, sizeof(data)) != sizeof(uint64_t)) {
        DERROR("EventfdWatcher read event fd error: {}.", std::strerror(errno));
        return;
    }

    DTRACE("EventfdWatcher receive notify success.");

    fd_watcher->task_handle_();
}


TimerEventWatcher::TimerEventWatcher(EventLoop* event_loop,
                                     AsyncWatcherTaskFunctor&& handle, uint32_t delay_sec, bool persist)
    : AsyncWatcher(event_loop->event_loop(), std::move(handle), AsyncWatcherType::TIMER),
      delay_sec_(delay_sec), persist_(persist)  {

}

bool TimerEventWatcher::DoInitImpl() {
    timer_->data = static_cast<void*>(this);
    ev_timer_init(timer_, TimerEventWatcher::NotifyHandle, delay_sec_, persist_ ? delay_sec_ : 0);

    return true;
}

void TimerEventWatcher::DoTerminateImpl() {
    // TODO:
}

void TimerEventWatcher::NotifyHandle(struct ev_loop* event_loop, struct ev_timer* timer, int events) {
    if (events & EV_ERROR) {
        DERROR("TimerEventWatcher receive error: {}.", events);
        return;
    }

    TimerEventWatcher* timer_watcher = static_cast<TimerEventWatcher*>(timer->data);
    if (!timer_watcher) {
        DERROR("TimerEventWatcher watcher is nil.");
        return;
    }

    if (events & EV_TIMER) {
        timer_watcher->task_handle_();
    }
}

} // namespace drpc
