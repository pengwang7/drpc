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

namespace drpc {

EventLoop::EventLoop() {
    // Create libev event loop used EPOLL engine.
    event_loop_ = ev_loop_new(EVBACKEND_EPOLL);
    DASSERT(event_loop_, "EventLoop is nil.");

    DoInit();
}

EventLoop::~EventLoop() {
    async_watcher_->Terminate();
    async_watcher_->Cancel();

    delete pending_task_queue_;
    pending_task_queue_ = nullptr;

    OBJECT_SAFE_DESTROY(event_loop_, ev_loop_destroy);
}

void EventLoop::Run() {
    status_.store(kStarting);

    // The actual thread id.
    thread_id_ = std::this_thread::get_id();

    // Register async watcher to event loop.
    DASSERT(async_watcher_->Watching(), "The async watcher watching failed.");

    status_.store(kRunning);

    // Start libev event loop to watching all register IO events.
    ev_run(event_loop_, 0);

    status_.store(kStopped);
}

void EventLoop::Stop() {
    DASSERT(status_.load() == kRunning, "Stop failed.");

    status_.store(kStopping);

    auto stop = [this]() -> void {
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

        DDEBUG("The event loop is stopped.");
    };

    RunInLoop(stop);
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
            DTRACE("Status: {}", ToString());
            DASSERT(!IsRunning(), "Status: {}", ToString());
        }
    }
}

TimerControllerPtr EventLoop::RunEvery(const Functor& task, uint32_t delay_sec, bool persist) {
    TimerControllerPtr cnt = TimerController::Create(this, task, delay_sec, persist);

    cnt->Run();

    return cnt;
}

TimerControllerPtr EventLoop::RunAfter(const Functor& task, uint32_t delay_sec, bool persist) {
    TimerControllerPtr cnt = TimerController::Create(this, task, delay_sec, persist);

    cnt->Run();

    return cnt;
}

void EventLoop::DoInit() {
    status_.store(kInitializing);

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

void EventLoop::DoPendingTasks() {
    notified_.store(false);

    Functor task;

    while (pending_task_queue_->try_dequeue(task)) {
        DDEBUG("Doing pending queue task.");
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
