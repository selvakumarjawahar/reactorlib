#pragma once

#include "event_handler.h"
#include "utils.h"
#include <unordered_map>
namespace lib_reactor {
template <typename HANDLE_TYPE, typename EVENT_TYPE>
class HandlerMap {
public:
    ErrorCode AddEventToTable(
        EventHandlerInterface<HANDLE_TYPE, EVENT_TYPE>* event_handler,
        EVENT_TYPE event) noexcept
    {
        if (!(event_handler && event_handler->GetHandle()))
            return ErrorCode::InvalidEventHandler;

        const auto result = demux_table.emplace(
            event_handler->GetHandle(), EventInfo { event_handler, event });

        if (result.second)
            return ErrorCode::Ok;

        return ErrorCode::EventExists;
    }

    ErrorCode RemoveEventFromTable(
        const EventHandlerInterface<HANDLE_TYPE, EVENT_TYPE>* event_handler) noexcept
    {
        if (!(event_handler && event_handler->GetHandle()))
            return ErrorCode::InvalidEventHandler;

        const auto result = demux_table.erase(event_handler->GetHandle());

        if (result != 1)
            return ErrorCode::EventDoesNotExists;

        return ErrorCode::Ok;
    }

    ErrorCode DispatchEvent(HANDLE_TYPE event_handle,
        EVENT_TYPE event) const noexcept
    {
        if (!event_handle)
            return ErrorCode::InvalidEventHandler;

        auto event_info_itr = demux_table.find(event_handle);

        if (event_info_itr == demux_table.end())
            return ErrorCode::EventDoesNotExists;

        auto event_info = event_info_itr->second;
        event_info.event_handler->HandleEvent(event);
        return ErrorCode::Ok;
    }

    std::size_t EventCount() const noexcept
    {
        return demux_table.size();
    }

private:
    struct EventInfo {
        EventHandlerInterface<HANDLE_TYPE, EVENT_TYPE>* event_handler = nullptr;
        EVENT_TYPE event {};
    };
    std::unordered_map<
        HANDLE_TYPE, EventInfo>
        demux_table;
};
}
