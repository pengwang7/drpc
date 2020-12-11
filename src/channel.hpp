#ifndef __CHANNEL_HPP__
#define __CHANNEL_HPP__

#include "async_socket.hpp"
#include <memory>
namespace drpc {

class EventLoop;

class Channel : public std::enable_shared_from_this<Channel> {
public:
    Channel(EventLoop* event_loop, AsyncSocket* socket);

    ~Channel();

    bool Attach();

    void SendMessage(std::string message);

    void Close();

    void SetNewCallback();

    void SetReadMessageCallback();

    void SetWriteCompleteCallback();

    void SetTimeoutCallback();

    void SetClosedCallback();

private:
    void AsyncSocketReadHandle();

    void AsyncSocketWriteHandle();

    void AsyncSocketCloseHandle();

    void AsyncSocketErrorHandle();

private:
    EventLoop* event_loop_;

    std::unique_ptr<AsyncSocket> async_socket_;
};

} /* end namespace drpc */

#endif /* __CHANNEL_HPP__ */
