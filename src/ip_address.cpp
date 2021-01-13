#include "ip_address.hpp"

namespace drpc {

// Prefixes used for categorizing IPv6 addresses.
static const in6_addr kV4MappedPrefix = {
    {{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0xFF, 0xFF, 0}}
};

bool IPAddress::operator==(const IPAddress& other) const {
    if (family_ != other.family_) {
        return false;
    }

    if (family_ == AF_INET) {
        return memcmp(&u_.ip4, &other.u_.ip4, sizeof(u_.ip4)) == 0;
    }

    if (family_ == AF_INET6) {
        return memcmp(&u_.ip6, &other.u_.ip6, sizeof(u_.ip6)) == 0;
    }

    return family_ == AF_UNSPEC;
}

bool IPAddress::operator!=(const IPAddress& other) const {
    return !((*this) == other);
}

bool IPAddress::operator<(const IPAddress& other) const {
    // IPv4 is 'less than' IPv6
    if (family_ != other.family_) {
        if (family_ == AF_UNSPEC) {
            return true;
        }

        if (family_ == AF_INET && other.family_ == AF_INET6) {
            return true;
        }

        return false;
    }

    // Comparing addresses of the same family.
    switch (family_) {
        case AF_INET: {
            return ntohl(u_.ip4.s_addr) < ntohl(other.u_.ip4.s_addr);
        }

        case AF_INET6: {
            return memcmp(&u_.ip6.s6_addr, &other.u_.ip6.s6_addr, 16) < 0;
        }
    }

    // Catches AF_UNSPEC and invalid addresses.
    return false;
}

bool IPAddress::operator>(const IPAddress& other) const {
    return (*this) != other && !((*this) < other);
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

IPAddress IPAddress::AsIPv6Address() const {
    if (family_ != AF_INET) {
        return *this;
    }

    in6_addr v6addr = kV4MappedPrefix;
    memcpy(&v6addr.s6_addr[12], &u_.ip4.s_addr, sizeof(u_.ip4.s_addr));

    return IPAddress(v6addr);
}

bool IPAddress::IPIsUnspec(const IPAddress& ip) const {
    return ip.family() == AF_UNSPEC;
}

bool IPFromString(const std::string& str, IPAddress* out) {
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

} /* end namespace drpc */
