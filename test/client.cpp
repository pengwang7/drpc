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
    message.set_service("PublishService");
    message.set_method("Publish");
    message.set_request("exten: 1001, status: idle");

    std::string result;
    message.SerializeToString(&result);

    sleep(3);

    char buf[256] = {0};
    uint32_t len = htonl(static_cast<uint32_t>(result.size()));
    memcpy(buf, &len, sizeof(uint32_t));
    memcpy(buf + 4, result.c_str(), result.size());

    ret = send(fd, buf, result.size() + 4, 0);
    if (ret == static_cast<ssize_t>(result.size() + 4)) {
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
