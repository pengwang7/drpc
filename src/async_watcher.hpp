#ifndef __ASYNC_WATCHER_HPP__
#define __ASYNC_WATCHER_HPP__

#include <functional>

struct ev_io;
struct ev_loop;

namespace drpc {

class EventLoop;

class AsyncWatcher {
public:
    using AsyncWatcherTaskFunctor = std::function<void()>;

public:
    virtual ~AsyncWatcher();

    bool Init();

    bool Watching();

    void Cancel();

    void Terminate();

protected:
    AsyncWatcher(struct ev_loop* event_loop, AsyncWatcherTaskFunctor&& handle);

    virtual bool DoInitImpl() = 0;

    virtual void DoTerminateImpl() = 0;

protected:
    struct ev_loop* event_loop_;

    struct ev_io* io_;

    AsyncWatcherTaskFunctor task_handle_;

    AsyncWatcherTaskFunctor cancel_handle_;

    bool attached_;
};


class EventfdWatcher : public AsyncWatcher {
public:
    EventfdWatcher(EventLoop* event_loop, AsyncWatcherTaskFunctor&& handle);

    ~EventfdWatcher();

    void Notify();

    int event_fd();

private:
    bool DoInitImpl() override;

    void DoTerminateImpl() override;

    static void NotifyHandle(struct ev_loop* event_loop, struct ev_io* io, int events);

private:
    int event_fd_;
};

class TimerEventWatcher : public AsyncWatcher {

};

} /* end namespace drpc */

#endif /* __ASYNC_WATCHER_HPP__ */
