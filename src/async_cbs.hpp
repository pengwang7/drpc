#ifndef __ASYNC_CBS_HPP__
#define __ASYNC_CBS_HPP__

#include "buffer.hpp"

namespace drpc {

class Channel;

using channel_ptr = std::shared_ptr<Channel>;

using NewChannelCallback = std::function<void(const channel_ptr& chan)>;

using RecvMessageCallback = std::function<void(const channel_ptr& chan, Buffer& buf)>;

using SendCompleteCallback = std::function<void(const channel_ptr& chan)>;

using TimedoutCallback = std::function<void(const channel_ptr&)>;

using CloseCallback = std::function<void(const channel_ptr&)>;

} /* end namespace drpc */

#endif /* __ASYNC_CBS_HPP__ */
