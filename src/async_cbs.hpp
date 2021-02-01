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

#ifndef __ASYNC_CBS_HPP__
#define __ASYNC_CBS_HPP__

#include "buffer.hpp"

namespace drpc {

class Channel;

class Scheduled;

using channel_ptr = std::shared_ptr<Channel>;

using weak_channel_ptr = std::weak_ptr<Channel>;

using scheduled_ptr = std::shared_ptr<Scheduled>;

using NewChannelCallback = std::function<void(const channel_ptr& chan)>;

using RecvMessageCallback = std::function<void(const channel_ptr& chan, Buffer& buf)>;

using SendCompleteCallback = std::function<void(const channel_ptr& chan)>;

using TimedoutCallback = std::function<void(const channel_ptr&)>;

using CloseCallback = std::function<void(const channel_ptr&)>;

} /* end namespace drpc */

#endif /* __ASYNC_CBS_HPP__ */
