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

#include "logger.hpp"
#include "socket.hpp"

namespace drpc {

bool SetSocketNonblocking(int fd) {
    int flags = 0;

    if ((flags = fcntl(fd, F_GETFL, NULL)) < 0) {
        DERROR("SetSocketNonblocking failed: %s", strerror(errno));
    	return false;
    }

    if (!(flags & O_NONBLOCK)) {
    	if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) == -1) {
            DERROR("SetSocketNonblocking failed: %s", strerror(errno));
    		return false;
    	}
    }

    return true;
}

bool GetSocketNonblocking(int fd) {
    int flags = 0;

    flags = fcntl(fd, F_GETFL, NULL);

    return !(flags & O_NONBLOCK);
}

void SetSocketOptions(int fd, int option) {
    int on = 1;

    switch (option) {
    case O_NONBLOCK:
        SetSocketNonblocking(fd);
        break;

    case SO_REUSEADDR:
        setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, (void*)&on, static_cast<socklen_t>(sizeof(on)));
        break;

    case SO_REUSEPORT:
        // REUSEPORT on Linux 3.9+ means, "Multiple servers (processes or
        // threads) can bind to the same port if they each set the option. 
        setsockopt(fd, SOL_SOCKET, SO_REUSEPORT, (void*)&on, static_cast<socklen_t>(sizeof(on)));
        break;

    case TCP_DEFER_ACCEPT:
        // TCP_DEFER_ACCEPT tells the kernel to call defer accept() only
        // after data has arrived and ready to read. 
        setsockopt(fd, IPPROTO_TCP, TCP_DEFER_ACCEPT, &on, static_cast<socklen_t>(sizeof(on))); 
        break;

    case TCP_NODELAY:
        setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, &on, static_cast<socklen_t>(sizeof(on)));
        break;

    case TCP_QUICKACK:
        setsockopt(fd, IPPROTO_TCP, TCP_QUICKACK, &on, static_cast<socklen_t>(sizeof(on)));
        break;

//    TCP_DEFER_ACCEPT == SO_KEEPALIVE
//    case SO_KEEPALIVE:
//        setsockopt(fd_, SOL_SOCKET, SO_KEEPALIVE, &on, static_cast<socklen_t>(sizeof(on)));
//        break;
    }
}

bool GetSocketOptions(int fd, int option) {
    int opt_val = 0;
    socklen_t opt_len = sizeof(opt_val);

    switch (option) {
    case O_NONBLOCK:
        opt_val = GetSocketNonblocking(fd);
        break;
    case SO_REUSEADDR:
        getsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &opt_val, &opt_len);
        break;
    case SO_REUSEPORT:
        getsockopt(fd, SOL_SOCKET, SO_REUSEPORT, &opt_val, &opt_len);
        break;
    case TCP_DEFER_ACCEPT:
        getsockopt(fd, IPPROTO_TCP, TCP_DEFER_ACCEPT, &opt_val, &opt_len);
        break;
    case TCP_NODELAY:
        getsockopt(fd, IPPROTO_TCP, TCP_NODELAY, &opt_val, &opt_len);
        break;
    case TCP_QUICKACK:
        getsockopt(fd, IPPROTO_TCP, TCP_QUICKACK, &opt_val, &opt_len);
        break;
    default:
        return false;
    }

    return (opt_val == 1);
}

void SetSendRecvBufferSize(int fd, int option, size_t size) {
    setsockopt(fd, SOL_SOCKET, option, (void*)&size, static_cast<socklen_t>(sizeof(size)));
}

void GetSendRecvBufferSize(int fd, int option, size_t& size) {
    socklen_t optlen = sizeof(size);
    getsockopt(fd, SOL_SOCKET, option, (void*)&size, &optlen);
}

int CreateSocket(int protocol, bool nonblocking) {
    int fd = -1;

    switch (protocol) {
    case AF_INET:
    case AF_UNIX:
        fd = socket(protocol, SOCK_STREAM, 0);
        break;
    default:
        goto error;
    };

    if (nonblocking) {
        if (!SetSocketNonblocking(fd)) {
            goto error;
        }
    }

    return fd;

error:
    close(fd);

    return -1;
}

