#include <stdio.h>

#include "event_loop.hpp"
#include "async_socket.hpp"
#include "channel.hpp"
#include "async_watcher.hpp"
#include "logger.hpp"
#include "buffer.hpp"
#include "byte_buffer.hpp"

static int kStartFlag = 0;

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

void    test_other() {
    char full_path[256] = {0};
    char config_path[128] = {0};
    char config_name[128] = {0};

    snprintf(full_path, sizeof(full_path), "http://www.baidu.com/abcderc/ww/./www/pengwang.tar.gz");
    char* q = NULL;
    char* p = strrchr(full_path, '/');
    if (!p) {
        return;
    }

    q = p;
    q ++;
    if (!q) {
        return;
    }

    *p = '\0';

    strncpy(config_path, full_path, sizeof(config_path));
    size_t l = strlen(config_path);
    snprintf(config_path + l, sizeof(config_path) - l, "/");
    strncpy(config_name, q, sizeof(config_name));

    drpc::DDEBUG("path:%s", config_path);
    drpc::DDEBUG("name:%s", config_name);
}

void channel_refs() {
    std::unique_ptr<drpc::EventLoop> event_loop;
    event_loop.reset(new drpc::EventLoop());

    drpc::AsyncSocket* socket = new drpc::AsyncSocket(event_loop.get(), 100, drpc::kReadEvent);

    drpc::channel_ptr chan(new drpc::Channel(event_loop.get(), socket));
    chan->Init();

    drpc::DDEBUG("channel refs: %d.", chan.use_count());
}

int main() {
    drpc::Logger::Instance().Init();

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

    drpc::Logger::Instance().Destroy();
    return 0;
}
