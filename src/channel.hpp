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
    static const uint32_t kContextMax = 16;

    enum Status {
        kDisconnected = 0,
        kConnecting = 1,
        kConnected = 2,
        kDisconnecting = 3,
    };

public:
    Channel(EventLoop* event_loop, int fd, uint64_t id);

    virtual ~Channel();

    bool Attach();

    void Close();

    void SendMessage(std::string& message);

public:
    void SetChannelCallback(const ChannelCallback& cb) {
        channel_cb_ = cb;
    }

    void SetMessageCallback(const MessageCallback& cb) {
        recv_message_cb_ = cb;
    }

    void SetWriteCompleteCallback(const WriteCompleteCallback& cb) {
        send_complete_cb_ = cb;
    }

    void SetCloseCallback(const CloseCallback& cb) {
        channel_closed_cb_ = cb;
    }

    void SetContext(uint32_t index, const Any& context) {
        DASSERT(index < kContextMax, "Invalid index.");
        context_[index] = context;
    }

    const Any& GetContext(uint32_t index) const {
        DASSERT(index < kContextMax, "Invalid index.");
        return context_[index];
    }

    int fd() const {
        return fd_;
    }

    uint64_t id() const {
        return id_;
    }

    bool IsConnected() const {
        return status_ == kConnected;
    }

    bool IsConnecting() const {
        return status_ == kConnecting;
    }

    bool IsDisconnected() const {
        return status_ == kDisconnected;
    }

    bool IsDisconnecting() const {
        return status_ == kDisconnecting;
    }

private:
    void InternalClose();

    void InternalSendMessage(char* data, size_t len);

    void AsyncSocketReadHandle();

    void AsyncSocketWriteHandle();

    void AsyncSocketClose();

private:
    EventLoop* event_loop_;

    int fd_;

    uint64_t id_;

    std::unique_ptr<AsyncSocket> socket_;

    std::atomic<Status> status_;

    Any context_[kContextMax];

    Buffer input_buffer_;

    Buffer output_buffer_;

    ChannelCallback channel_cb_;

    MessageCallback recv_message_cb_;

    WriteCompleteCallback send_complete_cb_;

    CloseCallback channel_closed_cb_;
};

} // namespace drpc

#endif // __CHANNEL_HPP__
