#pragma once

#include "event_handler.h"
#include "handler_map.h"
#include "utils.h"

#include <algorithm>
#include <chrono>
#include <string>
#include <vector>
#include <sys/epoll.h>
#include <unistd.h>
#include <unordered_map>

namespace lib_reactor {
using EpollHandlerInterface = EventHandlerInterface<int, uint32_t>;

constexpr auto MAX_EVENTS = 1024; // This is arbitary.
constexpr auto EPOLL_TIMEOUT_MS = 1000; // This is arbitary
class EpollReactor {
public:
    EpollReactor();
    EpollReactor(EpollReactor&) = delete;
    EpollReactor(EpollReactor&&) = delete;
    EpollReactor& operator=(EpollReactor&) = delete;
    EpollReactor& operator=(EpollReactor&&) = delete;

    ErrorCode RegisterHandler(EpollHandlerInterface* event_handler, uint32_t event_flag) noexcept;
    ErrorCode RemoveHandler(const EpollHandlerInterface* event_handler) noexcept;
    ErrorCode HandleEvents() noexcept;
    ~EpollReactor();

private:
    int epoll_fd;
    std::unordered_map<int , EpollHandlerInterface*> handler_map;

};

}
