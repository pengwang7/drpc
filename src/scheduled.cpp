#include <ev.h>

#include "logger.hpp"
#include "scheduled.hpp"
#include "event_loop.hpp"

namespace drpc {

Scheduled::Scheduled(EventLoop* event_loop, const ScheduledFunctor& cb, uint64_t seconds, bool persist) {
    DTRACE("Create Scheduled:%p.", this);

    event_loop_ = event_loop;
    timer_ = new(std::nothrow) ev_timer;
    delay_seconds_ = seconds;
    attached_ = false;
    persist_ = persist;
	cb1_ = cb;

    DASSERT(event_loop, "EventLoop is nil.");
    DASSERT(timer_, "The ev_timer is nil.");
}

Scheduled::~Scheduled() {
    DTRACE("Destroy Scheduled:%p.", this);

    delete timer_;
}

scheduled_ptr Scheduled::CreateScheduled(EventLoop* event_loop, ScheduledFunctor&& cb, uint64_t seconds, bool persist) {
    scheduled_ptr sched(new Scheduled(event_loop, std::move(cb), seconds, persist));
    DASSERT(sched, "Create Scheduled error.");

    sched->self_ = sched;

    return sched;
}

scheduled_ptr Scheduled::CreateScheduled(EventLoop* event_loop, const ScheduledFunctor& cb, uint64_t seconds, bool persist) {
    scheduled_ptr sched(new Scheduled(event_loop, cb, seconds, persist));
    DASSERT(sched, "Create Scheduled error.");

    sched->self_ = sched;

    return sched;
}

void Scheduled::Run() {
    DTRACE("Scheduled will be run.");

    auto fn = [&]() -> void {
        if (attached_) {
            ev_timer_stop(event_loop_->event_loop(), timer_);
        }

        if (persist_) {
            ev_timer_init(timer_, OnTrigger, delay_seconds_, delay_seconds_);
        } else {
            ev_timer_init(timer_, OnTrigger, delay_seconds_, 0);
        }

        timer_->data = static_cast<void*>(this);

        ev_timer_start(event_loop_->event_loop(), timer_);

        attached_ = true;
    };

    event_loop_->SendToQueue(fn);
}

void Scheduled::Cancel() {
    DTRACE("Scheduled will be cancel.");

    auto fn = [&]() -> void {
        if (attached_) {
            ev_timer_stop(event_loop_->event_loop(), timer_);
        }

        attached_ = false;

        cb1_ = ScheduledFunctor();
        cb2_ = ScheduledFunctor();

        self_.reset();
    };

    event_loop_->SendToQueue(fn);
}

void Scheduled::OnTrigger() {
    if (cb1_) {
        cb1_();
    }

    if (persist_) {
        return;
    }

    attached_ = false;

    cb1_ = ScheduledFunctor();
    cb2_ = ScheduledFunctor();

    self_.reset();
}

void Scheduled::OnTrigger(EV_P_ ev_timer* timer, int events) {
    if (events & EV_ERROR) {
        DERROR("OnTrigger error.");
        return;
    }

    Scheduled* sched = static_cast<Scheduled*>(timer->data);
    if (!sched) {
        DERROR("The sched is nil.");
        return;
    }

    sched->OnTrigger();
}

} /* end namespace drpc */
