#ifndef __UDP_SERVER_HPP__
#define __UDP_SERVER_HPP__

#include "udp_message.hpp"
#include "event_loop.hpp"

namespace drpc {

class AsyncSocket;

class UdpServer {
public:
    UdpServer(EventLoop* event_loop, Endpoint* endpoint);

    ~UdpServer();

public:
    void Start();

private:
    EventLoop* event_loop_;

    std::unique_ptr<AsyncSocket> async_socket_;
};

class UdpServerManager {
public:
    UdpServerManager(EventLoop* event_loop);

    ~UdpServerManager();

public:
    void Start();

    void AddUdpServer(UdpServer* udp_server);

private:
    void Cycle();

private:
    std::unique_ptr<std::thread> loop_thread_;

    std::queue<UdpServer*> udp_servers_;
};

} // namespace drpc

#endif // __UDP_SERVER_HPP__
