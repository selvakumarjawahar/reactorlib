#pragma once
#include <stdint.h>
namespace lib_reactor {

/*
     * This is event handler interface, abstract class.
     * The handle type and event type is parameterized using templates
     */
template <typename HANDLE_TYPE, typename EVENT_TYPE>
class EventHandlerInterface {
public:
    virtual void HandleEvent(EVENT_TYPE event) noexcept = 0;
    virtual HANDLE_TYPE GetHandle() const noexcept = 0;

protected:
    virtual ~EventHandlerInterface() {};
};

/*
 * In this library we are using epoll as our event demultiplexer. 
 * This alias gives us EventHandlerInterface with handle type as int and 
 * event type as uint32_t as defined by epoll
 */
using EpollHandlerInterface = EventHandlerInterface<int, uint32_t>;

}
