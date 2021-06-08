#pragma once

#include <memory>

#include "libp2p/host/host.hpp"
#include "libp2p/protocol/ping.hpp"
#include "network/router.hpp"

namespace kagome::network {
  class SyncRouter : public network::Router,
                     public std::enable_shared_from_this<SyncRouter> {
    libp2p::Host &host_;
    log::Logger log_;
    std::shared_ptr<libp2p::protocol::Ping> ping_proto_;
    std::shared_ptr<SyncProtocol> sync_protocol_;

   public:
    SyncRouter(libp2p::Host &host,
               std::shared_ptr<SyncProtocol> sync_protocol,
               std::shared_ptr<libp2p::protocol::Ping> ping_proto)
        : host_{host},
          log_{log::createLogger("SyncRouter", "network")},
          sync_protocol_{std::move(sync_protocol)},
          ping_proto_{std::move(ping_proto)} {
    }

    std::shared_ptr<BlockAnnounceProtocol> getBlockAnnounceProtocol()
        const override {
      return nullptr;
    }
    std::shared_ptr<GossipProtocol> getGossipProtocol() const override {
      return nullptr;
    }
    std::shared_ptr<PropagateTransactionsProtocol>
    getPropagateTransactionsProtocol() const override {
      return nullptr;
    }
    std::shared_ptr<SupProtocol> getSupProtocol() const override {
      return nullptr;
    }
    std::shared_ptr<SyncProtocol> getSyncProtocol() const override {
      return nullptr;
    }

    bool prepare() {
      host_.setProtocolHandler(
          ping_proto_->getProtocolId(), [wp = weak_from_this()](auto &&stream) {
            if (auto self = wp.lock()) {
              if (auto peer_id = stream->remotePeerId()) {
                self->log_->info("Handled {} protocol stream from: {}",
                                 self->ping_proto_->getProtocolId(),
                                 peer_id.value().toBase58());
                self->ping_proto_->handle(
                    std::forward<decltype(stream)>(stream));
              }
            }
          });

      sync_protocol_->start();
    }

    bool start() {
      host_.start();
    }

    bool stop() {
      if(host_.getNetwork().getListener().isStarted()) {
        host_.stop();
      }
    }
  };
}  // namespace kagome::network
