#include <google/protobuf/descriptor.h>

#include "rpc.pb.h"
#include "logger.hpp"
#include "channel.hpp"
#include "rpc_channel.hpp"

namespace drpc {

RpcChannel::RpcChannel(const channel_ptr& chan, RpcServiceHashTable* services)
    : msid_(0), chan_(chan), default_(&RpcMessage::default_instance()), service_map_(services) {
    memset(&msg_hdr_, 0, sizeof(msg_hdr_));
    DASSERT(chan_, "RpcChannel create failed.");
    DTRACE("Create RpcChannel: %p.", this);
}

RpcChannel::~RpcChannel() {
    DTRACE("The RpcChannel destroy: %p, csid: %s, msid: %d.", this, chan_->csid().c_str(), msid_);

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

void RpcChannel::OnRpcMessage(const channel_ptr& chan, Buffer& buffer) {
    bool ret = false;
    ErrorCode ec = INVALID_REQUEST;

    std::string content;
    std::shared_ptr<RpcMessage> rpc_message;

    std::shared_ptr<google::protobuf::Message> message(default_->New());
    if (!message) {
        return;
    }

    if (!MessageTransform(buffer, content)) {
        DWARNING("OnRpcMessage message transform failed.");
        goto error;
    }

    if (!message->ParseFromString(content)) {
        DERROR("OnRpcMessage protobuf parse from string failed.");
        goto error;
    }

    rpc_message = std::dynamic_pointer_cast<RpcMessage>(message);
    if (!rpc_message) {
        DERROR("OnRpcMessage std::dynamic_pointer_cast failed.");
        return;
    }

    DDEBUG("The string serialize to proto message: id: %d, "
            "service: %s, method: %s, request: %s",
            rpc_message->id(), rpc_message->service().c_str(),
            rpc_message->method().c_str(), rpc_message->request().c_str());

    switch (rpc_message->type()) {
    case REQUEST:
        ret = OnRpcRequest(rpc_message, ec);
        break;

    case RESPONSE:
//        ret = OnRpcResponse();
        break;

    default:
        DERROR("OnRpcMessage the message type is invalid.");
    };

    if (ret && ec == NO_ERROR) {
        return;
    }

error:
    OnRpcError(ec);

    DWARNING("OnRpcMessage met error: %d.", ec);
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

    DDEBUG("MessageTransform the message version: %d, type: %d, length:%d.",
            msg_hdr_.version, msg_hdr_.type, msg_hdr_.length);

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
            DERROR("OnRpcRequest %s service not found %s method.",
                    rpc_message->service().c_str(), rpc_message->method().c_str());
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
        DERROR("OnRpcRequest %s service not found.", rpc_message->service().c_str());
        ec = NO_SERVICE;
    }

    return false;
}

bool RpcChannel::OnRpcResponse(const RpcMessagePtr& rpc_message) {
    return false;
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

} /* end namespace drpc */
