#include "server_options.hpp"
#include "socket.hpp"
#include "ip_address.hpp"
#include "listener.hpp"
#include "event_loop.hpp"
#include "async_socket.hpp"
#include "channel.hpp"
#include "async_watcher.hpp"
#include "logger.hpp"
#include "buffer.hpp"
#include "byte_buffer.hpp"
#include "scheduled.hpp"
#include "timing_wheel.hpp"
#include "thread_pool.hpp"
#include "event_loop_group.hpp"
#include "rpc_channel.hpp"
#include "server.hpp"
//#include "rpc.pb.h"
#include "service.pb.h"

static int kStartFlag = 0;

class PublishServiceImpl : public drpc::PublishService {
public:
    PublishServiceImpl() {}

    virtual ~PublishServiceImpl() {}

public:
    virtual void Publish(::google::protobuf::RpcController* controller,
                         const ::drpc::PublishRequest* request,
                         ::drpc::PublishResponse* response,
                         ::google::protobuf::Closure* done) {

        // Sync model.
        drpc::ClosureGuard done_guard(done);

        drpc::DDEBUG("The message: %s", request->message().c_str());

        response->set_message("200OK");
    }
};

void thread_env(drpc::EventLoop* arg) {
    if (!arg) {
        return;
    }

    drpc::EventLoop* event_loop = static_cast<drpc::EventLoop*>(arg);

    drpc::EventfdWatcher* async = dynamic_cast<drpc::EventfdWatcher*>(event_loop->async_watcher());
    if (!async) {
        return;
    }

    kStartFlag = 1;

    drpc::DDEBUG("The thread will be start notify.");

    int cc = 0;

    while (true) {
        sleep(1);
        async->Notify();
        cc ++;

        if (cc >= 10) {
            event_loop->Stop();
            break;
        }
    }

    return;
}

void buffer_test() {
    drpc::Buffer* buf1 = new drpc::Buffer(36);
    if (!buf1) {
        drpc::DERROR("Buffer new failed.");
        return;
    }

    drpc::DDEBUG("Buffer new success.");

    if (buf1->empty()) {
        drpc::DDEBUG("Buffer is empty.");
    } else {
        drpc::DDEBUG("Buffer is not empty.");
    }

    drpc::DDEBUG("1 Buffer current size: %d, capacity: %d.", buf1->size(), buf1->capacity());

    buf1->AppendData("pengwang", 8);

    drpc::DDEBUG("2 Buffer current size: %d, capacity: %d.", buf1->size(), buf1->capacity());

    drpc::DDEBUG("Buffer data1: %s", buf1->data());

    drpc::DDEBUG("Buffer data2: %s", buf1->begin());

    std::string content = "66778899";
    buf1->AppendData(content);
/*
    char array[16] = {0};
    snprintf(array, sizeof(array), "12345678912345");
    buf1->AppendData(array);
*/
    buf1->AppendData("666666666666666666666666666666", 30);

    drpc::DDEBUG("3 Buffer current size: %d, capacity: %d.", buf1->size(), buf1->capacity());

    drpc::DDEBUG("Buffer data3: %s", buf1->data());

    drpc::DDEBUG("Buffer data4: %s", buf1->begin());

}

void channel_refs() {
    std::unique_ptr<drpc::EventLoop> event_loop;
    event_loop.reset(new drpc::EventLoop());

    drpc::AsyncSocket* socket = new drpc::AsyncSocket(event_loop.get(), 100, drpc::kReadEvent);

    drpc::channel_ptr chan(new drpc::Channel(socket));
    chan->Init();

    drpc::DDEBUG("channel refs: %d.", chan.use_count());
}

void timeout_cb() {
    drpc::DDEBUG("execute timeout_cb.");
}

void test_scheduled() {
    std::unique_ptr<drpc::EventLoop> event_loop;
    event_loop.reset(new drpc::EventLoop());
    if (!event_loop) {
        drpc::DERROR("The event_loop is nil.");
        return;
    }

    drpc::scheduled_ptr sched;
    {
        sched = drpc::Scheduled::CreateScheduled(event_loop.get(), timeout_cb, 10);
        if (!sched) {
            drpc::DERROR("Create sched failed.");
            return;
        }

        drpc::DDEBUG("The sched 111 use count: %d.", sched.use_count());
    }

    if (sched) {
        drpc::DDEBUG("The sched 222 use count: %d.", sched.use_count());
    } else {
        drpc::DDEBUG("The sched 333 use count: %d.", sched.use_count());
    }

    sched->Run();

    sched.reset();

    //event_loop->Run();
}

void test_event_loop_group() {
    drpc::DTRACE("test_event_loop_group begin.");

    drpc::ServerOptions options;
    options.address = "127.0.0.1";
    options.port = 6689;
    options.timeout = 20;
    options.server_mode = drpc::ServerMode::OLPT_NORMAL;

    std::unique_ptr<drpc::EventLoopGroup> group(new drpc::EventLoopGroup(&options, "test_group"));
    if (!group) {
        drpc::DERROR("The event loop group is nil.");
        return;
    }

    group->Run();


    sleep(12);

    group->Wait();

    group->Stop();

    drpc::DTRACE("test_event_loop_group end.");
}

