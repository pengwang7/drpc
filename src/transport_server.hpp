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

#ifndef __TRANSPORT_SERVER_HPP__
#define __TRANSPORT_SERVER_HPP__

#include "server_options.hpp"
#include "event_loop_group.hpp"

namespace drpc {

class Server {
public:
    Server();

    ~Server();

public:
    bool Start();

    bool Stop();

    bool AddService(::google::protobuf::Service* service);

    ServerOptions* GetServerOptions();

private:
    void OnConnection();

    void OnMessage();

private:
    ServerOptions* options_;

    std::unique_ptr<EventLoopGroup> group_;

    std::unique_ptr<Listener> listener_;

    std::unordered_map<std::string, ::google::protobuf::Service*> service_map_;
};

} /* end namespace drpc */

#endif /* __TRANSPORT_SERVER_HPP__ */
