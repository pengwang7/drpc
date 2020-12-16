#ifndef __EVENT_LOOP_HPP__
#define __EVENT_LOOP_HPP__

#include <atomic>
#include <thread>
#include <functional>

#include "concurrentqueue.h"
#include "async_watcher.hpp"

struct ev_loop;

namespace drpc {

class EventLoop {
public:
    using TaskFunctor = std::function<void()>;

public:
    EventLoop();

    ~EventLoop();

    void Run();

    void Stop();

    void SendToQueue(const TaskFunctor& task);

    void SendToQueue(TaskFunctor&& task);

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

    // For test
    AsyncWatcher* async_watcher() {
        return async_watcher_.get();
    }

private:
    void DoInit();

    void DoPendingTasks();

    bool PendingTaskQueueIsEmpty();

private:
    struct ev_loop* event_loop_;

    std::unique_ptr<EventfdWatcher> async_watcher_;

    std::thread::id thread_id_;

    moodycamel::ConcurrentQueue<TaskFunctor>* pending_task_queue_;

    std::atomic<bool> notified_;
};

} /* end namespace drpc */

#endif /* __EVENT_LOOP_HPP__ */