void test_listener() {
    drpc::DTRACE("test_listener begin.");

    std::unique_ptr<drpc::EventLoop> event_loop;
    event_loop.reset(new drpc::EventLoop());
    if (!event_loop) {
        drpc::DERROR("The event_loop is nil.");
        return;
    }

    drpc::ServerOptions options;
    options.address = "127.0.0.1";
    options.port = 6689;
    options.timeout = 20;
    options.server_mode = drpc::ServerMode::OLPT_NORMAL;

    std::unique_ptr<drpc::Listener> listener;
    listener.reset(new drpc::Listener(event_loop.get(), &options));
    if (!listener) {
        drpc::DERROR("The listener is nil.");
        return;
    }

    listener->Start();
    listener->Stop();

    //event_loop->Run();

    drpc::DTRACE("test_listener end.");
}

void control_thread(drpc::Server* server) {
    if (!server) {
        drpc::DERROR("control thread server is nil.");
        return;
    }

    sleep(60);

    drpc::DDEBUG("control thread call stop.");

    server->Stop();
}

void server_test() {
    PublishServiceImpl* publish_service = new PublishServiceImpl();
    if (!publish_service) {
        return;
    }

    drpc::ServerOptions options;
    options.address = "127.0.0.1";
    options.port = 6689;
    options.timeout = 10;
    options.server_mode = drpc::ServerMode::OLPT_NORMAL;

    drpc::Server* server = new drpc::Server();

    server->AddService(publish_service);

    // Create a new thread for stop rpc server.
    std::thread th1(control_thread, server);

    if (server->Start(&options)) {
        drpc::DDEBUG("Server start success.");
    } else {
        drpc::DERROR("Server start failed.");
    }

    drpc::DDEBUG("server test before thread join.");

    th1.join();

    // Need to free.
    delete publish_service;
    delete server;

    drpc::DDEBUG("server test after thread join.");

    drpc::DTRACE("test_server end.");
}

#include "rpc.pb.h"

void proto_message_test() {
    drpc::DTRACE("proto_message_test begin.");

//    message RpcMessage
//    {
//        MessageType type = 1;
//        fixed64 id = 2;
//    
//        string service = 3;
//        string method = 4;
//        bytes request = 5;
//    
//        bytes response = 6;
//    
//        //optional ErrorCode error = 7;
//    }

    drpc::RpcMessage message;
    message.set_type(drpc::REQUEST);
    message.set_id(101);
    message.set_service("TestService");
    message.set_method("GetExten");
    message.set_request("exten: 1001, type: pjsip");

    std::string result;

    message.SerializeToString(&result);

    drpc::DDEBUG("The proto message serialize to string: %s, size: %d", result.c_str(), result.size());


    drpc::RpcMessage message2;
    message2.ParseFromString(result);

    drpc::DDEBUG("The string serialize to proto message: id: %d, service: %s, method: %s, request: %s",
                message2.id(), message2.service().c_str(), message2.method().c_str(), message2.request().c_str());

    drpc::DTRACE("proto_message_test end.");
}

void    test_rpc_msg_hdr() {
    drpc::rpc_msg_hdr hdr;
    memset(&hdr, 0, sizeof(hdr));
    hdr.version = 3;
    hdr.type = 1;
    hdr.padding = 0;

    printf("===hdr size:%lu\n", sizeof(hdr));
    char c;
    memcpy(&c, &hdr, 1);

    printf("cc11===%0x\n", (c & 0x02));
    printf("cc22===%0x\n", (c >> 2));

    printf("c====%0X\n", c);

    drpc::rpc_msg_hdr hdr2;
    memset(&hdr2, 0, sizeof(hdr2));
    memcpy(&hdr2, &c, 1);

    printf("hdr2:%d, %d\n", hdr2.version, hdr2.type);
}

int main() {
    drpc::Logger::Instance().Init();

//    test_scheduled();
//
//    test_event_loop_group();

//    test_listener();

    server_test();

//    test_rpc_msg_hdr();

//      proto_message_test();

#if 0
    //channel_refs();

    //return 0;

    //buffer_test();

    //return 0;

    std::unique_ptr<drpc::EventLoop> event_loop;
    event_loop.reset(new drpc::EventLoop());
    if (!event_loop) {
        drpc::DERROR("The event_loop is nil.");
        return -1;
    }


    std::thread t1(thread_env, event_loop.get());

    while (!kStartFlag) {
        sleep(2);
        continue;
    }

    drpc::DDEBUG("The event_loop will be run.");

    event_loop->Run();

    drpc::DDEBUG("The event_loop will be stop.");

    t1.join();
#endif
    drpc::Logger::Instance().Destroy();

    return 0;
}
