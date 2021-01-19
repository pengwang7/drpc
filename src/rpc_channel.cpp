#include <google/protobuf/descriptor.h>

#include "rpc.pb.h"
#include "logger.hpp"
#include "channel.hpp"
#include "rpc_channel.hpp"

namespace drpc {

RpcChannel::RpcChannel(const channel_ptr& chan, RpcServiceHashTable* services)
    : msid_(0), chan_(chan), default_(&RpcMessage::default_instance()), service_map_(services) {
    DASSERT(chan_, "RpcChannel create failed.");
    DTRACE("Create RpcChannel: %p.", this);
}

RpcChannel::~RpcChannel() {
    DTRACE("The RpcChannel destroy: %p, csid: %s, msid: %d.", this, chan_->csid().c_str(), msid_);
}

void RpcChannel::CallMethod(const google::protobuf::MethodDescriptor* method,
                    google::protobuf::RpcController* controller,
                    const google::protobuf::Message* request,
                    google::protobuf::Message* response,
                    google::protobuf::Closure* done) {

}

void RpcChannel::OnRpcMessage(const channel_ptr& chan, Buffer& buffer) {
    std::string content;
    if (!MessageTransform(buffer, content)) {
        DWARNING("OnRpcMessage message transform failed.");
        return;
    }

    std::shared_ptr<google::protobuf::Message> message(default_->New());
    if (!message) {
        return;
    }

    if (!message->ParseFromString(content)) {
        DERROR("OnRpcMessage protobuf parse from string failed.");
        return;
    }

    std::shared_ptr<RpcMessage> rpc_message = std::dynamic_pointer_cast<RpcMessage>(message);
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
        OnRpcRequest(rpc_message);
        break;

    case RESPONSE:
        //OnRpcResponse();
        break;

    default:
        DERROR("OnRpcMessage the message type is invalid.");
    };
}

bool RpcChannel::MessageTransform(Buffer& buffer, std::string& content) {
    std::unique_ptr<ByteBufferReader> io_reader(new ByteBufferReader(buffer));
    if (!io_reader) {
        DERROR("MessageTransform new ByteBufferReader failed.");
        return false;
    }

    uint32_t hs = 0;
    if (!io_reader->ReadUInt32(&hs)) {
        DWARNING("MessageTransform buffer unread bytes < 4.");
        return false;
    }

    if (buffer.UnreadByteSize() < hs + sizeof(hs)) {
        DDEBUG("MessageTransform data is not complete.");
        return false;
    }

    io_reader->Consume(sizeof(hs));

    if (!io_reader->ReadString(&content, hs)) {
        DERROR("MessageTransform buffer read failed.");
        return false;
    }

    io_reader->Consume(hs);

    return true;
}

void RpcChannel::SetMessageId(std::size_t msid) {

}

void RpcChannel::OnRpcRequest(const RpcMessagePtr& rpc_message) {
    auto it = service_map_->find(rpc_message->service());
    if (it != service_map_->end()) {
        google::protobuf::Service* service = it->second;
        if (!service) {
            DERROR("OnRpcRequest the service is nil.");
            return;
        }

        const google::protobuf::ServiceDescriptor* sdes = service->GetDescriptor();
        const google::protobuf::MethodDescriptor* method = sdes->FindMethodByName(rpc_message->method());
        if (!method) {
            DERROR("OnRpcRequest %s service not found %s method.",
                    rpc_message->service().c_str(), rpc_message->method().c_str());
        }
    } else {
        DERROR("OnRpcRequest %s service not found.", rpc_message->service().c_str());
    }
}

void RpcChannel::OnRpcResponse(const RpcMessagePtr& rpc_message) {

}

void RpcChannel::OnDone(const ProMessagePtr& message) {

}

} /* end namespace drpc */
