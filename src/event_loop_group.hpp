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

#ifndef __EVENT_LOOP_GROUP_HPP__
#define __EVENT_LOOP_GROUP_HPP__

#include <mutex>
#include <atomic>
#include <thread>
#include <vector>
#include <string>

namespace drpc {

class EventLoop;
class ServerOptions;

class EventLoopGroup {
public:
    EventLoopGroup(ServerOptions* options, std::string name);

    ~EventLoopGroup();

public:
    void Run(bool detach = true);

    void Stop();

    void Wait();

    EventLoop* event_loop();

    std::size_t size();

private:
    class Thread {
    public:
        Thread();

        ~Thread();

        void Run(bool detach);

        void Stop();

        void Wait();

        void Cycle();

        bool IsRunning() const;

        EventLoop* event_loop() const;

        std::thread::id thread_id() const;

    private:
        std::shared_ptr<std::thread> thread_;

        std::shared_ptr<EventLoop> event_loop_;

        std::mutex mutex_;

        enum {INITIALIZE, RUNNING, STOPPED};

        std::atomic<int> state_;

        bool detach_;
    };

private:
    std::size_t group_size_;

    std::string group_name_;

    std::mutex group_mutex_;

    std::atomic<std::size_t> group_cursor_;

    std::vector<std::shared_ptr<Thread>> thread_group_;
};

} /* end namespace drpc */

#endif /* __EVENT_LOOP_GROUP_HPP__ */
