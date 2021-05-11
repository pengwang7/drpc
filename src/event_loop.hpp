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

#ifndef __EVENT_LOOP_HPP__
#define __EVENT_LOOP_HPP__

#include <thread>

#include "any_cast.hpp"
#include "status_manager.hpp"
#include "concurrentqueue.hpp"
#include "constructor_magic.hpp"
#include "async_watcher.hpp"

namespace drpc {

const uint32_t kContextMax = 16;

// This is the IO Event driving kernel. Reactor model with
// one loop peer thread. This class is a wrapper of event_base
// but not only a wrapper. It provides a simple way to run a
// IO event driving loop.
class EventLoop : public StatusManager {
public:
    using Functor = std::function<void()>;

public:
    EventLoop();

    ~EventLoop();

    void Run();

    void Stop();

    void RunInLoop(Functor&& task);

    void QueueInLoop(Functor&& task);

    void RunInLoop(const Functor& task);

    void QueueInLoop(const Functor& task);

    // InvokeTimerPtr RunAfter(Duration delay, Functor&& task);

    // InvokeTimerPtr RunEvery(Duration interval, Functor&& task);

public:
    void SetContext(uint32_t index, const Any& context) {
        DASSERT(index < kContextMax, "Invalid index.");
        context_[index] = context;
    }

    const Any& GetContext(uint32_t index) const {
        DASSERT(index < kContextMax, "Invalid index.");
        return context_[index];
    }

    struct event_base* event_loop() const {
        return event_loop_;
    }

    std::thread::id thread_id() const {
        return thread_id_;
    }

    bool IsConsistent() const {
        return thread_id_ == std::this_thread::get_id();
    }

private:
    void DoInit();

    void StopInLoop();

    void DoPendingTasks();

    bool PendingTaskQueueIsEmpty();

private:
    DISALLOW_COPY_AND_ASSIGN(EventLoop);

    struct event_base* event_loop_;

    std::thread::id thread_id_;

    Any context_[kContextMax];

    // We use this to notify the thread when we put a task into the pending_functors_ queue.
    std::unique_ptr<EventfdWatcher> async_watcher_;

    // When we put a task into the pending_functors_ queue,
    // we need to notify the thread to execute it. But we don't want to notify repeatedly.
    std::atomic<bool> notified_;

    // Task queue.
    moodycamel::ConcurrentQueue<Functor>* pending_task_queue_;
};

} // namespace drpc

#endif // __EVENT_LOOP_HPP__
