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

#include "logger.h"
#include "event_loop.h"
#include "timer_controller.h"

namespace drpc {

TimerController::TimerController(
    EventLoop* event_loop, const TimeoutFunctor& cb, uint32_t delay_sec, bool persist)
    : event_loop_(event_loop), delay_sec_(delay_sec),
      persist_(persist), timeout_cb_(cb) {

}

TimerControllerPtr TimerController::Create(EventLoop* event_loop,
        const TimeoutFunctor& cb, uint32_t delay_sec, bool persist) {
    TimerControllerPtr controller(new TimerController(event_loop, cb, delay_sec, persist));
    controller->self_holder_ = controller;

    return controller;
}

void TimerController::Run() {
    auto fn = [this]() {
        DDEBUG("TimerController Run.");
        auto weak_cnt = std::weak_ptr<TimerController>(shared_from_this());
        timer_watcher_.reset(new TimerEventWatcher(event_loop_,
        [weak_cnt]() {
            auto cnt = weak_cnt.lock();
            if (cnt) {
                cnt->OnTrigger();
            }
        }, delay_sec_, persist_));

        timer_watcher_->SetCancelCallback(
        [weak_cnt]() {
            auto cnt = weak_cnt.lock();
            if (cnt) {
                cnt->OnCancel();
            }
        }
        );

        DASSERT(timer_watcher_->Init(), "The timer watcher init failed.");

        DASSERT(timer_watcher_->Watching(), "The timer watcher watching failed.");
    };

    event_loop_->RunInLoop(fn);
}

void TimerController::Cancel() {
    auto weak_cnt = std::weak_ptr<TimerController>(shared_from_this());

    auto fn = [weak_cnt]() {
        auto cnt = weak_cnt.lock();
        if (cnt && cnt->timer_watcher_) {
            cnt->timer_watcher_->Cancel();
        }
    };

    event_loop_->RunInLoop(fn);
}

void TimerController::OnTrigger() {
    if (timeout_cb_) {
        timeout_cb_();
    }

    if (!persist_) {
        Cancel();
    }
}

void TimerController::OnCancel() {
    persist_ = false;

    DDEBUG("TimerController::OnCancel");

    timer_watcher_.reset();
    self_holder_.reset();
}

} // namespace drpc