int ConnectSocket(int fd, std::string& addr, uint16_t port, struct timeval* tv) {
    int ret = 0;
    int res = 0;

    struct sockaddr_in srvaddr;
    memset(&srvaddr, 0, sizeof(srvaddr));
    srvaddr.sin_family = AF_INET;
    srvaddr.sin_port = htons(port);
    srvaddr.sin_addr.s_addr = inet_addr(addr.c_str());

    do {
        ret = connect(fd, reinterpret_cast<struct sockaddr*>(&srvaddr), sizeof(srvaddr));
    } while (ret < 0 && errno == EINTR);

    if (ret == 0) {
        return 0;
    }

    if (ret < 0 && errno == EINPROGRESS) {
        fd_set conn_fdset, error_fdset;
        FD_ZERO(&conn_fdset);
        FD_ZERO(&error_fdset);
        FD_SET(fd, &conn_fdset);
        FD_SET(fd, &error_fdset);

        res = select(fd + 1, NULL, &conn_fdset, &error_fdset, tv);
        if (res < 0) {
            DERROR("ConnectSocket failed: %s", strerror(errno));
            return -1;
        } else if (res == 0) {
            errno = ETIMEDOUT;
            DERROR("ConnectSocket failed: %s", strerror(errno));
            return 0;
        }

        int so_error;
        socklen_t so_error_len = sizeof(so_error);

        if (FD_ISSET(fd, &error_fdset)) {
            getsockopt(fd, SOL_SOCKET, SO_ERROR, &so_error, &so_error_len);
            errno = so_error;
            DERROR("ConnectSocket failed: %s", strerror(errno));
            return -1;
        }
    } else {
        DERROR("ConnectSocket failed: %s", strerror(errno));
        return -1;
    }

    return 0;
}

bool BindSocket(int fd, std::string local_address) {
    unlink(local_address.c_str());

    struct sockaddr_un baddr;
    memset(&baddr, 0, sizeof(baddr));;

    baddr.sun_family = AF_UNIX;
    strcpy(baddr.sun_path, local_address.c_str());

    if (bind(fd, (struct sockaddr*)&baddr, sizeof(baddr)) < 0) {
        DERROR("BindSocket failed: %s", strerror(errno));
        return false;
    }

    return true;
}

bool BindSocket(int fd, IPAddress ip, uint16_t port) {
    struct sockaddr_in baddr;
    memset(&baddr, 0, sizeof(baddr));

    baddr.sin_family = ip.family();
    baddr.sin_port = htons(port);
    baddr.sin_addr = ip.ipv4_address();

    if (bind(fd, (struct sockaddr*)&baddr, sizeof(baddr)) < 0) {
        DERROR("BindSocket failed: %s", strerror(errno));
        return false;
    }

    return true;
}

bool ListenSocket(int fd, int so_max_conn) {
    if (so_max_conn <= 0) {
        so_max_conn = 256;
    }

    // The behavior of the backlog argument on TCP sockets changed with Linux 2.2.
    // Now it specifies the queue length for completely established sockets waiting
    // to be accepted, instead of the number of incomplete connection requests.
    // The maximum length of the queue for incomplete sockets can be set
    // using /proc/sys/net/ipv4/tcp_max_syn_backlog.
    // When syncookies are enabled there is no logical maximum length and this setting
    // is ignored.
    if (listen(fd, so_max_conn) < 0) {
        DERROR("ListenSocket failed: %s", strerror(errno));
        return false;
    }

    return true;
}

int AcceptSocket(int fd, std::string& peer_addr) {
    char buf[INET_ADDRSTRLEN];
    struct sockaddr_in raddr;
    socklen_t addr_len = sizeof(raddr);

    int conn_fd = accept(fd, reinterpret_cast<struct sockaddr*>(&raddr), &addr_len);
    if (conn_fd < 0) {
        DERROR("AcceptSocket failed: %s", strerror(errno));
        return -1;
    }

    inet_ntop(raddr.sin_family, &raddr.sin_addr, buf, sizeof(buf));

    peer_addr = std::string(buf);

    return conn_fd;
}

} /* end namespace drpc */
