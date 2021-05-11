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

#include <map>

#include "options.hpp"
#include "listener.hpp"
#include "closure_guard.hpp"
#include "event_loop_group.hpp"

namespace google {

namespace protobuf {

class Service;

} /* end namespace protobuf */

} /* end namespace google */

namespace drpc {

class Server : public StatusManager {
public:
    using DoneCallback = std::function<void()>;

public:
    Server();

    ~Server();

public:
    bool Start(Options* options);

    bool Stop();

    bool AddService(google::protobuf::Service* service);

public:
    void SetChannelCallback(const ChannelCallback& cb) {
        chan_cb_ = cb;
    }

    void SetMessageCallback(const MessageCallback& cb) {
        message_cb_ = cb;
    }

private:
    void Init();

    void StopLoopGroup();

    void StopInLoop(DoneCallback cb);

    void RemoveChannel(const ChannelPtr& channel);

    void HandleNewChannel(int fd, const std::string& raddr);

    void OnChannel(const ChannelPtr& channel);

    EventLoop* GetNextEventLoop();

private:
    DISALLOW_COPY_AND_ASSIGN(Server);

    using ServiceMap = std::map<std::string, google::protobuf::Service*>;

    using ChannelMap = std::map<uint64_t, ChannelPtr>;

    Options* options_;

    std::unique_ptr<Endpoint> endpoint_;

    std::unique_ptr<EventLoop> event_loop_;

    std::unique_ptr<NetworkListener> listener_;

    std::shared_ptr<EventLoopGroup> group_;

    uint64_t next_channel_id_;

    ServiceMap services_;

    ChannelMap channels_;

    ChannelCallback chan_cb_;

    MessageCallback message_cb_;
};

} // namespace drpc

#endif // __SERVER_HPP__
