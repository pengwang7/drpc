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

#include "async_cbs.h"
#include "rpc_msg_hdr.h"

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
class RpcMessage;

using ProMessagePtr = std::shared_ptr<google::protobuf::Message>;
using RpcMessagePtr = std::shared_ptr<RpcMessage>;
using RpcServiceHashTable = std::unordered_map<std::string, google::protobuf::Service*>;

class RpcChannel : public ::google::protobuf::RpcChannel {
public:
    RpcChannel(const channel_ptr& chan, RpcServiceHashTable* services);

    virtual ~RpcChannel();

public:
    // Override.
    void CallMethod(const google::protobuf::MethodDescriptor* method,
                    google::protobuf::RpcController* controller,
                    const google::protobuf::Message* request,
                    google::protobuf::Message* response,
                    google::protobuf::Closure* done);

    void OnRpcMessage(const channel_ptr& chan, Buffer& buffer);

    void SetRefreshCallback(const RefreshCallback& cb);

    void SetAnyContext(const any& context);

    any& GetAnyContext();

    void test_json_protobuf(RpcMessage* message);

private:
    bool ReadRpcHeader(Buffer& buffer, ByteBufferReader* io_reader);

    bool MessageTransform(Buffer& buffer, std::string& content);

    void SetMessageId(std::size_t msid);

    void OnRpcJsonMessage(std::string content);

    void OnRpcProtobufMessage(std::string content);

    bool OnRpcRequest(const RpcMessagePtr& rpc_message, enum ErrorCode& ec);

    bool OnRpcResponse(/*const RpcMessagePtr& rpc_message*/);

    void OnRpcRefresh();

    void OnRpcError(enum ErrorCode& ec);

    void Done(google::protobuf::Message* pro_message);

private:
    typedef struct {
        google::protobuf::Message* response;
        google::protobuf::Closure* done;
    } outstanding_call;

    // TODO: Not used.
    any context_;

    std::size_t msid_;

    channel_ptr chan_;

    rpc_msg_hdr msg_hdr_;

    RefreshCallback refresh_cb_;

    const RpcMessage* default_;

    std::map<std::size_t, outstanding_call> outstandings_;

    std::unordered_map<std::string, google::protobuf::Service*>* service_map_;
};

} // namespace drpc

#endif // __RPC_CHANNEL_H__
