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
#include "utility.hpp"
#include "event_loop.hpp"

namespace drpc {

EventLoop::EventLoop() {
    DoInit();
}

EventLoop::~EventLoop() {
    OBJECT_SAFE_DESTROY(event_loop_, event_base_free);

    delete pending_task_queue_;
    pending_task_queue_ = nullptr;
}

void EventLoop::Run() {
    status_.store(kStarting);

    // The actual thread id.
    thread_id_ = std::this_thread::get_id();

    // Register async watcher to event loop.
    DASSERT(async_watcher_->AsyncWait(), "The async watcher watching failed.");

    status_.store(kRunning);

    // Start event loop to watching all register IO events.
    int rc = event_base_dispatch(event_loop_);
    if (rc == 1) {
        DERROR("RUN failed, no event registered.");
    } else if (rc == -1) {
        DERROR("Run failed, {} {}.", errno, std::strerror(errno));
    }

    async_watcher_.reset();

    DTRACE("EventLoop stopped.");

    status_.store(kStopped);
}

void EventLoop::Stop() {
    DASSERT(status_.load() == kRunning, "EventLoop status error.");

    status_.store(kStopping);

    QueueInLoop(std::bind(&EventLoop::StopInLoop, this));
}

void EventLoop::RunInLoop(Functor&& task) {
    if (IsRunning() && IsConsistent()) {
        task();
    } else {
        QueueInLoop(std::move(task));
    }
}

void EventLoop::QueueInLoop(Functor&& task) {
    while (!pending_task_queue_->enqueue(std::move(task))) {}

    if (!notified_.load()) {
        notified_.store(true);

        if (async_watcher_) {
            async_watcher_->Notify();
        } else {
            DTRACE("async_watcher_ is empty, maybe we call EventLoop::QueueInLoop on a stopped EventLoop.");
            DASSERT(!IsRunning(), "{}", StatusToString());
        }
    }
}

void EventLoop::RunInLoop(const Functor& task) {
    if (IsRunning() && IsConsistent()) {
        task();
    } else {
        QueueInLoop(task);
    }
}

void EventLoop::QueueInLoop(const Functor& task) {
    while (!pending_task_queue_->enqueue(task)) {}

    if (!notified_.load()) {
        notified_.store(true);

        if (async_watcher_) {
            async_watcher_->Notify();
        } else {
            DTRACE("async_watcher_ is empty, maybe we call EventLoop::QueueInLoop on a stopped EventLoop.");
            DASSERT(!IsRunning(), "{}", StatusToString());
        }
    }
}

void EventLoop::DoInit() {
    status_.store(kInitializing);

    // Each event_base executes in a single thread, selected with no lock.
    struct event_config* cfg = event_config_new();
    DASSERT(cfg, "event_config is nil.");

    event_config_set_flag(cfg, EVENT_BASE_FLAG_NOLOCK);
    event_config_set_flag(cfg, EVENT_BASE_FLAG_EPOLL_USE_CHANGELIST);

    event_loop_ = event_base_new_with_config(cfg);
    DASSERT(cfg, "event_base is nil.");

    event_config_free(cfg);
    cfg = NULL;

    notified_.store(false);

    thread_id_ = std::this_thread::get_id();

    // The queue for do other thread post task.
    pending_task_queue_ = new moodycamel::ConcurrentQueue<Functor>();
    DASSERT(pending_task_queue_, "ConcurrentQueue is nil.");

    // Message notification between different threads.
    async_watcher_.reset(new EventfdWatcher(this, std::bind(&EventLoop::DoPendingTasks, this)));
    DASSERT(async_watcher_, "AsyncWatcher is nil.");

    DASSERT(async_watcher_->Init(), "The async watcher init failed.");

    status_.store(kInitialized);
}

void EventLoop::StopInLoop() {
    DTRACE("EventLoop is stopping now.");

    DASSERT(status_.load() == kStopping, "EventLoop status error.");

    auto fn = [this]() -> void {
        for (int i = 0; ; i++) {
            DTRACE("Calling DoPendingFunctors index={}.", i);

            DoPendingTasks();

            if (PendingTaskQueueIsEmpty()) {
                break;
            }
        }
    };

    DTRACE("Before event_base_loopexit, invoke DoPendingTasks.");

    fn();

    event_base_loopexit(event_loop_, nullptr);

    DTRACE("After event_base_loopexit, invoke DoPendingTasks.");

    fn();

    DTRACE("End of StopInLoop.");
}

void EventLoop::DoPendingTasks() {
    notified_.store(false);

    Functor task;

    while (pending_task_queue_->try_dequeue(task)) {
        task();
    }
}

bool EventLoop::PendingTaskQueueIsEmpty() {
    if (!pending_task_queue_) {
        return true;
    }

    return pending_task_queue_->size_approx() == 0;
}

} // namespace drpc
