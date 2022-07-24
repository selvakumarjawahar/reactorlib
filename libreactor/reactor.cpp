#include "reactor.h"

namespace lib_reactor {

EpollReactor::EpollReactor()
{
    epoll_fd = epoll_create1(0);
    if (epoll_fd < 0)
        throw ErrorCode::EpollCreateError;
}

EpollReactor::~EpollReactor()
{
    close(epoll_fd);
}

ErrorCode EpollReactor::RegisterHandler(
    EpollHandlerInterface* event_handler,
    uint32_t event_flag) noexcept
{
    struct epoll_event event;
    event.data.fd = event_handler->GetHandle();
    event.data.ptr = NULL;
    event.events = event_flag;
    epoll_ctl(epoll_fd, EPOLL_CTL_ADD, event_handler->GetHandle(),&event);
    handler_map[event_handler->GetHandle()] = event_handler;
    return ErrorCode::Ok;
}

ErrorCode EpollReactor::RemoveHandler(
    const EpollHandlerInterface* event_handler) noexcept
{
    epoll_ctl(epoll_fd, EPOLL_CTL_DEL, event_handler->GetHandle(),NULL);
    return ErrorCode::Ok;

}

ErrorCode EpollReactor::HandleEvents() noexcept
{
    bool run_event_loop = true;
    ErrorCode err = ErrorCode::Ok;
    struct epoll_event events[MAX_EVENTS];

    while (run_event_loop) {
        auto event_count  = epoll_wait(epoll_fd, events, MAX_EVENTS, EPOLL_TIMEOUT_MS);
        if (event_count > 0)
        {
        }
    }
    return err;
}

}
