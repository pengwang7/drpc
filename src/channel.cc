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

Channel::Channel(EventLoop* event_loop, int fd, uint64_t id)
    : event_loop_(event_loop), fd_(fd), id_(id), status_(kDisconnected) {

    DTRACE("Create channel fd={}, id={}.", fd_, id_);

    if (fd_ >= 0) {
        socket_.reset(new AsyncSocket(event_loop_, fd, kNoneEvent));
        socket_->SetReadCallback(std::bind(&Channel::AsyncSocketReadHandle, this));
        socket_->SetWriteCallback(std::bind(&Channel::AsyncSocketWriteHandle, this));
    }
}

Channel::~Channel() {
    DASSERT(status_ == kDisconnected, "Channel status error.");

    DTRACE("Destroy channel fd={}, id={}.", fd_, id_);

    if (fd_ >= 0) {
        DASSERT(socket_, "Channel socket ref error.");
        DASSERT(fd_ == socket_->fd(), "Channel fd error.");
        close(fd_);
        fd_ = -1;
    }
}

bool Channel::Attach() {
    DASSERT(event_loop_->IsConsistent(), "Channel error.");

    status_ = kConnected;

    socket_->ModifyIOEvents(true, false);
    if (socket_->IsNone()) {
        DERROR("Channel attach to event loop failed, events is none.");
        return false;
    }

    DASSERT(socket_->IsAttached(), "Channel socket not attach.");

    if (channel_cb_) {
        channel_cb_(shared_from_this());
    }

    return true;
}

void Channel::Close() {
    status_ = kDisconnecting;

    // NOTE: Lambda capture initializers only available with
    // -std=c++14 or -std=gnu++14.
    auto self = shared_from_this();
    auto fn = [self]() -> void {
        DASSERT(self->event_loop_->IsConsistent(), "Channel error.");
        self->InternalClose();
    };

    event_loop_->QueueInLoop(fn);
}

void Channel::SendMessage(std::string& message) {
    if (status_ != kConnected || message.empty()) {
        return;
    }

    if (event_loop_->IsConsistent()) {
        InternalSendMessage(const_cast<char*>(message.c_str()), message.size());
    } else {
        event_loop_->RunInLoop(std::bind(&Channel::InternalSendMessage, shared_from_this(), const_cast<char*>(message.c_str()), message.size()));
    }
}

void Channel::InternalClose() {
    DASSERT(event_loop_->IsConsistent(), "Channel error.");

    // Avoid multi calling.
    if (status_ == kDisconnected) {
        return;
    }

    DASSERT(status_ == kDisconnecting, "Channel status error.");

    socket_->DisableAllIOEvents();
    socket_->Close();

    // When direct used shared_from_this(), status_ error.(heap-use-after-free)
    ChannelPtr self(shared_from_this());

    if (channel_cb_) {
        channel_cb_(self);
    }

    if (channel_closed_cb_) {
        channel_closed_cb_(self);
    }

    status_ = kDisconnected;
}

void Channel::InternalSendMessage(char* data, size_t len) {
    DASSERT(event_loop_->IsConsistent(), "Channel error.");

    if (status_ == kDisconnected) {
        DWARNING("Channel status is kDisconnected, give up writing.");
        return;
    }

    ssize_t bytes_transferred = 0;
    size_t remaining_bytes = len;

    // If the async_socket_ is writable and send_buffer_ unread bytes is not zero,
    // it means had remaining data at before time. Ensure that send data order.
    // First send remaining data, last send current data.
    if (!socket_->IsWritable() && !output_buffer_.UnreadByteSize()) {
        bytes_transferred = send(socket_->fd(), data, len, MSG_NOSIGNAL);
        if (bytes_transferred >= 0) {
            remaining_bytes -= bytes_transferred;
            if (!remaining_bytes && send_complete_cb_) {
                send_complete_cb_(shared_from_this());
            }
        } else if (EVUTIL_ERR_RW_RETRIABLE(errno)) {
            bytes_transferred = 0;
        } else {
            status_ = kDisconnecting;
            InternalClose();
        }
    }

    // Append remaining data to send_buffer_.
    if (remaining_bytes > 0) {
        output_buffer_.AppendData(data + bytes_transferred, remaining_bytes);
        socket_->ModifyIOEvents(true, true);
    }
}

void Channel::AsyncSocketReadHandle() {
    DASSERT(event_loop_->IsConsistent(), "Channel error.");

    ssize_t bytes_transferred = input_buffer_.RecvData(socket_->fd());
    if (bytes_transferred <= 0) {
        // Ignore error:
        // errno == EINTR || errno == EAGAIN.
        if (bytes_transferred == RETRIABLE_ERROR) {
            return;
        }

        status_ = kDisconnecting;
        InternalClose();

        return;
    }

    if (recv_message_cb_) {
        recv_message_cb_(shared_from_this(), &input_buffer_);
    }
}

void Channel::AsyncSocketWriteHandle() {
    DASSERT(event_loop_->IsConsistent(), "Channel error.");
    DASSERT(socket_->IsWritable(), "Channel is not writable.");

    ssize_t bytes_transferred = send(socket_->fd(), output_buffer_.begin(), output_buffer_.UnreadByteSize(), MSG_NOSIGNAL);
    if (bytes_transferred > 0) {
        ByteBufferReader io_reader(output_buffer_);
        io_reader.Consume(static_cast<size_t>(bytes_transferred));

        if (!output_buffer_.UnreadByteSize()) {
            socket_->ModifyIOEvents(true, false);
            if (send_complete_cb_) send_complete_cb_(shared_from_this());
        }
    } else if (!EVUTIL_ERR_RW_RETRIABLE(errno)) {
        status_ = kDisconnecting;
        InternalClose();
    }
}

} // namespace drpc
