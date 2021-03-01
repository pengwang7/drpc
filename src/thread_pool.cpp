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

#include "logger.hpp"
#include "thread_pool.hpp"

namespace drpc {

StaticThreadPool::StaticThreadPool() {

}

StaticThreadPool::~StaticThreadPool() {

}

void StaticThreadPool::Start() {

}

void StaticThreadPool::Stop() {

}

void StaticThreadPool::AddTask(const Task& task) {

}

std::size_t StaticThreadPool::GetTaskSize() {
    return 0;
}

DynamicThreadPool::DynamicThread::DynamicThread(DynamicThreadPool* thread_pool)
    : thread_pool_(thread_pool), thd_(new std::thread(&DynamicThreadPool::DynamicThread::Worker, this)) {

}

DynamicThreadPool::DynamicThread::~DynamicThread() {
    DTRACE("Join thread(%ld).", thd_->get_id());
    thd_->join();
}

void DynamicThreadPool::DynamicThread::Worker() {
    thread_pool_->Worker();

    std::unique_lock<std::mutex> guard(thread_pool_->mutex_);

    thread_pool_->idle_thds_ --;

    thread_pool_->dead_thds_.push_back(this);

    if (thread_pool_->shutdown_ && !thread_pool_->idle_thds_) {
        guard.unlock();
        thread_pool_->cond_.notify_one();
        DDEBUG("Notify thread pool exit.");
    }
}

DynamicThreadPool::DynamicThreadPool(DynamicThreadPoolOptions* options)
    : options_(options), shutdown_(false), idle_thds_(0), busy_thds_(0), wait_thds_(0) {
    task_queue_ = new std::queue<Task>();
    DASSERT(task_queue_, "task queue is nil.");
}

DynamicThreadPool::~DynamicThreadPool() {
    DASSERT(task_queue_->size() == 0, "The task queue size is not 0.");
    delete task_queue_;
}

void DynamicThreadPool::Start() {
    std::lock_guard<std::mutex> guard(mutex_);
    for (std::size_t i = 0; i < options_->min_thds; ++ i) {
        idle_thds_ ++;
        new DynamicThread(this);
    }

    manager_thd_.reset(new std::thread(std::bind(&DynamicThreadPool::Manager, this)));
}

void DynamicThreadPool::Stop() {
    if (shutdown_) {
        DWARNING("The thread pool is already stop.");
        return;
    }

    {
        std::unique_lock<std::mutex> guard(mutex_);
        shutdown_ = true;
    }

    cond_.notify_all();

    std::unique_lock<std::mutex> guard(mutex_);

    while (idle_thds_ != 0) {
        cond_.wait(guard);
    }

    for (auto t = dead_thds_.begin(); t != dead_thds_.end(); t = dead_thds_.erase(t)) {
        delete (*t);
    }

    if (manager_thd_->joinable()) {
        DDEBUG("The join manager thread(%ld).", manager_thd_->get_id());
        manager_thd_->join();
    }
}

void DynamicThreadPool::AddTask(const Task& task) {
    {
        std::lock_guard<std::mutex> guard(mutex_);
        task_queue_->push(task);
    }

    cond_.notify_one();
}

std::size_t DynamicThreadPool::GetTaskSize() {
    std::lock_guard<std::mutex> guard(mutex_);

    return task_queue_->size();
}

void DynamicThreadPool::Worker() {
    for (; ;) {
        std::unique_lock<std::mutex> guard(mutex_);
        while (task_queue_->empty() && !shutdown_) {
            cond_.wait(guard);
        }

        if (shutdown_) {
            DTRACE("The state is shutdown, thread(%ld) exit.", std::this_thread::get_id());
            break;
        }

        auto task = task_queue_->front();
        task_queue_->pop();

        busy_thds_ ++;

        guard.unlock();

        task();

        guard.lock();

        busy_thds_ --;

        guard.unlock();
    }
}

void DynamicThreadPool::Manager() {
    while (!shutdown_) {
        sleep(options_->manager_rate);

        DDEBUG("The manager thread(%ld) is doing!", std::this_thread::get_id());

        std::size_t all_size = GetTaskSize();

        std::lock_guard <std::mutex> guard(mutex_);

        DDEBUG("%d, %d, %d, %d", all_size,  options_->min_pending_tasks, idle_thds_, options_->max_thds);

        if (all_size >= options_->min_pending_tasks && idle_thds_ < options_->max_thds) {
            for (std::size_t i = 0; i < options_->default_add_thds; ++ i) {
                new DynamicThread(this);

                idle_thds_ ++;
            }
        }

        if (!shutdown_) {
            break;
        }

//        std::size_t alive_thds = idle_thds_;
//        std::size_t busy_thds = busy_thds_;
//
//        if (alive_thds > 2 * busy_thds && alive_thds > options_->min_thds) {
//            DDEBUG("2222222222222222222222222222222222222222");
//            guard.lock();
//
//            wait_thds_ = options_->default_del_thds;
//
//            guard.unlock();
//
//            for (std::size_t i = 0; i < options_->default_del_thds; ++ i) {
//                cond_.notify_one();
//            }
//        }
    }
}

} /* end namespace drpc */
