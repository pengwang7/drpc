#include "logger.hpp"
#include "endpoint.hpp"
#include "duration.hpp"
#include "status_manager.hpp"
#include "byte_buffer.hpp"
#include "async_watcher.hpp"
#include "listener.hpp"
#include "event_loop.hpp"
#include "event_loop_group.hpp"
#include "channel.hpp"
#include "rpc_channel.hpp"
#include "rpc_packet.hpp"
#include "server.hpp"
#include "rpc.pb.h"
#include "service.pb.h"
#include "udp/udp_message.hpp"

using namespace drpc;

void test_endpoint() {
    Endpoint endpoint1("192.168.129.1:8089");
    Endpoint endpoint2("8.8.8.8", 9090);

    DDEBUG("endpoint1:{}.", endpoint1.ToString());
    DDEBUG("endpoint2:{}.", endpoint2.ToString());
}

void test_event_loop() {
    EventLoop* event_loop = new EventLoop();
    DASSERT(event_loop, "EventLoop is nil.");
    delete event_loop;
    event_loop = nullptr;
}

void test_event_loop_group() {
    EventLoopGroup* group = new EventLoopGroup(10, "test");
    DASSERT(group, "EventLoopGroup is nil.");

    group->Run();

    sleep(30);

    group->Stop();
    group->Wait();

    delete group;
    group = nullptr;
}

void test_listener() {
    Endpoint endpoint("127.0.0.1:6677");
    EventLoop* event_loop = new EventLoop();
    NetworkListener* listener = new TcpNetworkListener(event_loop, &endpoint);
    if (!listener) {
        return;
    }

    DDEBUG("TcpNetworkListener Start.");

    listener->Start();

    event_loop->Run();

    delete listener;
    delete event_loop;
}

void control_thread(Server* server) {
    if (!server) {
        return;
    }

    sleep(100);

    DDEBUG("control thread call stop.");

    server->Stop();
}

class PubSubServiceImpl : public PubSubService {
public:
	void Request(::google::protobuf::RpcController* controller,
		const ::drpc::SubscribeRequest* request,
		::drpc::SubscribeResponse* response,
		::google::protobuf::Closure* done) {

        // Sync model.
        ClosureGuard done_guard(done);

        DDEBUG("PubSubServiceImpl request message: {}", request->message());

        response->set_message("200OK");
	}

};

void test_server() {
	PubSubServiceImpl* pubsub_service = new PubSubServiceImpl();
	Server* server = new Server();

	server->AddService(pubsub_service);

	Options options;
	options.address = "127.0.0.1";
	options.port = 6689;
	options.threads = 4;

	std::thread th1(control_thread, server);

	server->Start(&options);

	th1.join();

	delete pubsub_service;
	delete server;
}

void test_udp_message() {
	Message* message = new Message(12, 1024);
	DASSERT(message, "The udp message is nil.");
}

int main() {
    Logger::Instance().Init();

    //test_endpoint();

    //memory_order_test();

    //test_event_loop();

    //test_event_loop_group();

    //test_listener();

    test_server();

    Logger::Instance().Destroy();

    return 0;
}
