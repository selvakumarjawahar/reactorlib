#pragma once

#include "event_handler.h"
#include "utils.h"

#include <sys/epoll.h>
#include <unistd.h>

namespace lib_reactor {

constexpr auto MAX_EVENTS = 1024; // This is arbitary.
constexpr auto EPOLL_TIMEOUT_MS = 1000; // This is arbitary

/*
 * Reactor interface class. Manages Epoll
 */
class EpollReactor {
public:
    /*
     * This class is a resource manager. This manages epoll resource
     * by managing the epoll fd handle. Following Rule of 5, we do not
     * want to provide copy or move semantic for our epoll resource.
     */
    EpollReactor();
    EpollReactor(EpollReactor&) = delete;
    EpollReactor(EpollReactor&&) = delete;
    EpollReactor& operator=(EpollReactor&) = delete;
    EpollReactor& operator=(EpollReactor&&) = delete;
    ~EpollReactor();

    // API used by application to register a handler.
    ErrorCode RegisterHandler(EpollHandlerInterface* event_handler) noexcept;

    // API used by application to remove a handler
    ErrorCode RemoveHandler(const EpollHandlerInterface* event_handler) noexcept;

    // This API will start the epoll event loop and block
    void HandleEvents() noexcept;

private:
    int epoll_fd;
    int event_count = 0;
};

}
