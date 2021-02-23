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

#ifndef __THREAD_POOL_HPP__
#define __THREAD_POOL_HPP__

#include <list>
#include <mutex>
#include <memory>
#include <thread>
#include <functional>
#include <condition_variable>

#include "concurrentqueue.h"

namespace drpc {

class ThreadPoolInterface {
public:
    using Task = std::function<void()>;

public:
    virtual ~ThreadPoolInterface() {}

    virtual void Start() = 0;

    virtual void Stop() = 0;

    virtual void AddTask(const Task& task) = 0;

    virtual std::size_t GetTaskSize() const = 0;
};

class StaticThreadPool : public ThreadPoolInterface {
public:
    StaticThreadPool();

    ~StaticThreadPool();

public:
    void Start();

    void Stop();

    void AddTask(const Task& task);

    std::size_t GetTaskSize();
};

class DynamicThreadPool : public ThreadPoolInterface {
public:
    DynamicThreadPool();

    ~DynamicThreadPool();

public:
    void Start();

    void Stop();

    void AddTask(const Task& task);

    std::size_t GetTaskSize();

private:
    void Worker();

    void Manager();

private:
    std::size_t max_thds_;

    std::szie_t min_thds_;

    std::size_t idle_thds_;

    std::size_t busy_thds_;

    std::size_t wait_thds_;

    bool shutdown_;

    std::mutex lock_;

    std::condition_variable cond_;

    moodycamel::ConcurrentQueue<Task>* task_queue_;

    std::unique_ptr<std::thread> manager_thd_;

    std::list<std::unique_ptr<std::thread>> worker_thds_;

};

} /* end namespace drpc */

#endif /* __THREAD_POOL_HPP__ */
