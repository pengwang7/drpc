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

#ifndef __EVENT_LOOP_H__
#define __EVENT_LOOP_H__

#include <thread>
#include <functional>

#include "status.h"
#include "constructor_magic.h"
#include "concurrentqueue.h"
#include "async_watcher.h"
//#include "scheduled.hpp"

struct ev_loop;

namespace drpc {

class EventLoop : public StatusObserver {
public:
    using Functor = std::function<void()>;

public:
    EventLoop();

    ~EventLoop();

    void Run();

    void Stop();

    void RunInLoop(const Functor& task);

    void QueueInLoop(const Functor& task);

public:
    struct ev_loop* event_loop() const {
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

    void DoPendingTasks();

    bool PendingTaskQueueIsEmpty();

private:
    DISALLOW_COPY_AND_ASSIGN(EventLoop);

    struct ev_loop* event_loop_;

    std::unique_ptr<EventfdWatcher> async_watcher_;

    //scheduled_ptr sched_;

    std::thread::id thread_id_;

    moodycamel::ConcurrentQueue<Functor>* pending_task_queue_;

    std::atomic<bool> notified_;
};

} // namespace drpc

#endif // __EVENT_LOOP_H__
