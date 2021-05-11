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

#include <google/protobuf/descriptor.h>

#include "rpc.pb.h"
#include "json2pb/pb_to_json.h"
#include "json2pb/json_to_pb.h"
#include "json2pb/encode_decode.h"
#include "logger.hpp"
#include "channel.hpp"
#include "rpc_packet.hpp"
#include "rpc_channel.hpp"

namespace drpc {

RpcChannel::RpcChannel(const ChannelPtr& channel) : channel_(channel) {
    DASSERT(channel_, "RpcChannel create failed.");
}

RpcChannel::~RpcChannel() {
    DTRACE("RpcChannel destroy.");
}

void RpcChannel::CallMethod(const google::protobuf::MethodDescriptor* method,
                            google::protobuf::RpcController* controller,
                            const google::protobuf::Message* request,
                            google::protobuf::Message* response,
                            google::protobuf::Closure* done) {
    // TODO:
}

void RpcChannel::SetServiceMap(ServiceMap* services) {
    services_ = services;
}

// NOTE: In here, we can't be sure that there is only one message in the buffer,
// so we have to process all the complete messages.
void RpcChannel::OnRpcMessage(const ChannelPtr& channel, Buffer* buffer) {
    std::unique_ptr<ByteBufferReader> io_reader(new ByteBufferReader(*buffer));
    if (!io_reader) {
        DERROR("OnMessage new ByteBufferReader failed.");
        return;
    }

    RpcPacket* packet = nullptr;

    // Process all messages in the buffer.
    while ((packet = RpcPacket::Parse(*buffer, io_reader.get()))) {
        switch (packet->GetPayloadType()) {
        case PAYLOAD_TYPE_JSON:
            OnRpcJsonMessage(packet->GetBody());
            break;
        case PAYLOAD_TYPE_PROTO:
            OnRpcProtoMessage(packet->GetBody());
            break;
        default:
            DERROR("RpcPacket with invalid payload type({}).", packet->GetPayloadType());
        };

        delete packet;
    }
}

void RpcChannel::OnRpcJsonMessage(std::string content) {
    bool ret = false;
    ErrorCode ec = INVALID_REQUEST;
    std::shared_ptr<RpcMessage> rpc_message(default_->New());
    if (!rpc_message) {
        return;
    }

    if (!json2pb::JsonToProtoMessage(content, rpc_message.get())) {
        DERROR("OnRpcJsonMessage json to protobuf failed.");
        goto error;
    }

    DDEBUG("The json serialize to proto message: id: {}, service: {}, method: {}, request: {}",
           rpc_message->id(), rpc_message->service().c_str(), rpc_message->method().c_str(),
           rpc_message->request().c_str());

    switch (rpc_message->type()) {
    case REQUEST:
        ret = OnRpcRequest(rpc_message, ec);
        break;

    case RESPONSE:
        ret = OnRpcResponse();
        break;

    default:
        DERROR("OnRpcJsonMessage the message type is invalid.");
    };

    if (ret && ec == NO_ERROR) {
        return;
    }

error:
    OnRpcError(ec, rpc_message->id());

    DERROR("OnRpcJsonMessage met error: {}.", ec);
}

void RpcChannel::OnRpcProtoMessage(std::string content) {
    bool ret = false;
    ErrorCode ec = INVALID_REQUEST;
    std::shared_ptr<RpcMessage> rpc_message(default_->New());
    if (!rpc_message) {
        return;
    }

    if (!rpc_message->ParseFromString(content)) {
        DERROR("OnRpcProtobufMessage protobuf parse from string failed.");
        goto error;
    }

    DDEBUG("The string serialize to proto message: id: {}, "
           "service: {}, method: {}, request: {}",
           rpc_message->id(), rpc_message->service(),
           rpc_message->method(), rpc_message->request());

    switch (rpc_message->type()) {
    case REQUEST:
        ret = OnRpcRequest(rpc_message, ec);
        break;

    case RESPONSE:
        ret = OnRpcResponse();
        break;

    default:
        DERROR("OnRpcProtobufMessage the message type is invalid.");
    };

    if (ret && ec == NO_ERROR) {
        return;
    }

error:
    OnRpcError(ec, rpc_message->id());

    DERROR("OnRpcProtobufMessage met error: {}.", ec);
}

bool RpcChannel::OnRpcRequest(const RpcMessagePtr& rpc_message, enum ErrorCode& ec) {
    auto it = services_->find(rpc_message->service());
    if (it != services_->end()) {
        google::protobuf::Service* service = it->second;
        if (!service) {
            DERROR("OnRpcRequest the service is nil.");
            ec = NO_SERVICE;
            return false;
        }

        auto* sdes = service->GetDescriptor();
        auto* method = sdes->FindMethodByName(rpc_message->method());
        if (!method) {
            DERROR("OnRpcRequest {} service not found {} method.",
                   rpc_message->service(), rpc_message->method());
            ec = NO_METHOD;
            return false;
        }

        std::unique_ptr<google::protobuf::Message> request(service->GetRequestPrototype(method).New());
        if (request->ParseFromString(rpc_message->request())) {
            // Release response in Done callback.
            google::protobuf::Message* response = service->GetResponsePrototype(method).New();
            service->CallMethod(method, NULL, request.get(), response,
                                google::protobuf::NewCallback(this, &RpcChannel::Done, response, rpc_message->id()));
            ec = NO_ERROR;
            return true;
        }
    } else {
        DERROR("OnRpcRequest {} service not found.", rpc_message->service());
        ec = NO_SERVICE;
    }

    return false;
}

bool RpcChannel::OnRpcResponse() {
    // TODO:
    return true;
}

void RpcChannel::OnRpcError(enum ErrorCode ec, uint64_t id) {
    RpcMessage rpc_message;
    rpc_message.set_type(RESPONSE);
    rpc_message.set_id(id);
    rpc_message.set_response(ErrorCode_Name(ec));
    rpc_message.set_error(ec);

    std::string data;
    if (!rpc_message.SerializeToString(&data)) {
        DERROR("OnRpcError the rpc_message serialize to string failed.");
        return;
    }

    channel_->SendMessage(data);
}

void RpcChannel::Done(google::protobuf::Message* raw_message, uint64_t id) {
    std::unique_ptr<google::protobuf::Message> message(raw_message);

    RpcMessage rpc_message;
    rpc_message.set_type(RESPONSE);
    rpc_message.set_id(id);
    rpc_message.set_response(message->SerializeAsString());

    std::string data;
    if (!rpc_message.SerializeToString(&data)) {
        DERROR("Done the rpc_message serialize to string failed.");
        return;
    }

    channel_->SendMessage(data);
}

} // namespace drpc
