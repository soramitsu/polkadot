/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "libp2p/event/emitter.hpp"

#include <gtest/gtest.h>

using namespace libp2p::event;

class EventEmitterTest : public ::testing::Test {
 public:
  struct ConnectionOpened {
    std::string my_str;
  };
  struct ConnectionPaused {};
  struct ConnectionClosed {
    int code1;
    int code2;
  };
};

/**
 * @given event emitter with several events
 * @when subscribing to those events @and emitting them
 * @then events are successfully emitted
 */
TEST_F(EventEmitterTest, EmitEvents) {
  Emitter<ConnectionOpened, ConnectionPaused, ConnectionClosed> emitter{};

  std::string connection_opened{};
  auto connection_paused = -1;
  auto connection_closed_n = -1;
  auto connection_closed_k = -1;

  emitter.on<ConnectionOpened>([&connection_opened](ConnectionOpened event) {
    connection_opened = event.my_str;
  });
  emitter.on<ConnectionPaused>(
      [&connection_paused](ConnectionPaused) { connection_paused = 10; });
  emitter.on<ConnectionClosed>(
      [&connection_closed_n, &connection_closed_k](ConnectionClosed event) {
        connection_closed_n = event.code1;
        connection_closed_k = event.code2;
      });

  emitter.emit(ConnectionOpened{"foo"});
  emitter.emit(ConnectionPaused{});
  emitter.emit(ConnectionClosed{2, 5});

  ASSERT_EQ(connection_opened, "foo");
  ASSERT_EQ(connection_paused, 10);
  ASSERT_EQ(connection_closed_n, 2);
  ASSERT_EQ(connection_closed_k, 5);
}

/**
 * @given event emitter
 * @when subscribing to the event @and unsubscribing from it
 * @then operations execute successfully
 */
TEST_F(EventEmitterTest, Unsubscribe) {
  Emitter<ConnectionOpened, ConnectionPaused, ConnectionClosed> emitter{};

  auto event_res = -1;

  auto subscription = emitter.on<ConnectionPaused>([&event_res](ConnectionPaused) {
    event_res++;
  });
  emitter.emit(ConnectionPaused{});
  ASSERT_EQ(event_res, 0);

  subscription.unsubscribe();
  emitter.emit(ConnectionPaused{});
  ASSERT_EQ(event_res, 0);
}
