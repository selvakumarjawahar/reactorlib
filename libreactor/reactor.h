#pragma once

#include "event_handler.h"
#include "utils.h"

#include <sys/epoll.h>
#include <unistd.h>

namespace lib_reactor {

constexpr auto MAX_EVENTS = 1024; // This is arbitary.
constexpr auto EPOLL_TIMEOUT_MS = 1000; // This is arbitary
class EpollReactor {
public:
    EpollReactor();
    EpollReactor(EpollReactor&) = delete;
    EpollReactor(EpollReactor&&) = delete;
    EpollReactor& operator=(EpollReactor&) = delete;
    EpollReactor& operator=(EpollReactor&&) = delete;

    ErrorCode RegisterHandler(EpollHandlerInterface* event_handler) noexcept;
    ErrorCode RemoveHandler(const EpollHandlerInterface* event_handler) noexcept;
    void HandleEvents() noexcept;
    ~EpollReactor();

private:
    int epoll_fd;
    int event_count = 0;
};

}
