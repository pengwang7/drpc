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
#include "logger.h"
#include "service.pb.h"
#include "rpc_msg_hdr.h"

void connect_and_sendrecv() {
    int fd;
    struct sockaddr_in server;

    fd = socket(AF_INET, SOCK_STREAM, 0);
    bzero(&server, sizeof(server));
    server.sin_family = AF_INET;
    server.sin_port = htons(6689);
    server.sin_addr.s_addr = inet_addr("127.0.0.1");

    ssize_t ret = connect(fd, (struct sockaddr *)&server, sizeof(struct sockaddr));
    if (ret < 0) {
        return;
    }

    drpc::RpcMessage message;
    message.set_type(drpc::REQUEST);
    message.set_id(fd);
    message.set_service("drpc.PublishService");
    message.set_method("Publish");


    drpc::PublishRequest request;
    request.set_message("exten: 1001, status: idle");
    message.set_request(request.SerializeAsString());

    std::string result;
    message.SerializeToString(&result);

    char buf[256] = {0};

    drpc::RpcPacket::Header header;
    memset(&header, 0, sizeof(header));

    drpc::RpcPacket::Body body;

    header.version = 1;
    header.length = htonl(static_cast<uint32_t>(result.size()));
    header.payload = drpc::PAYLOAD_TYPE_PROTOBUF;

    
    //drpc::RpcPacket* packet = new drpc::RpcPacket(header, body);


    memcpy(buf, &header, sizeof(header));
    memcpy(buf + sizeof(header), result.c_str(), result.size());

//    drpc::rpc_msg_hdr msg_hdr;
//    memset(&msg_hdr, 0, sizeof(msg_hdr));
//    msg_hdr.version = 2;
//    msg_hdr.type = 1;
//
//    char c;
//    memcpy(&c, &msg_hdr, 1);
//
//    uint32_t len = htonl(static_cast<uint32_t>(result.size()));
//    memcpy(buf, &c, 1);
//    memcpy(buf + 1, &len, sizeof(uint32_t));
//    memcpy(buf + 1 + 4, result.c_str(), result.size());

    int len = sizeof(header) + result.size();
    ret = send(fd, buf, len, 0);
    if (ret == static_cast<ssize_t>(len)) {
        DDEBUG("Send message success.");
    } else {
        DERROR("Send message failed, {}", std::strerror(errno));
        close(fd);
        return;
    }

    drpc::RpcMessage message1;

    memset(buf, 0, sizeof(buf));
    ret = recv(fd, buf, sizeof(buf), 0);
    if (ret <= 0) {
        DERROR("Recv message failed, {}", std::strerror(errno));
        close(fd);
        return;
    }

    DDEBUG("Recv message success.");

    message1.ParseFromArray(buf, static_cast<uint32_t>(ret));

//    drpc::DDEBUG("The string serialize to proto message: id: %d, service: %s, method: %s, request: %s, errno: %d",
//                message1.id(), message1.service().c_str(), message1.method().c_str(),
//                message1.response().c_str(), message1.error());

    close(fd);

    //delete packet;
}

int main() {
    drpc::Logger::Instance().Init();

    DTRACE("Client test begin.");

    for (int i = 0; i < 1; ++ i) {
        connect_and_sendrecv();
    }

    DTRACE("Client test end.");

    drpc::Logger::Instance().Destroy();

    return 0;
}
