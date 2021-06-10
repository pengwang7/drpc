#include "logger.hpp"
#include "udp_server.hpp"

namespace drpc {

class UdpServer {
public:
    UdpServer(EventLoop* event_loop, Endpoint* endpoint);

    ~UdpServer();

public:
    void Start();

private:
    EventLoop* event_loop_;
};

UdpServer::UdpServer(EventLoop* event_loop, Endpoint* endpoint) {

}

UdpServer::~UdpServer() {

}

void UdpServer::Start() {

}


UdpServerManager::UdpServerManager(EventLoop* event_loop)
    : event_loop_(event_loop) {
    DASSERT(event_loop_ != nullptr, "event_loop_ is nil.");
}

UdpServerManager::~UdpServerManager() {

}

void UdpServerManager::Start() {
    if (loop_thread_ != nullptr) {
        return;
    }

    loop_thread_.reset(std::thread(std::bind(&UdpServerManager::Cycle, this)));
}

void UdpServerManager::AddUdpServer(UdpServer* udp_server) {
    if (!udp_server) {
        DERROR("AddUdpServer udp_server is nil.");
        return;
    }

    udp_servers_.push(udp_server);
}

void UdpServerManager::Cycle() {
    event_loop_->Run();
}

} // namespace drpc
