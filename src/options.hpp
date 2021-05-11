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

#ifndef __OPTIONS_HPP__
#define __OPTIONS_HPP__

#include <limits>
#include <string>

namespace drpc {

// Server options.
class Options {
public:
    // Port number to use. Defaults to 8086. Set to 0 get an assigned port.
    uint16_t port = 8086;

    // IPv4 address in dotted decimal form or IPv6 address in hexadecimal notation.
    // If empty, the address will be any address.
    std::string address = "0.0.0.0";

    // Defaults to 'nprocs' threads.
    std::size_t threads = 4;

    // SSL not support.
    bool enable_ssl = false;

    // Channel heartbeat. Defaults to enabled.
    bool enable_check_timeout = true;

    // Channel heartbeat timeout seconds. Defaults to 300.
    uint32_t timeout = 300;

    // The server stop stop_time seconds after start.
    // For ASAN test, default 0 means not enabled.
    uint32_t stop_time = 0;

    // Process open max file descriptor limit.
    std::size_t fd_limits = 819200;

    // Maximum size of channel stream buffer. Defaults to architecture maximum.
    // Reaching this limit will result in a message_size error code.
    std::size_t max_streambuf_size = std::numeric_limits<std::size_t>::max();
};

} // namespace drpc

#endif // __OPTIONS_HPP__
