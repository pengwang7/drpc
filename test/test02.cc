#include "logger.hpp"
#include "server.hpp"
#include "rpc.pb.h"
#include "service.pb.h"

#include "udp/udp_server.hpp"

class BfcpServiceImpl : public drpc::BfcpService {
public:
    BfcpServiceImpl() {
        event_loop_.reset(new drpc::EventLoop());
        DASSERT(event_loop_ != nullptr, "event_loop_ is nil.");
    }

    virtual ~BfcpServiceImpl() {}

public:
    void RequestBfcpPort(::google::protobuf::RpcController* controller,
                    const ::drpc::BfcpServiceRequest* request,
                    ::drpc::BfcpServiceResponse* response,
                    ::google::protobuf::Closure* done) {

    }

    void RequestBfcpMethod(::google::protobuf::RpcController* controller,
                    const ::drpc::BfcpServiceRequest* request,
                    ::drpc::BfcpServiceResponse* response,
                    ::google::protobuf::Closure* done) {

    }


private:
    std::shared_ptr<drpc::EventLoop> event_loop_;
};

struct PortRange {
public:
    PortRange() {
        port_0 = 60000;
        port_1 = 65535;
    }

    int port_0;
    int port_1;
};

int main() {
    drpc::Logger::Instance().Init();

    drpc::UdpServerManager* udp_manager = new drpc::UdpServerManager();
    if (!udp_manager) {
        DERROR("Create UdpManager failed.");
        return -1;
    }

    udp_manager->Start();

    PortRange port_range;
    for (int i = 0; i < 2; ++ i) {
        std::string unique_id = "testi=" + std::to_string(i);
        drpc::Endpoint ep("0.0.0.0", port_range.port_0 + i);
        DTRACE("the unique id={}.", unique_id);
        drpc::UdpServer* udp_server = new drpc::UdpServer(udp_manager->event_loop(), &ep, unique_id);
        if (!udp_server) {
            continue;
        }

        udp_manager->AddUdpServer(udp_server);

//        sleep(1);
//
//        udp_manager->DelUdpServer(udp_server);
    }

    for (int i = 0; i < 51; ++ i) {
        sleep(20);
    }

    udp_manager->Stop();

    delete udp_manager;

    drpc::Logger::Instance().Destroy();

    return 0;
}
