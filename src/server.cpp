#include "logger.hpp"
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

    return true;
}

bool Server::Stop() {
    return true;
}

bool Server::AddService(::google::protobuf::Service* service) {
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

    group_.reset(new EventLoopGroup(options_->threads, "event-loop-group"));
    if (!group_) {
        DERROR("Server init failed, group is nil.");
        return false;
    }

    return true;
}

void Server::OnConnection() {

}

void Server::OnMessage() {

}

} /* end namespace drpc */
