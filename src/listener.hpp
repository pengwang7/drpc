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

#ifndef __LISTENER_HPP__
#define __LISTENER_HPP__

#include "utility.hpp"
#include "server_options.hpp"

namespace drpc {

class EventLoop;
class IPAddress;
class AsyncSocket;

using NewConnCallback = std::function<void(int fd, std::string& peer_addr)>;

class Listener {
public:
    Listener(EventLoop* event_loop, ServerOptions* options);

    ~Listener();

public:
    bool Start();

    bool Stop();

    void SetNewConnCallback(NewConnCallback cb);

private:
    void AcceptHandle();

private:
    EventLoop* event_loop_;

    std::unique_ptr<AsyncSocket> listen_socket_;

    ServerOptions* options_;

    NewConnCallback new_conn_cb_;
};

} /* end namespace drpc */

#endif /* __LISTENER_HPP__ */
