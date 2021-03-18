/*
 *  Copyright 2011 The WebRTC Project Authors. All rights reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#ifndef __IP_ADDRESS_HPP__
#define __IP_ADDRESS_HPP__

#include <arpa/inet.h>
#include <netinet/in.h>

#include <string.h>

#include <string>

namespace drpc {

enum IPv6AddressFlag {
    IPV6_ADDRESS_FLAG_NONE = 0x00,

    // Temporary address is dynamic by nature and will not carry MAC
    // address.
    IPV6_ADDRESS_FLAG_TEMPORARY = 1 << 0,

    // Temporary address could become deprecated once the preferred
    // lifetime is reached. It is still valid but just shouldn't be used
    // to create new connection.
    IPV6_ADDRESS_FLAG_DEPRECATED = 1 << 1,
};

// Version-agnostic IP address class, wraps a union of in_addr and in6_addr.
class IPAddress {
public:
    IPAddress() : family_(AF_UNSPEC) {
        memset(&u_, 0, sizeof(u_));
    }

    explicit IPAddress(const in_addr& ip4) : family_(AF_INET) {
        memset(&u_, 0, sizeof(u_));
        u_.ip4 = ip4;
    }

    explicit IPAddress(const in6_addr& ip6) : family_(AF_INET6) {
        u_.ip6 = ip6;
    }

    explicit IPAddress(uint32_t ip_in_host_byte_order) : family_(AF_INET) {
        memset(&u_, 0, sizeof(u_));
        u_.ip4.s_addr = htonl(ip_in_host_byte_order);
    }

    IPAddress(const IPAddress& other) : family_(other.family_) {
        memcpy(&u_, &other.u_, sizeof(u_));
    }

    virtual ~IPAddress() {}

    const IPAddress& operator=(const IPAddress& other) {
        family_ = other.family_;
        memcpy(&u_, &other.u_, sizeof(u_));
        return *this;
    }

    bool operator==(const IPAddress& other) const;
    bool operator!=(const IPAddress& other) const;
    bool operator<(const IPAddress& other) const;
    bool operator>(const IPAddress& other) const;

    uint16_t family() const {
        return family_;
    }
    in_addr ipv4_address() const;
    in6_addr ipv6_address() const;

    // Returns the number of bytes needed to store the raw address.
    size_t size() const;

    // Wraps inet_ntop.
    std::string ToString() const;

    // Returns this address as an IPv6 address.
    // Maps v4 addresses (as ::ffff:a.b.c.d), returns v6 addresses unchanged.
    IPAddress AsIPv6Address() const;

    // For socketaddress' benefit. Returns the IP in host byte order.
    uint32_t V4AddressAsHostOrderInteger() const;

    // Get the network layer overhead per packet based on the IP address family.
    int overhead() const;

    // Whether this is an unspecified IP address.
    bool IPIsUnspec(const IPAddress& ip) const;

private:
    uint16_t family_;

    union {
        in_addr ip4;
        in6_addr ip6;
    } u_;
};

bool IPFromString(const std::string& str, IPAddress* out);

} // namespace drpc

#endif // __IP_ADDRESS_H__
