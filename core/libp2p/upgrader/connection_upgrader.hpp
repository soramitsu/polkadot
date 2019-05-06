/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef KAGOME_CORE_LIBP2P_UPGRADER_CONNECTION_UPGRADER_HPP
#define KAGOME_CORE_LIBP2P_UPGRADER_CONNECTION_UPGRADER_HPP

#include <memory>

#include "libp2p/muxer/yamux/yamux.hpp"
#include "libp2p/transport/muxed_connection.hpp"

namespace libp2p::upgrader {

  /**
   * @brief connection type
   */
  enum class ConnectionType { SERVER_SIDE, CLIENT_SIDE };

  /**
   * @class MuxerOptions muxer options
   */
  struct MuxerOptions {
    ConnectionType connection_type_;
  };

  /**
   * @class ConnectionUpgrader connection upgrader
   */
  class ConnectionUpgrader {
   public:
    virtual ~ConnectionUpgrader() = default;

    using NewStreamHandler = muxer::Yamux::NewStreamHandler;
    using ConnectionPtr = std::shared_ptr<transport::Connection>;

    /**
     * @brief upgrades connection to muxed
     * @param connection shared ptr to connection instance
     * @param handler generic protocol handler
     * @return muxed connection instance
     */
    virtual outcome::result<std::unique_ptr<transport::MuxedConnection>>
    upgradeToMuxed(ConnectionPtr connection,
                   NewStreamHandler handler) const = 0;
  };
}  // namespace libp2p::upgrader

#endif  // KAGOME_CORE_LIBP2P_UPGRADER_CONNECTION_UPGRADER_HPP
