#include <ev.h>

#include "logger.hpp"
#include "utility.hpp"
#include "event_loop.hpp"

namespace drpc {

EventLoop::EventLoop() {
    DTRACE("The event loop create:%p.", this);

    DoInit();
}

EventLoop::~EventLoop() {
    DTRACE("The event loop destroy:%p.", this);

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
    auto stop_task = [this]() -> void {
        // Do all tasks before stop event loop.
        while (true) {
            if (PendingTaskQueueIsEmpty()) {
                break;
            }

            DoPendingTasks();
        }

        // If ev_break is called on a different thread than EV_run, then the Event Loop does not exit.
        ev_break(event_loop_, EVBREAK_ALL);

        DDEBUG("The event loop is stopped:%p.", this);
    };

    SendToQueue(stop_task);
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
