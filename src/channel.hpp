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

#ifndef __CHANNEL_HPP__
#define __CHANNEL_HPP__

#include "any_cast.hpp"
#include "async_cbs.hpp"
#include "async_socket.hpp"
#include "byte_buffer.hpp"

namespace drpc {

class EventLoop;

class Channel : public std::enable_shared_from_this<Channel> {
public:
    Channel(EventLoop* event_loop, AsyncSocket* socket);

    ~Channel();

    void Init();

    bool Attach();

    void SendMessage(std::string& message);

    void Close();

    void SetNewChannelCallback(const NewChannelCallback& cb);

    void SetRecvMessageCallback(const RecvMessageCallback& cb);

    void SetSendCompleteCallback(const SendCompleteCallback& cb);

    void SetTimedoutCallback(const TimedoutCallback& cb);

    void SetCloseCallback(const CloseCallback& cb);

private:
    void InternalSendMessage(char* data, size_t len);

    void AsyncSocketReadHandle();

    void AsyncSocketWriteHandle();

private:
    EventLoop* event_loop_;

    std::unique_ptr<AsyncSocket> async_socket_;

    Buffer recv_buffer_;
    Buffer send_buffer_;

    any context_;

    NewChannelCallback new_channel_cb_;

    RecvMessageCallback recv_message_cb_;

    SendCompleteCallback send_complete_cb_;

    TimedoutCallback channel_timeout_cb_;

    CloseCallback channel_closed_cb_;
};

} /* end namespace drpc */

#endif /* __CHANNEL_HPP__ */
