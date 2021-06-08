#pragma once

#include <memory>

#include "log/logger.hpp"
#include "network/sync_protocol_observer.hpp"

namespace kagome::network {
  class SyncObserver : public SyncProtocolObserver,
                       public std::enable_shared_from_this<SyncObserver> {
    log::Logger log_;

   public:
    SyncObserver() : log_{log::createLogger("SyncObserver", "network")} {}
    outcome::result<BlocksResponse> onBlocksRequest(
        const BlocksRequest &request) const override {
      BlocksResponse response{request.id};
      return response;
    }
  };
}  // namespace kagome::network
