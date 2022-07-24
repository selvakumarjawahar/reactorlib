#pragma once
#include <exception>
#include <string>

namespace lib_reactor {
enum class ErrorCode {
    Ok,
    EpollCreateError,
    EpollAddError,
    EpollDelError,
    UnknownError
};
}
