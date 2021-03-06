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
#include "endpoint.hpp"

namespace drpc {

class EventLoop;
class AsyncSocket;

using NewConnCallback = std::function<void(int fd, std::string& peer_addr)>;

// This class implement a listener to listen address(ip + port) on the network.
class NetworkListener {
public:
    NetworkListener(EventLoop* event_loop, Endpoint* endpoint);

    virtual ~NetworkListener();

    virtual bool Start() = 0;

    virtual bool Stop() = 0;

    void SetNewConnCallback(NewConnCallback cb) {
        new_conn_cb_ = cb;
    }

protected:
    void SetListenerSocketOptions(int fd);

    void AcceptHandle();

protected:
    EventLoop* event_loop_;

    std::unique_ptr<AsyncSocket> socket_;

    Endpoint* endpoint_;

    NewConnCallback new_conn_cb_;
};

class BasicNetworkListener : public NetworkListener {
public:
    BasicNetworkListener(EventLoop* event_loop, Endpoint* endpoint);

    ~BasicNetworkListener();

public:
    bool Start() override;

    bool Stop() override;
};

class TcpNetworkListener : public NetworkListener {
public:
    TcpNetworkListener(EventLoop* event_loop, Endpoint* endpoint);

    ~TcpNetworkListener();

public:
    bool Start() override;

    bool Stop() override;
};

} // namespace drpc

#endif // __LISTENER_HPP__
