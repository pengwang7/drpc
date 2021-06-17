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

#ifndef __UTILITY_HPP__
#define __UTILITY_HPP__

#include <cstddef>
#include <cassert>
#include <unistd.h>
#include <sys/eventfd.h>

#include <memory>
#include <functional>

namespace drpc {

#define RETRIABLE_ERROR (-1000)

#if EAGAIN == EWOULDBLOCK
#define EVUTIL_ERR_IS_EAGAIN(e)                 \
                ((e) == EAGAIN)
#else
#define EVUTIL_ERR_IS_EAGAIN(e)                 \
                ((e) == EAGAIN || (e) == EWOULDBLOCK)
#endif

// True iff e is an error that means a read/write operation can be retried.
#define EVUTIL_ERR_RW_RETRIABLE(e)				\
                ((e) == EINTR || EVUTIL_ERR_IS_EAGAIN(e))

// True iff e is an error that means an connect can be retried.
#define EVUTIL_ERR_CONNECT_RETRIABLE(e)			\
                ((e) == EINTR || (e) == EINPROGRESS)

// True iff e is an error that means a accept can be retried.
#define EVUTIL_ERR_ACCEPT_RETRIABLE(e)			\
                ((e) == EINTR || EVUTIL_ERR_IS_EAGAIN(e) || (e) == ECONNABORTED)

// True iff e is an error that means the connection was refused.
#define EVUTIL_ERR_CONNECT_REFUSED(e)			\
                ((e) == ECONNREFUSED)

#define OBJECT_SAFE_DESTROY(_obj, _destroy_fn) \
                do {                                \
                    if ((_obj)) {                   \
                        _destroy_fn((_obj));        \
                        (_obj) = nullptr;           \
                    }                               \
                } while (0)

#define htonll(x)                                            \
                ((((x) & 0xff00000000000000ull) >> 56)       \
                | (((x) & 0x00ff000000000000ull) >> 40)      \
                | (((x) & 0x0000ff0000000000ull) >> 24)      \
                | (((x) & 0x000000ff00000000ull) >> 8)       \
                | (((x) & 0x00000000ff000000ull) << 8)       \
                | (((x) & 0x0000000000ff0000ull) << 24)      \
                | (((x) & 0x000000000000ff00ull) << 40)      \
                | (((x) & 0x00000000000000ffull) << 56))

#define ntohll(x) htonll(x)

template <typename T, typename F>
inline T implicit_cast(F const& val) {
    return val;
}

} // namespace drpc

#endif // __UTILITY_HPP__
