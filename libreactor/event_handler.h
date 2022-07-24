#pragma once
namespace lib_reactor {
template <typename HANDLE_TYPE, typename EVENT_TYPE>
class EventHandlerInterface {
public:
    virtual void HandleEvent(EVENT_TYPE event) noexcept = 0;
    virtual HANDLE_TYPE GetHandle() const noexcept = 0;

protected:
    virtual ~EventHandlerInterface() {};
};
}
