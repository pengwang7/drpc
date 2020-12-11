#include "channel.hpp"
#include "event_loop.hpp"

namespace drpc {

Channel::Channel(EventLoop* event_loop, AsyncSocket* socket) {

}

Channel::~Channel() {

}

bool Channel::Attach() {
    return true;
}

void Channel::SendMessage(std::string message) {

}

void Channel::Close() {

}

void Channel::SetNewCallback() {

}

void Channel::SetReadMessageCallback() {

}

void Channel::SetWriteCompleteCallback() {

}

void Channel::SetTimeoutCallback() {

}

void Channel::SetClosedCallback() {

}

void Channel::AsyncSocketReadHandle() {

}

void Channel::AsyncSocketWriteHandle() {

}

void Channel::AsyncSocketCloseHandle() {

}

void Channel::AsyncSocketErrorHandle() {

}

} /* end namespace drpc */
