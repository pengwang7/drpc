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

#ifndef __SERVER_H__
#define __SERVER_H__

#include <unordered_map>

#include "listener.h"
#include "closure_guard.h"
#include "event_loop_group.h"
#include "server_options.h"

namespace google {

namespace protobuf {

class Service;

} /* end namespace protobuf */

} /* end namespace google */

namespace drpc {

//class TimingWheel;

class Server : public StatusObserver {
public:
    Server();

    ~Server();

public:
    bool Start(ServerOptions* options);

    bool Stop();

    bool AddService(google::protobuf::Service* service);

    ServerOptions* GetServerOptions();

    EventLoop* GetListenerEventLoop();

private:
    bool DoInit(ServerOptions* options);

    void BuildChannel(int fd, std::string& peer_addr);

    void OnNewChannel(const channel_ptr& chan);

    void OnRefreshChannel(const channel_ptr& chan);

    void OnCloseChannel(const channel_ptr& chan);

    void OnTimeoutChannel(const channel_ptr& chan);

private:
    using RpcChannelHashTable = std::unordered_map<uint64_t, rpc_channel_ptr>;

    ServerOptions* options_;

    uint64_t next_channel_id_;

    std::mutex mutex_;

    std::mutex hash_mutex_;

    RpcChannelHashTable hash_table_;

    std::unique_ptr<EventLoopGroup> group_;

    std::unique_ptr<NetworkListener> network_listener_;

    std::unique_ptr<EventLoop> listener_event_loop_;

    TimerControllerPtr timer_controller_;

    //std::unique_ptr<EventLoop> timer_event_loop_;

    //std::unique_ptr<TimingWheel> timing_wheel_;

    std::unordered_map<std::string, google::protobuf::Service*> service_map_;
};

} // namespace drpc

#endif // __SERVER_H__
