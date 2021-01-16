#include <google/protobuf/descriptor.h>

#include "rpc.pb.h"
#include "logger.hpp"
#include "channel.hpp"
#include "rpc_channel.hpp"

namespace drpc {

RpcChannel::RpcChannel(const channel_ptr& chan) : chan_(chan) {
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

}

void RpcChannel::SetMessageId(std::size_t msid) {

}

void RpcChannel::OnRpcRequest(const channel_ptr& chan, const ProtoMessage& message) {

}

void RpcChannel::OnRpcResponse(const channel_ptr& chan, const ProtoMessage& message) {

}

void RpcChannel::OnDone(const channel_ptr& chan, const ProtoMessage& message) {

}

} /* end namespace drpc */
