#ifndef __UTILITY_HPP__
#define __UTILITY_HPP__

#include <assert.h>
#include <unistd.h>
#include <sys/eventfd.h>

namespace drpc {

#define OBJECT_SAFE_DESTROY(_obj, _destroy_fn) do { \
            if ((_obj)) { \
                _destroy_fn((_obj)); \
                (_obj) = NULL; \
            } \
} while (0)

} /* end namespace drpc */

#endif /* __UTILITY_HPP__ */
