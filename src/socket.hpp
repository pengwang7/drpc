/*
 * MIT License
 *
 * Copyright (c) 2020 pengwang7
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#ifndef __SOCKET_HPP__
#define __SOCKET_HPP__

#include <unistd.h>
#include <sys/un.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/tcp.h>

#include "endpoint.hpp"

namespace drpc {

int SetSocketNonblocking(int fd);

bool GetSocketNonblocking(int fd);

int SetSocketOptions(int fd, int option);

bool GetSocketOptions(int fd, int option);

void SetSendRecvBufferSize(int fd, int option, size_t size);

void GetSendRecvBufferSize(int fd, int option, size_t& size);

int CreateSocket(int protocol, bool nonblocking = true);

int ConnectSocket(int fd, std::string& addr, uint16_t port, struct timeval* tv);

bool BindSocket(int fd, std::string local_address);

bool BindSocket(int fd, Endpoint* endpoint);

bool ListenSocket(int fd, int max_conn);

int AcceptSocket(int fd, std::string& peer_addr);

} // namespace drpc

#endif // __SOCKET_HPP__
