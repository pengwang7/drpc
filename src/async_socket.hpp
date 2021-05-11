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

#ifndef __ASYNC_SOCKET_HPP__
#define __ASYNC_SOCKET_HPP__

#include <string>
#include <functional>

#include "constructor_magic.hpp"

namespace drpc {

const int kNoneEvent = 0x00;

const int kReadEvent = 0x02;

const int kWriteEvent = 0x04;

struct event;
class EventLoop;

// This class doesn't own the file descriptor, file descriptor
// could be a socket, an eventfd, a timerfd, or a signalfd.
class AsyncSocket {
public:
    using AsyncSocketCallback = std::function<void()>;

public:
    AsyncSocket(EventLoop* event_loop, int fd, int io_events);

    ~AsyncSocket();

public:
    bool Attach();

    bool Detach();

    void Close();

    void SetReadCallback(const AsyncSocketCallback& cb);

    void SetWriteCallback(const AsyncSocketCallback& cb);

    void ModifyIOEvents(bool readable, bool writable);

    void DisableAllIOEvents();

    bool IsAttached() const;

    bool IsNone() const;

    bool IsReadable() const;

    bool IsWritable() const;

    std::string EventsToString() const;

    EventLoop* event_loop() const;

    int fd() const;

private:
    void IOEventHandle(int fd, short which);

    static void IOEventHandle(int fd, short which, void* args);

private:
    DISALLOW_COPY_AND_ASSIGN(AsyncSocket);

    EventLoop* event_loop_;

    struct event* io_;

    int fd_;

    int events_;

    bool attached_;

    AsyncSocketCallback read_cb_;

    AsyncSocketCallback write_cb_;
};

} // namespace drpc

#endif // __ASYNC_SOCKET_HPP__
