#include <catch2/catch.hpp>
#include "event_handler.h"
#include "utils.h"
#include "zmq_reactor.h"

#include <memory>
#include <zmq.hpp>
#include <exception>
#include <string>
#include <iostream>

#include <thread>
#include <chrono>
#include <zmq_addon.hpp>

using namespace iris_reactor;

const std::string connection_string_pub_sub = "inproc://127.0.0.1:5563";
const std::string connection_string_dealer_router = "inproc://127.0.0.1:5671";

const std::string test_msg_pub_sub = "This is a pub sub zmq test";
const std::string test_msg_router_dealer = "This is a router dealer zmq test";

class ZMQSubEventHandler : public EventHandlerInterface<zmq::socket_ref,
                                                         zmq::event_flags> {
public:

  ZMQSubEventHandler(zmq::context_t& ctx, zmq::event_flags event_type,
                      ZMQReactor& reactor_ref) :
    sub_socket{ctx, zmq::socket_type::sub},
    sub_socket_ref{sub_socket},
    event{event_type},
    reactor{reactor_ref}
  {
    sub_socket.connect(connection_string_pub_sub);
    sub_socket.set(zmq::sockopt::subscribe, "");
  }

  void HandleEvent(zmq::event_flags event) noexcept override
  {
    if (event != this->event) return;

    event_called++;
    zmq::message_t msg;
    const auto ret = sub_socket.recv(msg, zmq::recv_flags::none);
    if (ret.has_value())
    {
      msg_str = msg.to_string();
      reactor.RemoveEvent(this);
    }
  }

  zmq::socket_ref GetHandle() const noexcept override
  {
    return sub_socket_ref;
  }

  int GetEventCalledCount() const noexcept
  {
    return event_called;
  }

  std::string GetMsgString() const noexcept
  {
    return msg_str;
  }

private:

  zmq::socket_t sub_socket;
  zmq::socket_ref sub_socket_ref;
  zmq::event_flags event;
  ZMQReactor& reactor;
  int event_called = 0;
  std::string msg_str;
};

static void pub_send(zmq::context_t& ctx)
{
  zmq::socket_t pub_socket{ ctx, zmq::socket_type::pub };

  pub_socket.bind(connection_string_pub_sub);
  pub_socket.send(zmq::buffer(test_msg_pub_sub),
                  zmq::send_flags::dontwait);
}


class ZMQRouterEventHandler : public EventHandlerInterface<zmq::socket_ref,
                                                         zmq::event_flags> {
public:

  ZMQRouterEventHandler(zmq::context_t& ctx, zmq::event_flags event_type,
                      ZMQReactor& reactor_ref) :
    router_socket{ctx, zmq::socket_type::router},
    router_socket_ref{router_socket},
    event{event_type},
    reactor{reactor_ref}
  {
    router_socket.bind(connection_string_dealer_router);
  }

  void HandleEvent(zmq::event_flags event) noexcept override
  {
    if (event != this->event) return;

    event_called++;
    zmq::multipart_t msg(router_socket);
    if (msg.size() > 0)
    {
      msg_str = msg.back().to_string();
      reactor.RemoveEvent(this);
    }
  }

  zmq::socket_ref GetHandle() const noexcept override
  {
    return router_socket_ref;
  }

  int GetEventCalledCount() const noexcept
  {
    return event_called;
  }

  std::string GetMsgString() const noexcept
  {
    return msg_str;
  }


private:

  zmq::socket_t router_socket;
  zmq::socket_ref router_socket_ref;
  zmq::event_flags event;
  ZMQReactor& reactor;
  int event_called = 0;
  std::string msg_str;
};

static void dealer_send(zmq::context_t& ctx)
{
  zmq::socket_t dealer_socket{ ctx, zmq::socket_type::dealer };

  dealer_socket.connect(connection_string_dealer_router);
  dealer_socket.send(zmq::buffer(test_msg_router_dealer),
                  zmq::send_flags::none);
}


TEST_CASE("AddingPubSub", "Check Adding pub sub socket")
{
  using namespace std::chrono_literals;
  zmq::context_t context;
  ZMQReactor     reactor{};
  auto zmq_sub_event1 = std::make_unique<ZMQSubEventHandler>(context,
                                                              zmq::event_flags::pollin,
                                                              reactor);
  auto zmq_sub_event2 = std::make_unique<ZMQSubEventHandler>(context,
                                                              zmq::event_flags::pollin,
                                                              reactor);
  CHECK(reactor.AddEvent(
          zmq_sub_event1.get(), zmq::event_flags::pollin) == ErrorCode::Ok);
  CHECK(reactor.AddEvent(
          zmq_sub_event2.get(), zmq::event_flags::pollin) == ErrorCode::Ok);
 
  std::thread publisher{ pub_send, std::ref(context) };
  CHECK(reactor.StartReactor(1000ms) == ErrorCode::Ok);
  publisher.join();
  CHECK(zmq_sub_event1->GetEventCalledCount() == 1);
  CHECK(zmq_sub_event1->GetMsgString() == test_msg_pub_sub);
  CHECK(zmq_sub_event2->GetEventCalledCount() == 1);
  CHECK(zmq_sub_event2->GetMsgString() == test_msg_pub_sub);
}


TEST_CASE("AddingDealerRouter", "Check Adding dealer router socket")
{
  using namespace std::chrono_literals;
  zmq::context_t context;
  ZMQReactor     reactor{};
  auto zmq_router_event1 = std::make_unique<ZMQRouterEventHandler>(context,
                                                              zmq::event_flags::pollin,
                                                              reactor);
 CHECK(reactor.AddEvent(
          zmq_router_event1.get(), zmq::event_flags::pollin) == ErrorCode::Ok);

  std::thread dealer{ dealer_send, std::ref(context) };
  CHECK(reactor.StartReactor(1000ms) == ErrorCode::Ok);
  dealer.join();
  CHECK(zmq_router_event1->GetEventCalledCount() == 1);
  CHECK(zmq_router_event1->GetMsgString() == test_msg_router_dealer);
}

TEST_CASE("AddingMixed", "Check Adding dealer router socket and pub sub")
{
  using namespace std::chrono_literals;
  zmq::context_t context;
  ZMQReactor     reactor{};
  auto zmq_sub_event1 = std::make_unique<ZMQSubEventHandler>(context,
                                                              zmq::event_flags::pollin,
                                                              reactor);
 CHECK(reactor.AddEvent(
          zmq_sub_event1.get(), zmq::event_flags::pollin) == ErrorCode::Ok);
 auto zmq_router_event1 = std::make_unique<ZMQRouterEventHandler>(context,
                                                              zmq::event_flags::pollin,
                                                              reactor);
 CHECK(reactor.AddEvent(
          zmq_router_event1.get(), zmq::event_flags::pollin) == ErrorCode::Ok);


  std::thread publisher{ pub_send, std::ref(context) };
  std::thread dealer{ dealer_send, std::ref(context) };
  CHECK(reactor.StartReactor(1000ms) == ErrorCode::Ok);
  publisher.join();
  dealer.join();
  CHECK(zmq_sub_event1->GetEventCalledCount() == 1);
  CHECK(zmq_sub_event1->GetMsgString() == test_msg_pub_sub);
  CHECK(zmq_router_event1->GetEventCalledCount() == 1);
  CHECK(zmq_router_event1->GetMsgString() == test_msg_router_dealer);

}
