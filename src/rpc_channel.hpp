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

#ifndef __RPC_CHANNEL_HPP__
#define __RPC_CHANNEL_HPP__

#include <map>
#include <google/protobuf/service.h>

#include "async_cbs.hpp"

namespace google {

namespace protobuf {

class Descriptor;
class ServiceDescriptor;
class MethodDescriptor;

class Closure;

class RpcController;

class Message;
class Service;

} // namespace protobuf

} // namespace google

namespace drpc {

enum ErrorCode : int;

class RpcChannel : public ::google::protobuf::RpcChannel {
public:
    using ServiceMap = std::map<std::string, google::protobuf::Service*>;

public:
    RpcChannel(const ChannelPtr& channel);

    virtual ~RpcChannel();

public:
    // Override.
    void CallMethod(const google::protobuf::MethodDescriptor* method,
                    google::protobuf::RpcController* controller,
                    const google::protobuf::Message* request,
                    google::protobuf::Message* response,
                    google::protobuf::Closure* done);

    void SetServiceMap(ServiceMap* services);

    void OnRpcMessage(const ChannelPtr& channel, Buffer* buffer);

private:
    void OnRpcJsonMessage(std::string content);

    void OnRpcProtoMessage(std::string content);

    bool OnRpcRequest(const RpcMessagePtr& rpc_message, enum ErrorCode& ec);

    bool OnRpcResponse();

    void OnRpcError(enum ErrorCode ec, uint64_t id);

    void Done(google::protobuf::Message* raw_message, uint64_t id);

private:
    ChannelPtr channel_;

    const RpcMessage* default_;

    ServiceMap* services_;
};

} // namespace drpc

#endif // __RPC_CHANNEL_HPP__
