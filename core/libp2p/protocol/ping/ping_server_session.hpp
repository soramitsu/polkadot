/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef KAGOME_PING_SERVER_SESSION_HPP
#define KAGOME_PING_SERVER_SESSION_HPP

#include <memory>
#include <vector>

#include "libp2p/connection/stream.hpp"
#include "libp2p/host.hpp"

namespace libp2p::protocol {
  class PingServerSession
      : public std::enable_shared_from_this<PingServerSession> {
   public:
    explicit PingServerSession(std::shared_ptr<connection::Stream> stream);

    void start();

    void stop();

   private:
    void read();

    void readCompleted();

    void write();

    void writeCompleted();

    std::shared_ptr<connection::Stream> stream_;
    std::vector<uint8_t> buffer_;

    bool is_started_ = false;
  };
}  // namespace libp2p::protocol

#endif  // KAGOME_PING_SERVER_SESSION_HPP
