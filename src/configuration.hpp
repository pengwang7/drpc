#ifndef __CONFIGURATION_HPP__
#define __CONFIGURATION_HPP__

namespace drpc {

class Configuration {
public:
    Configuration(unsigned short port = 8086) noexcept : port(port) {}

public:
    // Port number to use. Defaults to 8086. Set to 0 get an assigned port.
    unsigned short port;

    // IPv4 address in dotted decimal form or IPv6 address in hexadecimal notation.
    // If empty, the address will be any address.
    std::string address;

    // Defaults to 8 threads.
    std::size_t threads = 8;

    // Default used server mode.
    unsigned short server_mode = 0

    // Socket heartbeat timeout. Defaults to 300 seconds.
    int timeout_content = 300;

    // Process open max file descriptor limit.
    size_t fd_limits = 409600;

    // Maximum size of channel stream buffer. Defaults to architecture maximum.
    // Reaching this limit will result in a message_size error code.
    std::size_t max_streambuf_size = std::numeric_limits<std::size_t>::max();
};

} /* end namespace drpc */

#endif /* __CONFIGURATION_HPP__ */
