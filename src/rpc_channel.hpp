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

class RpcMessage;

using ProMessagePtr = std::shared_ptr<google::protobuf::Message>;
using RpcMessagePtr = std::shared_ptr<RpcMessage>;
using RpcServiceHashTable = std::unordered_map<std::string, google::protobuf::Service*>;

class RpcChannel : public ::google::protobuf::RpcChannel {
public:
    RpcChannel(const channel_ptr& chan, RpcServiceHashTable* services);

    virtual ~RpcChannel();

    void CallMethod(const google::protobuf::MethodDescriptor* method,
                        google::protobuf::RpcController* controller,
                        const google::protobuf::Message* request,
                        google::protobuf::Message* response,
                        google::protobuf::Closure* done);

    void OnRpcMessage(const channel_ptr& chan, Buffer& buffer);

private:
    bool MessageTransform(Buffer& buffer, std::string& content);

    void SetMessageId(std::size_t msid);

    bool OnRpcRequest(const RpcMessagePtr& rpc_message, std::string& reason);

    bool OnRpcResponse(const RpcMessagePtr& rpc_message);

    void OnRpcError(std::string& reason);

    void Done(const channel_ptr chan, google::protobuf::Message* pro_message);

private:
    typedef struct {
        google::protobuf::Message* response;
        google::protobuf::Closure* done;        
    } outstanding_call;

    std::size_t msid_;

    channel_ptr chan_;

    const google::protobuf::Message* default_;

    std::map<std::size_t, outstanding_call> outstandings_;

    std::unordered_map<std::string, google::protobuf::Service*>* service_map_;
};

} /* end namespace drpc */

#endif /* __RPC_CHANNEL_HPP__ */
