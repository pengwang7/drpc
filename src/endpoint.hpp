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

#ifndef __ENDPOINT_HPP__
#define __ENDPOINT_HPP__

#include <arpa/inet.h>
#include <netinet/in.h>

#include "constructor_magic.hpp"

namespace drpc {

class Endpoint {
public:
    Endpoint(const char* address, uint16_t port) : port_(port), is_valid_(false) {
        ParseAddressPort(address);
        assert(is_valid_);
    }

    Endpoint(const char* address_with_port) : port_(0), is_valid_(false) {
        std::string temp = std::string(address_with_port);
        std::string host;
        std::size_t index = 0;

        if (temp.empty()) {
            goto failed;
        }

        index = temp.rfind(':');
        if (index == std::string::npos || index == (temp.size() - 1)) {
            goto failed;
        }

        port_ = std::atoi(&temp[index + 1]);

        host = std::string(address_with_port, index);
        if (host[0] == '[') {
            if (*host.rbegin() != ']') {
                goto failed;
            }

            host = std::string(host.data() + 1, host.size() - 2);
        }

        if (*host.rbegin() == ']') {
            host = std::string(host.data(), host.size() - 1);
        }

        ParseAddressPort(host.c_str());

failed:
        assert(is_valid_);
    }

    std::string ToString() const {
        if (family_ != AF_INET && family_ != AF_INET6) {
            return std::string("");
        }

        char buf[INET6_ADDRSTRLEN] = {0};
        const void* src = &addr_.ipv4;

        if (family_ == AF_INET6) {
            src = &addr_.ipv6;
        }

        if (!inet_ntop(family_, src, buf, sizeof(buf))) {
            return std::string("");
        }

        return std::string(buf) + std::string(":") + std::to_string(port_);
    }

    in_addr ipv4_address() const {
        return addr_.ipv4;
    }

    in6_addr ipv6_address() const {
        return addr_.ipv6;
    }

    uint16_t port() const {
        return port_;
    }

    uint16_t family() const {
        return family_;
    }

private:
    void ParseAddressPort(const char* address) {
        if (inet_pton(AF_INET, address, &addr_.ipv4) != 0) {
            family_ = AF_INET;
            is_valid_ = true;
        }

        if (inet_pton(AF_INET6, address, &addr_.ipv6) != 0) {
            family_ = AF_INET6;
            is_valid_ = true;
        }
    }

private:
    DISALLOW_COPY_AND_ASSIGN(Endpoint);

    uint16_t family_;

    union {
        in_addr ipv4;
        in6_addr ipv6;
    } addr_;

    uint16_t port_;

    bool is_valid_;
};

} // namespace drpc

#endif // __ENDPOINT_HPP__
