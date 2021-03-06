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

#include "logger.hpp"
#include "channel.hpp"
#include "event_loop.hpp"

namespace drpc {

Channel::Channel(AsyncSocket* ast) {
    DASSERT(event_loop_ = ast->event_loop(), "Channel constructor error.");

    // The array size must be > 8.
    char p[8] = {0};
    snprintf(p, sizeof(p), "%d", ast->fd2());
    csid_ = std::string(p);

    async_socket_.reset(ast);
    DASSERT(async_socket_, "Channel constructor error.");

    DTRACE("Create channel success, csid: %s.", csid_.c_str());
}

Channel::~Channel() {
    DTRACE("Destroy channel: %p.", this);
    async_socket_->Close();
}

void Channel::Init() {
    // Do't set callbacks in the constructor because you will 
    // not be able to use shared_from_this() in the constructor,
    // and using this bind callback can cause some potential problems.
    async_socket_->SetReadCallback(std::bind(&Channel::AsyncSocketReadHandle, shared_from_this()));
    async_socket_->SetWriteCallback(std::bind(&Channel::AsyncSocketWriteHandle, shared_from_this()));
}

bool Channel::Attach() {
    DASSERT(event_loop_->IsConsistent(), "Channel error.");

    async_socket_->ModifyIOEvents(true, false);
    if (async_socket_->IsNone()) {
        DERROR("Channel attach to event loop failed, events is none.");
        return false;
    }

    if (!event_loop_->IncRegisterSize()) {
        DWARNING("Channel attach to event loop failed, too many channels for this loop.");
        return false;
    }

    if (!async_socket_->Attach()) {
        DERROR("Channel attach to event loop failed.");
        return false;
    }

    DASSERT(async_socket_->IsAttached(), "Channel attached flag not set.");

    if (new_channel_cb_) {
        new_channel_cb_(shared_from_this());
    }

    return true;
}

void Channel::SendMessage(std::string& message) {
    if (!message.size()) {
        DERROR("Channel will send message is nil.");
        return;
    }

    event_loop_->SendToQueue(std::bind(&Channel::InternalSendMessage, shared_from_this(), const_cast<char*>(message.c_str()), message.size()));

    DTRACE("Channel push send_fn to event_loop queue.");
}

void Channel::Close() {
    // NOTE: Lambda capture initializers only available with -std=c++14 or -std=gnu++14.
    auto self = shared_from_this();
    auto fn = [self]() -> void {
        DASSERT(self->event_loop_->IsConsistent(), "Channel error.");

        self->async_socket_->DisableAllIOEvents();
        self->async_socket_->Close();
        self->event_loop_->DecRegisterSize();

        if (self->channel_closed_cb_) {
            self->channel_closed_cb_(self);
        }

        // Need to set context_ is nullptr, 
        // Otherwise the reference count will not be freed.
        self->context_ = nullptr;
    };

    event_loop_->SendToQueue(fn);
}

void Channel::SetNewChannelCallback(const NewChannelCallback& cb) {
    new_channel_cb_ = cb;
}

void Channel::SetRecvMessageCallback(const RecvMessageCallback& cb) {
    recv_message_cb_ = cb;
}

void Channel::SetSendCompleteCallback(const SendCompleteCallback& cb) {
    send_complete_cb_ = cb;
}

void Channel::SetTimedoutCallback(const TimedoutCallback& cb) {
    channel_timeout_cb_ = cb;
}

void Channel::SetCloseCallback(const CloseCallback& cb) {
    channel_closed_cb_ = cb;
}

void Channel::SetAnyContext(const any& context) {
    context_ = context;
}

any& Channel::GetAnyContext() {
    return context_;
}

std::string Channel::csid() const {
    return csid_;
}

void Channel::InternalSendMessage(char* data, size_t len) {
    DASSERT(event_loop_->IsConsistent(), "Channel error.");

    ssize_t bytes_transferred = 0;
    size_t remaining_bytes = len;

    // If the async_socket_ is writable and send_buffer_ unread bytes is not zero,
    // it means had remaining data at before time. Ensure that send data order.
    // First send remaining data, last send current data.
    if (!async_socket_->IsWritable() && !send_buffer_.UnreadByteSize()) {
        bytes_transferred = send(async_socket_->fd2(), data, len, MSG_NOSIGNAL);
        if (bytes_transferred >= 0) {
            remaining_bytes -= bytes_transferred;
            if (!remaining_bytes && send_complete_cb_) {
                send_complete_cb_(shared_from_this());
            }
        } else if (EVUTIL_ERR_RW_RETRIABLE(errno)) {
            bytes_transferred = 0;
        } else {
            Close();
        }
    }

    // Append remaining data to send_buffer_.
    if (remaining_bytes > 0) {
        send_buffer_.AppendData(data + bytes_transferred, remaining_bytes);
        async_socket_->ModifyIOEvents(true, true);
    }
}

void Channel::AsyncSocketReadHandle() {
    DASSERT(event_loop_->IsConsistent(), "Channel error.");

    DTRACE("Channel will be recv peer data.");

    ssize_t bytes_transferred = recv_buffer_.RecvData(async_socket_->fd2());
    if (bytes_transferred <= 0) {
        if (bytes_transferred == 0) {
            DDEBUG("Peer socket is closed.");
        } else {
            DERROR("The buffer recv data failed: %s.", strerror(errno));
        }
        Close();
        return;
    }

    if (recv_message_cb_) {
        recv_message_cb_(shared_from_this(), recv_buffer_);
    }
}

void Channel::AsyncSocketWriteHandle() {
    DASSERT(event_loop_->IsConsistent(), "Channel error.");
    DASSERT(async_socket_->IsWritable(), "Channel is not writable.");

    ssize_t bytes_transferred = send(async_socket_->fd2(), send_buffer_.begin(), send_buffer_.UnreadByteSize(), MSG_NOSIGNAL);
    if (bytes_transferred > 0) {
        ByteBufferReader io_reader(send_buffer_);
        io_reader.Consume(static_cast<size_t>(bytes_transferred));

        if (!send_buffer_.UnreadByteSize()) {
            async_socket_->ModifyIOEvents(true, false);
            if (send_complete_cb_) send_complete_cb_(shared_from_this());
        }
    } else if (!EVUTIL_ERR_RW_RETRIABLE(errno)) {
        Close();
    }
}

} /* end namespace drpc */
