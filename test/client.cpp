#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <strings.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <netdb.h>
#include <string>
#include <iostream>

#include "rpc.pb.h"
#include "logger.hpp"
#include "service.pb.h"
#include "rpc_msg_hdr.hpp"

int main() {
    drpc::Logger::Instance().Init();

    drpc::DTRACE("Client test begin.");

    int fd;
    struct sockaddr_in server;

    fd = socket(AF_INET, SOCK_STREAM, 0);
    bzero(&server, sizeof(server));
    server.sin_family = AF_INET;
    server.sin_port = htons(6689);
    server.sin_addr.s_addr = inet_addr("127.0.0.1");

    ssize_t ret = connect(fd, (struct sockaddr *)&server, sizeof(struct sockaddr));
    if (ret < 0) {
        return -1;
    }

    drpc::RpcMessage message;
    message.set_type(drpc::REQUEST);
    message.set_id(101);
    message.set_service("drpc.PublishService");
    message.set_method("Publish");


    drpc::PublishRequest request;
    request.set_message("exten: 1001, status: idle");
    message.set_request(request.SerializeAsString());

    std::string result;
    message.SerializeToString(&result);

    sleep(3);

    char buf[256] = {0};

    drpc::rpc_msg_hdr msg_hdr;
    memset(&msg_hdr, 0, sizeof(msg_hdr));
    msg_hdr.version = 2;
    msg_hdr.type = 1;

    char c;
    memcpy(&c, &msg_hdr, 1);

    uint32_t len = htonl(static_cast<uint32_t>(result.size()));
    memcpy(buf, &c, 1);
    memcpy(buf + 1, &len, sizeof(uint32_t));
    memcpy(buf + 1 + 4, result.c_str(), result.size());

    ret = send(fd, buf, result.size() + 1 + 4, 0);
    if (ret == static_cast<ssize_t>(result.size() + 4 + 1)) {
        drpc::DDEBUG("Send message success.");
    } else {
        drpc::DERROR("Send message failed, %s", strerror(errno));
        return -1;
    }

    drpc::RpcMessage message1;

    memset(buf, 0, sizeof(buf));
    ret = recv(fd, buf, sizeof(buf), 0);
    if (ret <= 0) {
        drpc::DERROR("Recv message failed, %s", strerror(errno));
        return -1;
    }

    drpc::DDEBUG("Recv message success.");

    message1.ParseFromArray(buf, static_cast<uint32_t>(ret));

    drpc::DDEBUG("The string serialize to proto message: id: %d, service: %s, method: %s, request: %s, errno: %d",
                message1.id(), message1.service().c_str(), message1.method().c_str(),
                message1.response().c_str(), message1.error());    

    drpc::DTRACE("Client test end.");

    return 0;
}
