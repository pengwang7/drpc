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

#ifndef __SERVER_HPP__
#define __SERVER_HPP__

#include <unordered_map>

#include "closure_guard.hpp"
#include "server_options.hpp"
#include "event_loop_group.hpp"

namespace google {

namespace protobuf {

class Service;

} /* end namespace protobuf */

} /* end namespace google */

namespace drpc {

class Listener;

class Server {
public:
    Server();

    ~Server();

public:
    bool Start(ServerOptions* options);

    bool Stop();

    bool AddService(google::protobuf::Service* service);

    ServerOptions* GetServerOptions();

    //State* GetServerState();

public:
//    class ChannelManager {
//    public:
//        void Insert(const channel_ptr chan);
//
//        void Delete(const channel_ptr chan);
//    };

private:
    bool DoInit(ServerOptions* options);

    void BuildChannel(int fd, std::string& peer_addr);

    void OnNewChannel(const channel_ptr& chan);

    void OnCloseChannel(const channel_ptr& chan);

    void OnTimeoutChannel(const channel_ptr& chan);

    void OnMessage(const channel_ptr& chan, Buffer& buffer);

private:
    using ChannelHashTable = std::unordered_map<std::string, channel_ptr>;

    ServerOptions* options_;

    std::unique_ptr<EventLoopGroup> group_;

    std::unique_ptr<Listener> listener_;

    std::unique_ptr<EventLoop> listener_event_loop_;

    std::unique_ptr<ChannelHashTable> chan_table_;

    std::unordered_map<std::string, google::protobuf::Service*> service_map_;
};

} /* end namespace drpc */

#endif /* __SERVER_HPP__ */
