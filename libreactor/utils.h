#pragma once
#include <exception>
#include <string>

namespace lib_reactor {
enum class ErrorCode {
    Ok,
    EventExists,
    EventDoesNotExists,
    InvalidEventHandler,
    EpollCreateError,
    UnknownError
};
}
