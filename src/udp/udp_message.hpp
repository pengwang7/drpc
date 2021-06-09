#ifndef __UDP_MESSAGE_HPP__
#define __UDP_MESSAGE_HPP__

#include "buffer.hpp"
#include "constructor_magic.hpp"

namespace drpc {

namespace udp {

// 1472 = 1500(LINK) - 20(IP) - 8(UDP).
const int kUdpMtuMaxSize = 1472;

class Message : public Buffer {
public:
	explicit Message(int fd, uint32_t size = kUdpMtuMaxSize) : Buffer(size), fd_(fd) {
		memset(&remote_addr_, 0, sizeof(remote_addr_));
	}

	void SetRemoteAddress(const struct sockaddr& remote_addr) {
		 memcpy(&remote_addr_, &remote_addr, sizeof(remote_addr_));
	}

	struct sockaddr* GetRemoteAddress() {
		return (struct sockaddr*)&remote_addr_;
	}

	int fd() const {
		return fd_;
	}

private:
	DISALLOW_COPY_AND_ASSIGN(Message);

	int fd_;

	struct sockaddr_in remote_addr_;
};

using MessagePtr = std::shared_ptr<Message>;


inline bool SendMessage(int fd, const struct sockaddr* addr, const char* data, ssize_t len) {
	if (len <= 0) {
		return false;
	}

	ssize_t bytes_transferred = sendto(fd, data, len, 0, addr, sizeof(*addr));
	if (bytes_transferred != len) {
		return false;
	}

	return true;
}

//inline bool SendMessage(evpp_socket_t fd, const struct sockaddr* addr, const std::string& d) {
//    return SendMessage(fd, addr, d.data(), d.size());
//}
//
//inline bool SendMessage(const MessagePtr& msg) {
//    return SendMessage(msg->sockfd(), msg->remote_addr(), msg->data(), msg->size());
//}

} // namespace udp

} // namespace drpc

#endif // __UDP_MESSAGE_HPP__
