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
    EpollHandlerInterface* event_handler) noexcept
{
    struct epoll_event event;
    event.data.fd = event_handler->GetHandle();
    event.data.ptr = event_handler;
    const auto result = epoll_ctl(epoll_fd, EPOLL_CTL_ADD, event_handler->GetHandle(), &event);
    if (result) {
        return ErrorCode::EpollAddError;
    }
    event_count++;
    return ErrorCode::Ok;
}

ErrorCode EpollReactor::RemoveHandler(
    const EpollHandlerInterface* event_handler) noexcept
{

    const auto result = epoll_ctl(epoll_fd, EPOLL_CTL_DEL, event_handler->GetHandle(), NULL);
    if (result) {
        return ErrorCode::EpollDelError;
    }
    event_count--;
    return ErrorCode::Ok;
}

void EpollReactor::HandleEvents() noexcept
{
    struct epoll_event events[MAX_EVENTS];

    while (true) {
        if (event_count < 1) // If there are no events registered, exit the event loop
            break;
        const auto event_count = epoll_wait(epoll_fd, events, MAX_EVENTS, EPOLL_TIMEOUT_MS);
        if (event_count > 0) {
            for (int i = 0; i < event_count; i++) {
                if (events[i].data.ptr != NULL) {
                    reinterpret_cast<EpollHandlerInterface*>(events[i].data.ptr)->HandleEvent(events[i].events);
                }
            }
            break;
        }
    }
}

}
