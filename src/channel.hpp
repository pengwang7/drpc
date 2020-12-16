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
