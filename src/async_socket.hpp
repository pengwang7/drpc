#ifndef __ASYNC_SOCKET_HPP__
#define __ASYNC_SOCKET_HPP__

#include <ev.h>
#include <functional>

namespace drpc {

#define DRPC_NONE_EVENT         (0x00)
            
#define DRPC_READ_EVENT         (0x01)
            
#define DRPC_WRITE_EVENT        (0x02)

class EventLoop;

class AsyncSocket {
public:
    using AsyncSocketCallback = std::function<void()>;

public:
    AsyncSocket(EventLoop* event_loop, int fd, unsigned short io_events);

    ~AsyncSocket();

public:
    bool Attach();

    bool Detach();

    void Close();

    void SetReadCallback(const AsyncSocketCallback& cb);

    void SetWriteCallback(const AsyncSocketCallback& cb);

    void ModifyIOEvents(bool readable, bool writable);

    void DisableAllIOEvents();

    bool IsAttached() const;

    bool IsNone() const;

    bool IsReadable() const;

    bool IsWritable() const;

    int fd2() const;

private:
    void IOEventHandle(int events);

    static void IOEventHandle(struct ev_loop* loop, struct ev_io* io, int events);

private:
    EventLoop* event_loop_;

    struct ev_io* io_;

    unsigned short events_;

    bool attached_;

    AsyncSocketCallback read_cb_;

    AsyncSocketCallback write_cb_;
};

} /* end namespace drpc */

#endif /* __ASYNC_SOCKET_HPP__ */
