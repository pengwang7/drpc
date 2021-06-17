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

#ifndef __UDP_SERVER_HPP__
#define __UDP_SERVER_HPP__

#include "async_socket.hpp"
#include "udp_message.hpp"
#include "event_loop.hpp"

namespace drpc {

class AsyncSocket;

// UdpServer class own the file descriptor.
class UdpServer {
public:
    using UdpSocketReadCallback = std::function<void()>;

public:
    UdpServer(EventLoop* event_loop, Endpoint* endpoint, std::string id);

    ~UdpServer();

public:
    void Start();

    void Stop();;

public:
    void SetReadCallback(const UdpSocketReadCallback& cb) {
        //async_socket_->SetReadCallback(cb);
    }

    std::string unique_id() const {
        return unique_id_;
    }

private:
    void ReadMessageCallback();

private:
    EventLoop* event_loop_;

    int fd_;

    std::string unique_id_;

    std::unique_ptr<AsyncSocket> async_socket_;

    UdpSocketReadCallback udp_read_cb_;
};

class UdpServerManager {
public:
    UdpServerManager();

    ~UdpServerManager();

public:
    void Start();

    void Stop();

    void Wait();

    void AddUdpServer(UdpServer* udp_server);

    void DelUdpServer(UdpServer* udp_server);

    UdpServer* FindUdpServerById(std::string id);

    EventLoop* event_loop() const {
        return event_loop_;
    }

private:
    void Cycle();

private:
    EventLoop* event_loop_;

    std::unique_ptr<std::thread> loop_thread_;

    std::vector<UdpServer*> udp_servers_;

    std::mutex mutex_;
};

} // namespace drpc

#endif // __UDP_SERVER_HPP__
