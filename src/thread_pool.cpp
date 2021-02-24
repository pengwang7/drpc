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

DynamicThreadPool::DynamicThreadPool(DynamicThreadPoolOptions* options) {
    options_ = options;
    shutdown_ = false;

    idle_thds_ = options_->min_thds;
    busy_thds_ = 0;
    wait_thds_ = 0;

    task_queue_ = new std::queue<Task>();
    DASSERT(task_queue_, "task queue is nil.");
}

DynamicThreadPool::~DynamicThreadPool() {
    worker_thds_.clear();
}

void DynamicThreadPool::Start() {
    for (std::size_t i = 0; i < idle_thds_; ++ i) {
        std::shared_ptr<std::thread> thd(new std::thread(std::bind(&DynamicThreadPool::Worker, this)));
        worker_thds_[thd->get_id()] = thd;
    }

    manager_thd_.reset(new std::thread(std::bind(&DynamicThreadPool::Manager, this)));
}

void DynamicThreadPool::Stop() {
    if (shutdown_) {
        DWARNING("The thread pool is already stop.");
        return;
    }

    {
        std::lock_guard<std::mutex> lock(mutex_);
        shutdown_ = true;
        cond_.notify_all();
    }

    for (auto& pair : worker_thds_) {
        if (pair.second->joinable()) {
            pair.second->join();
        }
    }
}

void DynamicThreadPool::AddTask(const Task& task) {

}

std::size_t DynamicThreadPool::GetTaskSize() {
    std::lock_guard<std::mutex> lock(mutex_);

    return task_queue_->size();
}

void DynamicThreadPool::Worker() {
    for (; ;) {
        std::unique_lock <std::mutex> guard(mutex_);
        while (task_queue_->size() == 0 && !shutdown_) {
            // Auto free lock.
            cond_.wait(guard);

            // Auto access lock.
            if (wait_thds_ > 0) {
                wait_thds_ --;
                if (idle_thds_ > options_->min_thds) {
                    idle_thds_ --;
                    DTRACE("The thread(%ld) exit.", std::this_thread::get_id());
                    guard.unlock();
                    return;
                }
            }
        }

        if (shutdown_) {
            DTRACE("The state is shutdown, thread(%ld) exit.", std::this_thread::get_id());
            guard.unlock();
            return;
        }

        Task task = task_queue_->front();
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

        std::unique_lock <std::mutex> guard(mutex_);

        if (all_size >= options_->min_pending_tasks && idle_thds_ < options_->max_thds) {
            for (std::size_t i = 0; i < options_->default_add_thds; ++ i) {

            }
        }

        //          pthread_mutex_lock(&m_pthreadMutex);
        //
        //          int add_threadnum = 0;
        //          for(int i = 0; i < Max_ThreadNum && add_threadnum < DEFAULT_ADD_THREAD_COUNTS; i ++)
        //          {
        //              if(pthread_id[i] == 0 || !pool.ThreadAlive(pthread_id[i]))
        //              {
        //                  pthread_create(&pthread_id[i], NULL, Thread_WorkerFunc, NULL);
        //                  add_threadnum ++;
        //                  Idle_ThreadNum ++;
        //
        //              }
        //          }
        //
        //          pthread_mutex_unlock(&m_pthreadMutex);

        std::size_t alive_thds = idle_thds_;
        std::size_t busy_thds = busy_thds_;

        if (alive_thds > 2 * busy_thds && alive_thds > options_->min_thds) {
            guard.lock();

            wait_thds_ = options_->default_del_thds;

            guard.unlock();

            for (std::size_t i = 0; i < options_->default_del_thds; ++ i) {
                cond_.notify_one();
            }
        }
    }
}

} /* end namespace drpc */
