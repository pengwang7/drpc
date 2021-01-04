#include "event_loop.hpp"
#include "async_socket.hpp"
#include "channel.hpp"
#include "async_watcher.hpp"
#include "logger.hpp"
#include "buffer.hpp"
#include "byte_buffer.hpp"
#include "scheduled.hpp"

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

void channel_refs() {
    std::unique_ptr<drpc::EventLoop> event_loop;
    event_loop.reset(new drpc::EventLoop());

    drpc::AsyncSocket* socket = new drpc::AsyncSocket(event_loop.get(), 100, drpc::kReadEvent);

    drpc::channel_ptr chan(new drpc::Channel(event_loop.get(), socket));
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

int main() {
    drpc::Logger::Instance().Init();

    test_scheduled();

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
