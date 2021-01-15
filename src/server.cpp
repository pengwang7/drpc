#include <google/protobuf/service.h>
#include <google/protobuf/descriptor.h>

#include <sys/sysinfo.h>

#include "logger.hpp"
#include "event_loop.hpp"
#include "listener.hpp"
#include "server.hpp"

namespace drpc {

Server::Server() {
    DTRACE("Create Server: %p.", this);
}

Server::~Server() {
    DTRACE("Destroy Server: %p.", this);
}

bool Server::Start(ServerOptions* options) {\
    if (!DoInit(options)) {
        DERROR("Server start error.");
        return false;
    }

    group_->Run();

    if (!listener_->Start()) {
        DERROR("Server start error.");
        return false;
    }

    listener_event_loop_->Run();

    return true;
}

bool Server::Stop() {
    listener_->Stop();

    group_->Stop();

    return true;
}

bool Server::AddService(::google::protobuf::Service* service) {
    const google::protobuf::ServiceDescriptor* desc = service->GetDescriptor();
    if (service_map_.find(desc->full_name()) != service_map_.end()) {
        DWARNING("AddService failed, already had same name.");
        return false;
    }

    service_map_.insert({desc->full_name(), service});

    DDEBUG("The service add to rpc success(%s).", desc->full_name().c_str());

    return true;
}

ServerOptions* Server::GetServerOptions() {
    return options_;
}

bool Server::DoInit(ServerOptions* options) {
    if (!options) {
        DERROR("Server init failed, options is nil.");
        return false;
    }

    options_ = options;

    if (options_->threads <= 0) {
        options_->threads = get_nprocs();
    }

    group_.reset(new EventLoopGroup(options_->threads, "event-loop-group"));
    if (!group_) {
        DERROR("Server init failed, group is nil.");
        return false;
    }

    listener_event_loop_.reset(new EventLoop());
    if (!listener_event_loop_) {
        DERROR("Server init failed, listener event loop is nil.");
        return false;
    }

    listener_.reset(new Listener(listener_event_loop_.get(), options_));
    if (!listener_) {
        DERROR("Server init failed, listener is nil.");
        return false;
    }

    listener_->SetNewConnCallback(std::bind(&Server::BuildChannel,
                this, std::placeholders::_1, std::placeholders::_2));

    return true;
}

void Server::BuildChannel(int fd, std::string& peer_addr) {
    DTRACE("BuildChannel fd: %d, peer address: %s.", fd, peer_addr.c_str());
}

void Server::OnMessage() {

}

} /* end namespace drpc */
