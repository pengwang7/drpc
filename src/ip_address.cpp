#include "ip_address.hpp"

namespace drpc {

// Prefixes used for categorizing IPv6 addresses.
static const in6_addr kV4MappedPrefix = {
    {{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0xFF, 0xFF, 0}}};

bool IPAddress::operator==(const IPAddress& other) const {

}

bool IPAddress::operator!=(const IPAddress& other) const {

}

bool IPAddress::operator<(const IPAddress& other) const {

}

bool IPAddress::operator>(const IPAddress& other) const {

}
    
in_addr IPAddress::ipv4_address() const {
    return u_.ip4;
}

in6_addr IPAddress::ipv6_address() const {
    return u_.ip6;
}

size_t IPAddress::size() const {
    switch (family_) {
    case AF_INET:
        return sizeof(in_addr);
    case AF_INET6:
        return sizeof(in6_addr);
    }

    return 0;
}

std::string IPAddress::ToString() const {
    if (family_ != AF_INET && family_ != AF_INET6) {
        return std::string("");
    }

    char buf[INET6_ADDRSTRLEN] = {0};
    const void* src = &u_.ip4;

    if (family_ == AF_INET6) {
        src = &u_.ip6;
    }

    if (!inet_ntop(family_, src, buf, sizeof(buf))) {
        return std::string("");
    }

    return std::string(buf);
}

bool IPAddress::IPFromString(const std::string& str, IPAddress* out) {
    if (!out) {
        return false;
    }

    in_addr addr;
    if (inet_pton(AF_INET, str.c_str(), &addr) != 0) {
        *out = IPAddress(addr);
        return true;
    }

    in6_addr addr6;

    if (inet_pton(AF_INET6, str.c_str(), &addr6) != 0) {
        *out = IPAddress(addr6);
        return true;
    }

    *out = IPAddress();    

    return false;
}

IPAddress IPAddress::Normalized() const {

}

IPAddress IPAddress::AsIPv6Address() const {
    if (family_ != AF_INET) {
        return *this;
    }

    in6_addr v6addr = kV4MappedPrefix;
    memcpy(&v6addr.s6_addr[12], &u_.ip4.s_addr, sizeof(u_.ip4.s_addr));

    return IPAddress(v6addr);
}

bool IPAddress::IsNil() const {

}

} /* end namespace drpc */
