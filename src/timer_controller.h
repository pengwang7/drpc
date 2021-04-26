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

#ifndef __TIMER_CONTROLLER_H__
#define __TIMER_CONTROLLER_H__

#include <memory>
#include <functional>

namespace drpc {

class EventLoop;
class TimerEventWatcher;
class TimerController;

using TimerControllerPtr = std::shared_ptr<TimerController>;

class TimerController : public std::enable_shared_from_this<TimerController> {
public:
    using TimeoutFunctor = std::function<void()>;

    static TimerControllerPtr Create(EventLoop* event_loop, const TimeoutFunctor& cb, uint32_t delay_sec, bool persist);

    void Run();

    void Cancel();

private:
    TimerController(EventLoop* event_loop, const TimeoutFunctor& cb, uint32_t delay_sec, bool persist);

    void OnTrigger();

    void OnCancel();

private:
    EventLoop* event_loop_;

    std::shared_ptr<TimerController> self_holder_;

    std::unique_ptr<TimerEventWatcher> timer_watcher_;

    uint32_t delay_sec_;

    bool persist_;

    TimeoutFunctor timeout_cb_;
};

} // namespace drpc

#endif // __TIMER_CONTROLLER_H__
