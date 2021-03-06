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

#include "logger.hpp"
#include "utility.hpp"
#include "event_loop.hpp"

namespace drpc {

EventLoop::EventLoop(std::size_t max_register_size) {
    DTRACE("The event loop create:%p.", this);

    cur_register_size_ = 0;
    max_register_size_ = max_register_size;
    if (max_register_size_ == 0) {
        DDEBUG("The event loop don't limit channel registration size.");
    }

    notified_.store(false);

    // The first time thread id is set, the really
    // thread id is set when Run will be called.
    // Here's a scenario had memory leak.
    // 1. new event_loop()
    // 2. listener->Run()
    // 3. listener->Stop()
    // 4. Don not call event_loop->Run()
    thread_id_ = std::this_thread::get_id();

    DoInit();
}

EventLoop::~EventLoop() {
    DTRACE("The event loop destroy:%p.", this);

    async_watcher_->Terminate();
    async_watcher_->Cancel();

    delete pending_task_queue_;

    OBJECT_SAFE_DESTROY(event_loop_, ev_loop_destroy);
}

void EventLoop::Run() {
    // Set the thread to which the event loop belongs.
    thread_id_ = std::this_thread::get_id();

    // Register async watcher to event loop.
    DASSERT(async_watcher_->Watching(), "The async watcher watching failed.");

    // Start libev event loop to watching all register IO events.
    ev_run(event_loop_, 0);
}

void EventLoop::Stop() {
    if (sched_) {
        sched_->Cancel();
    }

    auto stop_task = [this]() -> void {
        // Do all tasks before stop event loop.
        while (true) {
            if (PendingTaskQueueIsEmpty()) {
                break;
            }

            DoPendingTasks();
        }

        // If ev_break is called on a different thread than ev_run,
        // then the Event Loop does not exit.
        ev_break(event_loop_, EVBREAK_ALL);

        DDEBUG("The event loop is stopped:%p.", this);
    };

    SendToQueue(stop_task);
}

void EventLoop::StartChannelTimeoutCheck(ev_tstamp seconds, const TaskFunctor& task) {
    if (seconds <= 0) {
        DWARNING("Check channel timeout failed.");
        return;
    }

    sched_ = Scheduled::Create(this, task, seconds, true);

    sched_->Run();
}

void EventLoop::SendToQueue(const TaskFunctor& task) {
    if (thread_id_ == std::this_thread::get_id()) {
        DDEBUG("Run in this thread.");

        task();
    } else {
        DDEBUG("Send to other thread.");

        while (!pending_task_queue_->enqueue(task)) {}

        if (!notified_.load()) {
            notified_.store(true);
            async_watcher_->Notify();
        }
    }
}

void EventLoop::SendToQueue(TaskFunctor&& task) {
    if (thread_id_ == std::this_thread::get_id()) {
        DDEBUG("Run in this thread.");

        task();
    } else {
        DDEBUG("Send to other thread.");

        while (!pending_task_queue_->enqueue(std::move(task))) {}

        if (!notified_.load()) {
            notified_.store(true);

            if (async_watcher_) {
                async_watcher_->Notify();
            }
        }
    }
}

void EventLoop::DoInit() {
    // Create libev event loop used EPOLL engine.
    event_loop_ = ev_loop_new(EVBACKEND_EPOLL);
    DASSERT(event_loop_, "EventLoop is nil.");

    // The queue for do other thread post task.
    pending_task_queue_ = new moodycamel::ConcurrentQueue<TaskFunctor>();
    DASSERT(pending_task_queue_, "ConcurrentQueue is nil.");

    // Message notification between different threads.
    async_watcher_.reset(new EventfdWatcher(this, std::bind(&EventLoop::DoPendingTasks, this)));
    DASSERT(async_watcher_, "AsyncWatcher is nil.");

    DASSERT(async_watcher_->Init(), "The async watcher init failed.");
}

void EventLoop::DoPendingTasks() {
    DDEBUG("Doing pending queue task.");

    TaskFunctor task;

    notified_.store(false);

    while (pending_task_queue_->try_dequeue(task)) {
        task();
    }
}

bool EventLoop::PendingTaskQueueIsEmpty() {
    return pending_task_queue_->size_approx() == 0;
}

} /* end namespace drpc */
