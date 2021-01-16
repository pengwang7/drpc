#ifndef __1RPC_CHANNEL_HPP__
#define __1RPC_CHANNEL_HPP__

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

} /* end namespace protobuf */

} /* end namespace google */

namespace drpc {

using ProtoMessage = std::shared_ptr<google::protobuf::Message>;

class RpcChannel : public ::google::protobuf::RpcChannel {
public:
    RpcChannel(const channel_ptr& chan);

    virtual ~RpcChannel();

    void CallMethod(const google::protobuf::MethodDescriptor* method,
                        google::protobuf::RpcController* controller,
                        const google::protobuf::Message* request,
                        google::protobuf::Message* response,
                        google::protobuf::Closure* done);

    void OnRpcMessage(const channel_ptr& chan, Buffer& buffer);

private:
    void SetMessageId(std::size_t msid);

    void OnRpcRequest(const channel_ptr& chan, const ProtoMessage& message);

    void OnRpcResponse(const channel_ptr& chan, const ProtoMessage& message);
    
    void OnDone(const channel_ptr& chan, const ProtoMessage& message);

private:
    typedef struct {
        google::protobuf::Message* response;
        google::protobuf::Closure* done;        
    } outstanding_call;

    std::size_t msid_;

    channel_ptr chan_;

    const ::google::protobuf::Message* prototype_;

    std::map<std::size_t, outstanding_call> outstandings_;

    std::unordered_map<std::string, google::protobuf::Service*> service_map_;
};

} /* end namespace drpc */

#endif /* __RPC_CHANNEL_HPP__ */
