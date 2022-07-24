#include <catch2/catch.hpp>
#include "demux_dispatch.h"
#include "event_handler.h"
#include "utils.h"
#include <memory>
#include <zmq.hpp>
#include <zmq_addon.hpp>

using namespace iris_reactor;

class TestEventHandler : public EventHandlerInterface<zmq::socket_ref,
                                                      zmq::event_flags> {
public:

  explicit TestEventHandler(zmq::context_t & ctx,
                            zmq::event_flags event_type) : event{event_type},
    event_handle{ctx, ZMQ_PUB}
  {
    handle_ref = event_handle;
  }

  void HandleEvent(zmq::event_flags event) noexcept override
  {
    if (event == this->event) event_called++;
  }

  zmq::socket_ref GetHandle() const noexcept override
  {
    return handle_ref;
  }

  int GetEventCalledCount() const noexcept
  {
    return event_called;
  }

private:

  zmq::event_flags event;
  zmq::socket_t event_handle;
  zmq::socket_ref handle_ref;
  int event_called = 0;
};

TEST_CASE("SmokeTest", "Check basic functionalities")
{
  zmq::event_flags  event = zmq::event_flags::pollin;
  zmq::context_t    ctx;
  TestEventHandler *test_event =
    new TestEventHandler{ ctx, event };
  DemuxAndDispatch<zmq::socket_ref, zmq::event_flags> demux_dispatch;

  CHECK(demux_dispatch.AddEventToTable(test_event, event) == ErrorCode::Ok);
  CHECK(demux_dispatch.DispatchEvent(
          test_event->GetHandle(), event) == ErrorCode::Ok);
  CHECK(demux_dispatch.RemoveEventFromTable(test_event) == ErrorCode::Ok);
  delete test_event;
}

TEST_CASE("RepeatEvent", "Adding same event again")
{
  zmq::event_flags  event = zmq::event_flags::pollin;
  zmq::context_t    ctx;
  TestEventHandler *test_event =
    new TestEventHandler{ ctx, event };
  DemuxAndDispatch<zmq::socket_ref, zmq::event_flags> demux_dispatch;


  CHECK(demux_dispatch.AddEventToTable(test_event, event) == ErrorCode::Ok);
  CHECK(demux_dispatch.AddEventToTable(test_event,
                                       event) == ErrorCode::EventExists);
  delete test_event;
}

TEST_CASE("RemoveNonExistant", "Removing nonexisting event")
{
  zmq::event_flags  event = zmq::event_flags::pollin;
  zmq::context_t    ctx;
  TestEventHandler *test_event =
    new TestEventHandler{ ctx, event };
  DemuxAndDispatch<zmq::socket_ref, zmq::event_flags> demux_dispatch;

  CHECK(demux_dispatch.AddEventToTable(test_event, event) == ErrorCode::Ok);
  CHECK(demux_dispatch.RemoveEventFromTable(test_event) == ErrorCode::Ok);
  CHECK(demux_dispatch.RemoveEventFromTable(
          test_event) == ErrorCode::EventDoesNotExists);
  delete test_event;
}

TEST_CASE("CheckEventCalled", "Check event handle called")
{
  zmq::event_flags  event = zmq::event_flags::pollin;
  zmq::context_t    ctx;
  TestEventHandler *test_event =
    new TestEventHandler{ ctx, event };
  DemuxAndDispatch<zmq::socket_ref, zmq::event_flags> demux_dispatch;

  CHECK(demux_dispatch.AddEventToTable(test_event, event) == ErrorCode::Ok);
  CHECK(demux_dispatch.DispatchEvent(
          test_event->GetHandle(), event) == ErrorCode::Ok);
  CHECK(test_event->GetEventCalledCount() == 1);
  delete test_event;
}

TEST_CASE("CallingNonExistantHandler", "Call a handler which does not exist")
{
  zmq::event_flags  event = zmq::event_flags::pollin;
  zmq::context_t    ctx;
  TestEventHandler *test_event =
    new TestEventHandler{ ctx, event };
  DemuxAndDispatch<zmq::socket_ref, zmq::event_flags> demux_dispatch;

  CHECK(demux_dispatch.AddEventToTable(test_event, event) == ErrorCode::Ok);
  CHECK(demux_dispatch.DispatchEvent(
          test_event->GetHandle(), event) == ErrorCode::Ok);
  CHECK(test_event->GetEventCalledCount() == 1);
  CHECK(demux_dispatch.RemoveEventFromTable(test_event) == ErrorCode::Ok);
  CHECK(demux_dispatch.DispatchEvent(
          test_event->GetHandle(), event) == ErrorCode::EventDoesNotExists);
  delete test_event;
}

TEST_CASE("MultipleEvent", "Check with Multiple Events")
{
  zmq::context_t    ctx;
  zmq::event_flags  event1      = zmq::event_flags::pollin;
  TestEventHandler *test_event1 =
    new TestEventHandler{ ctx, event1 };
  DemuxAndDispatch<zmq::socket_ref, zmq::event_flags> demux_dispatch;

  CHECK(demux_dispatch.AddEventToTable(test_event1, event1) == ErrorCode::Ok);
  CHECK(demux_dispatch.DispatchEvent(
          test_event1->GetHandle(), event1) == ErrorCode::Ok);
  CHECK(test_event1->GetEventCalledCount() == 1);
  zmq::event_flags  event2      = zmq::event_flags::pollout;
  TestEventHandler *test_event2 =
    new TestEventHandler{ ctx, event2 };
  CHECK(demux_dispatch.AddEventToTable(test_event2, event2) == ErrorCode::Ok);
  CHECK(demux_dispatch.DispatchEvent(
          test_event2->GetHandle(), event2) == ErrorCode::Ok);
  CHECK(test_event2->GetEventCalledCount() == 1);
  zmq::event_flags  event3      = zmq::event_flags::pollpri;
  TestEventHandler *test_event3 =
    new TestEventHandler{ ctx, event3 };
  CHECK(demux_dispatch.AddEventToTable(test_event3, event3) == ErrorCode::Ok);
  CHECK(demux_dispatch.DispatchEvent(
          test_event3->GetHandle(), event3) == ErrorCode::Ok);
  CHECK(test_event3->GetEventCalledCount() == 1);
  CHECK(demux_dispatch.DispatchEvent(
          test_event1->GetHandle(), event1) == ErrorCode::Ok);
  CHECK(test_event1->GetEventCalledCount() == 2);
  CHECK(demux_dispatch.DispatchEvent(
          test_event2->GetHandle(), event2) == ErrorCode::Ok);
  CHECK(test_event2->GetEventCalledCount() == 2);
  CHECK(demux_dispatch.DispatchEvent(
          test_event3->GetHandle(), event3) == ErrorCode::Ok);
  CHECK(test_event3->GetEventCalledCount() == 2);
  CHECK(demux_dispatch.RemoveEventFromTable(test_event1) == ErrorCode::Ok);
  CHECK(demux_dispatch.RemoveEventFromTable(test_event2) == ErrorCode::Ok);
  CHECK(demux_dispatch.RemoveEventFromTable(test_event3) == ErrorCode::Ok);
  CHECK(demux_dispatch.RemoveEventFromTable(
          test_event1) == ErrorCode::EventDoesNotExists);
  CHECK(demux_dispatch.RemoveEventFromTable(
          test_event2) == ErrorCode::EventDoesNotExists);
  CHECK(demux_dispatch.RemoveEventFromTable(
          test_event3) == ErrorCode::EventDoesNotExists);
  delete test_event1;
  delete test_event2;
  delete test_event3;
}
