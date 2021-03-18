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

struct ev_io;
struct ev_loop;

namespace drpc {

class EventLoop;

class AsyncWatcher {
public:
    using AsyncWatcherTaskFunctor = std::function<void()>;

public:
    AsyncWatcher(struct ev_loop* event_loop, AsyncWatcherTaskFunctor&& handle);

    virtual ~AsyncWatcher();

    bool Init();

    bool Watching();

    void Cancel();

    void Terminate();

protected:
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

private:
    bool DoInitImpl() override;

    void DoTerminateImpl() override;

    static void NotifyHandle(struct ev_loop* event_loop, struct ev_io* io, int events);

private:
    int event_fd_;
};

} // namespace drpc

#endif // __ASYNC_WATCHER_H__
