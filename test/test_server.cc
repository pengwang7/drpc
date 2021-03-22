#include "server_options.h"
#include "socket.h"
#include "ip_address.h"
#include "listener.h"
#include "event_loop.h"
#include "async_socket.h"
#include "channel.h"
#include "async_watcher.h"
#include "logger.h"
#include "buffer.h"
#include "byte_buffer.h"
//#include "scheduled.h"
//#include "timing_wheel.h"
//#include "thread_pool.h"
#include "event_loop_group.h"
#include "rpc_channel.h"
#include "server.h"
#include "rpc.pb.h"
#include "service.pb.h"

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

void control_thread(drpc::Server* server) {
    if (!server) {
        drpc::DERROR("control thread server is nil.");
        return;
    }

    sleep(120);

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
    options.listener_mode = drpc::ListenerMode::LISTEN_ETH_NET;
    options.port = 6689;
    options.enable_check_timeout = false;
    options.timeout = 10;
    options.threads = 4;
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

void buffer_test() {
    std::string request_line = "POST / HTTP/1.1\r\n";
    std::string content_type = "Content-Type: json\r\n";
    std::string content_length = "Content-Length: 10\r\n\r\n";
    std::string content_body = "aaabbbcccd";

    drpc::Buffer buffer;

    buffer.AppendData(request_line.c_str(), request_line.size());
    buffer.AppendData(content_type.c_str(), content_type.size());
    buffer.AppendData(content_length.c_str(), content_length.size());
    buffer.AppendData(content_body.c_str(), content_body.size());

    drpc::DDEBUG("The buffer size:%d", buffer.size());
    drpc::DDEBUG("The buffer data:%s", buffer.data());

    const char* crlf = buffer.Find2CRLF();
    if (crlf) {
        drpc::DDEBUG("crlf:%s", crlf);
    } else {
        drpc::DERROR("not found crlf.");
    }

    drpc::ByteBufferReader reader(buffer);
    std::string res;
    reader.ReadString(&res, 55);

    drpc::DDEBUG("The res is:%s", res.c_str());

}

int main() {
    drpc::Logger::Instance().Init();

    buffer_test();

    //server_test();

    drpc::Logger::Instance().Destroy();

    return 0;
}
