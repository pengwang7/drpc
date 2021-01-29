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

#ifndef __SCHEDULED_HPP__
#define __SCHEDULED_HPP__

#include <ev.h>
#include "async_cbs.hpp"

namespace drpc {

typedef double ev_tstamp;

class EventLoop;

class Scheduled {
    using ScheduledFunctor = std::function<void()>;

public:
    ~Scheduled();

    static scheduled_ptr CreateScheduled(EventLoop* event_loop, ScheduledFunctor&& cb, ev_tstamp seconds, bool persist = true);

    static scheduled_ptr CreateScheduled(EventLoop* event_loop, const ScheduledFunctor& cb, ev_tstamp seconds, bool persist = true);

    void Run();

    void Cancel();

private:
    // Scheduled managent by shared_ptr,
    // so we set the constructor to private.
    Scheduled(EventLoop* event_loop, const ScheduledFunctor& cb, ev_tstamp seconds, bool persist);

    void OnTrigger();

    static void OnTrigger(EV_P_ ev_timer* timer, int events);

private:
    // Scheduled register event_loop.
    EventLoop* event_loop_;

    // The libev timer for scheduled.
	struct ev_timer* timer_;

    bool attached_;

    // Scheduled is persist or once.
    bool persist_;

    ev_tstamp delay_seconds_;

    // Hold itself.
    scheduled_ptr self_;

    // Timeout callback.
    ScheduledFunctor cb1_;

    // Cancel callback.
    ScheduledFunctor cb2_;
};

} /* end namespace drpc */

#endif /* __SCHEDULED_HPP__ */
