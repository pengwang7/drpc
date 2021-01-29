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
#include "event_loop.hpp"
#include "event_loop_group.hpp"

namespace drpc {

EventLoopGroup::EventLoopGroup(std::size_t n, std::string name) {
    group_size_ = n;
    group_name_ = name;
    group_cursor_ = 0;
}

EventLoopGroup::~EventLoopGroup() {
    group_size_ = 0;
    group_cursor_ = 0;
    thread_group_.clear();
}

void EventLoopGroup::Run(bool detach) {
    if (group_size_ == 0) {
        DERROR("The group size is 0.");
        return;
    }

    for (std::size_t i = 0; i < group_size_; ++ i) {
        std::shared_ptr<Thread> thd(new Thread());
        thd->Run(detach);

        while (!thd->IsRunning()) {
            DDEBUG("The thread is not ready, wait 5000us.");
            usleep(5000);
        }

        thread_group_.emplace_back(thd);
    }

    DDEBUG("Start group all thread success.");
}

void EventLoopGroup::Stop() {
    for (auto& t : thread_group_) {
        t->Stop();
    }

    DDEBUG("Stop group all thread success.");
}

void EventLoopGroup::Wait() {
    for (auto& t : thread_group_) {
        t->Wait();
    }

    DDEBUG("Wait group all thread success.");
}

EventLoop* EventLoopGroup::event_loop() {
    EventLoop* best_event_loop = nullptr;
    std::size_t min = std::numeric_limits<std::size_t>::max();

    {
        // Lock there ? Maybe.
        // std::lock_guard<std::mutex> scoped_lock(group_mutex_);
        for (std::size_t i = 0; i < group_size_; ++ i) {
            if (thread_group_[i]->event_loop()->GetCurrentRegisterSize() < min) {
                min = thread_group_[i]->event_loop()->GetCurrentRegisterSize();
                best_event_loop = thread_group_[i]->event_loop();
            }
        }
    }

    return best_event_loop;
}

std::size_t EventLoopGroup::size() {
    return group_size_;
}

EventLoopGroup::Thread::Thread()
    : event_loop_(new EventLoop()), state_(INITIALIZE), detach_(false) {
    DASSERT(event_loop_, "Thread error.");
}

EventLoopGroup::Thread::~Thread() {
    DASSERT(state_ == STOPPED, "Thread state error.");
    DTRACE("Thread destroy: %p", this);
}

void EventLoopGroup::Thread::Run(bool detach) {
    thread_.reset(new std::thread(std::bind(&EventLoopGroup::Thread::Cycle, this)));
    if (detach) {
        thread_->detach();
        detach_ = true;
    }
}

void EventLoopGroup::Thread::Stop() {
    event_loop_->Stop();
}

void EventLoopGroup::Thread::Wait() {
    if (detach_) {
        state_ = STOPPED;
        return;
    }

    std::lock_guard<std::mutex> scoped_lock(mutex_);
    if (thread_ && thread_->joinable()) {
        thread_->join();
    }
}

void EventLoopGroup::Thread::Cycle() {
    DDEBUG("On cycle running thread id: %ld, event_loop name: %p.", pthread_self(), event_loop_.get());

    state_ = RUNNING;

    event_loop_->Run();

    state_ = STOPPED;

    DDEBUG("On cycle stopped thread id: %ld, event_loop name: %p.", pthread_self(), event_loop_.get());
}

bool EventLoopGroup::Thread::IsRunning() const {
    return state_ == RUNNING;
}

EventLoop* EventLoopGroup::Thread::event_loop() const {
    return event_loop_.get();
}

std::thread::id EventLoopGroup::Thread::thread_id() const {
    return std::thread::id();
}

} /* end namespace drpc */
