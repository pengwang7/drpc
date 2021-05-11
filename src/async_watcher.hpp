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

#ifndef __ASYNC_WATCHER_HPP__
#define __ASYNC_WATCHER_HPP__

#include <functional>

#include "duration.hpp"

struct event;
struct event_base;

namespace drpc {

class EventLoop;

class AsyncWatcher {
public:
    using AsyncWatcherFunctor = std::function<void()>;

    virtual ~AsyncWatcher();

    bool Init();

    void Cancel();

    void SetCancelCallback(const AsyncWatcherFunctor& cb) {
        cancel_handle_ = cb;
    }

protected:
    AsyncWatcher(struct event_base* event_loop, AsyncWatcherFunctor&& handle);

    bool Watching(Duration duration);

    void Close();

    void FreeEvent();

    virtual bool DoInitImpl() = 0;

    virtual void DoCloseImpl() {}

protected:
    struct event_base* event_loop_;

    struct event* io_;

    bool attached_;

    AsyncWatcherFunctor task_handle_;

    AsyncWatcherFunctor cancel_handle_;
};


class EventfdWatcher : public AsyncWatcher {
public:
    EventfdWatcher(EventLoop* event_loop, AsyncWatcherFunctor&& handle);

    ~EventfdWatcher();

    bool AsyncWait();

    void Notify();

private:
    bool DoInitImpl() override;

    void DoCloseImpl() override;

    static void NotifyHandle(int fd, short events, void* args);

private:
    int event_fd_;
};

//class TimerEventWatcher : public AsyncWatcher {
//public:
//    TimerEventWatcher(EventLoop* event_loop, AsyncWatcherTaskFunctor&& handle, uint32_t delay_sec, bool persist);
//
//private:
//    bool DoInitImpl() override;
//
//    void DoTerminateImpl() override;
//
//    static void NotifyHandle(struct ev_loop* event_loop, struct ev_timer* timer, int events);
//
//private:
//    uint32_t delay_sec_;
//
//    bool persist_;
//};

} // namespace drpc

#endif // __ASYNC_WATCHER_HPP__
