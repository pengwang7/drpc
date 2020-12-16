#ifndef __UTILITY_HPP__
#define __UTILITY_HPP__

#include <cstddef>
#include <cassert>
#include <unistd.h>
#include <sys/eventfd.h>

namespace drpc {

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
                        (_obj) = NULL;              \
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

} /* end namespace drpc */

#endif /* __UTILITY_HPP__ */
