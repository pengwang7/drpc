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
#include "logger.h"
#include "channel.h"
#include "rpc_channel.h"

namespace drpc {

RpcChannel::RpcChannel(const channel_ptr& chan, RpcServiceHashTable* services)
    : msid_(0), chan_(chan), default_(&RpcMessage::default_instance()), service_map_(services) {
    memset(&msg_hdr_, 0, sizeof(msg_hdr_));
    DASSERT(chan_, "RpcChannel create failed.");
}

RpcChannel::~RpcChannel() {
    DTRACE("The RpcChannel destroy csid: {}, msid: {}.", chan_->csid(), msid_);

    // When the RpcChannel for rpc client, need to used outstandings_.
    for (auto it = outstandings_.begin(); it != outstandings_.end(); ++ it) {
        outstanding_call out_call = it->second;
        delete out_call.response;
        delete out_call.done;
    }
}

void RpcChannel::CallMethod(const google::protobuf::MethodDescriptor* method,
                            google::protobuf::RpcController* controller,
                            const google::protobuf::Message* request,
                            google::protobuf::Message* response,
                            google::protobuf::Closure* done) {
    // TODO: this function for rpc client.
}

// NOTE: In here, we can't be sure that there is only one message in the buffer,
// so we have to process all the complete messages.
void RpcChannel::OnRpcMessage(const channel_ptr& chan, Buffer& buffer) {
    std::unique_ptr<ByteBufferReader> io_reader(new ByteBufferReader(buffer));
    if (!io_reader) {
        DERROR("MessageTransform new ByteBufferReader failed.");
        return;
    }

    RpcPacket* packet = nullptr;

    // Process all messages in buffer.
    while ((packet = RpcPacket::Parse(buffer, io_reader.get()))) {
        switch (packet->GetPayloadType()) {
        case PAYLOAD_TYPE_JSON:
            OnRpcJsonMessage(packet->GetBody());
            break;
        case PAYLOAD_TYPE_PROTOBUF:
            OnRpcProtobufMessage(packet->GetBody());
            break;
        default:
            DERROR("RpcPacket with invalid payload type({}).", packet->GetPayloadType());
        };

        delete packet;
    }
}

//void RpcChannel::OnRpcMessage(const channel_ptr& chan, Buffer& buffer) {
//    std::string content;
//    if (!MessageTransform(buffer, content)) {
//        DWARNING("OnRpcMessage message transform failed.");
//        return;
//    }
//
//    if (msg_hdr_.type == MSG_CONTENT_TYPE_JSON) {
//        OnRpcJsonMessage(content);
//    } else if (msg_hdr_.type == MSG_CONTENT_TYPE_PROTOBUF) {
//        OnRpcProtobufMessage(content);
//    } else {
//        DWARNING("OnRpcMessage invalid message type.");
//    }
//}

void RpcChannel::SetRefreshCallback(const RefreshCallback& cb) {
    refresh_cb_ = cb;
}

void RpcChannel::SetAnyContext(const any& context) {
    context_ = context;
}

any& RpcChannel::GetAnyContext() {
    return context_;
}

bool RpcChannel::ReadRpcHeader(Buffer& buffer, ByteBufferReader* io_reader) {
    if (!io_reader->ReadBytes(reinterpret_cast<char*>(&msg_hdr_), sizeof(msg_hdr_))) {
        DWARNING("ReadRpcHeader buffer unread bytes < rpc_msg_hdr.");
        return false;
    }

    msg_hdr_.length = ntohl(msg_hdr_.length);

    return true;
}

bool RpcChannel::MessageTransform(Buffer& buffer, std::string& content) {
    std::unique_ptr<ByteBufferReader> io_reader(new ByteBufferReader(buffer));
    if (!io_reader) {
        DERROR("MessageTransform new ByteBufferReader failed.");
        return false;
    }

    if (!ReadRpcHeader(buffer, io_reader.get())) {
        return false;
    }

    OnRpcRefresh();

//    DDEBUG("MessageTransform the message version: {}, type: {}, length: {}.",
//        (msg_hdr_.version), (msg_hdr_.type), msg_hdr_.length);

    if (buffer.UnreadByteSize() < msg_hdr_.length + sizeof(msg_hdr_)) {
        DDEBUG("MessageTransform data is not complete.");
        return false;
    }

    io_reader->Consume(sizeof(msg_hdr_));

    if (!io_reader->ReadString(&content, msg_hdr_.length)) {
        DERROR("MessageTransform buffer read failed.");
        return false;
    }

    io_reader->Consume(msg_hdr_.length);

    return true;
}

void RpcChannel::SetMessageId(std::size_t msid) {
    msid_ = msid;
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
    OnRpcError(ec);

    DERROR("OnRpcJsonMessage met error: {}.", ec);
}

void RpcChannel::OnRpcProtobufMessage(std::string content) {
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
    OnRpcError(ec);

    DERROR("OnRpcProtobufMessage met error: {}.", ec);
}

bool RpcChannel::OnRpcRequest(const RpcMessagePtr& rpc_message, enum ErrorCode& ec) {
    auto it = service_map_->find(rpc_message->service());
    if (it != service_map_->end()) {
        google::protobuf::Service* service = it->second;
        if (!service) {
            DERROR("OnRpcRequest the service is nil.");
            ec = NO_SERVICE;
            return false;
        }

        SetMessageId(rpc_message->id());

        const google::protobuf::ServiceDescriptor* sdes = service->GetDescriptor();
        const google::protobuf::MethodDescriptor* method = sdes->FindMethodByName(rpc_message->method());
        if (!method) {
            DERROR("OnRpcRequest {} service not found {} method.",
                   rpc_message->service(), rpc_message->method());
            ec = NO_METHOD;
            return false;
        }

        std::unique_ptr<google::protobuf::Message> request(service->GetRequestPrototype(method).New());
        if (request->ParseFromString(rpc_message->request())) {
            google::protobuf::Message* response = service->GetResponsePrototype(method).New();
            service->CallMethod(method, NULL, request.get(), response,
                                google::protobuf::NewCallback(this, &RpcChannel::Done, response));
            ec = NO_ERROR;
            return true;
        }
    } else {
        DERROR("OnRpcRequest {} service not found.", rpc_message->service());
        ec = NO_SERVICE;
    }

    return false;
}

bool RpcChannel::OnRpcResponse(/*const RpcMessagePtr& rpc_message*/) {
    // TODO:
    return true;
}

void RpcChannel::OnRpcRefresh() {
    if (refresh_cb_) {
        DDEBUG("OnRpcRefresh the channel csid: {}", chan_->csid());
        refresh_cb_(chan_);
    }
}

void RpcChannel::OnRpcError(enum ErrorCode& ec) {
    RpcMessage rpc_message;
    rpc_message.set_type(RESPONSE);
    rpc_message.set_id(msid_);
    rpc_message.set_response(std::string("failed"));
    rpc_message.set_error(ec);

    std::string data;
    if (!rpc_message.SerializeToString(&data)) {
        DERROR("Done the rpc_message serialize to string failed.");
        return;
    }

    chan_->SendMessage(data);
}

void RpcChannel::Done(google::protobuf::Message* pro_message) {
    std::unique_ptr<google::protobuf::Message> message(pro_message);

    RpcMessage rpc_message;
    rpc_message.set_type(RESPONSE);
    rpc_message.set_id(msid_);
    rpc_message.set_response(message->SerializeAsString());

    std::string data;
    if (!rpc_message.SerializeToString(&data)) {
        DERROR("Done the rpc_message serialize to string failed.");
        return;
    }

    chan_->SendMessage(data);
}

} // namespace drpc