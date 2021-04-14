/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef KAGOME_NETWORK_IMPL_ROUTER_LIBP2P_HPP
#define KAGOME_NETWORK_IMPL_ROUTER_LIBP2P_HPP

#include "network/router.hpp"

#include "application/app_configuration.hpp"
#include "application/app_state_manager.hpp"
#include "libp2p/connection/loopback_stream.hpp"
#include "libp2p/host/host.hpp"
#include "libp2p/protocol/ping.hpp"
#include "network/protocols/protocol_factory.hpp"
#include "network/sync_protocol_observer.hpp"
#include "network/types/bootstrap_nodes.hpp"
#include "network/types/own_peer_info.hpp"

namespace kagome::application {
  class ChainSpec;
}

namespace kagome::blockchain {
  class BlockStorage;
}

namespace kagome::network {
  class RouterLibp2p : public Router,
                       public std::enable_shared_from_this<RouterLibp2p> {
   public:
    RouterLibp2p(
        std::shared_ptr<application::AppStateManager> app_state_manager,
        libp2p::Host &host,
        const application::AppConfiguration &app_config,
        const OwnPeerInfo &own_info,
        const BootstrapNodes &bootstrap_nodes,
        std::shared_ptr<libp2p::protocol::Ping> ping_proto,
        std::shared_ptr<network::ProtocolFactory> protocol_factory);

    ~RouterLibp2p() override = default;

    /** @see AppStateManager::takeControl */
    bool prepare();

    /** @see AppStateManager::takeControl */
    bool start();

    /** @see AppStateManager::takeControl */
    void stop();

    std::shared_ptr<BlockAnnounceProtocol> getBlockAnnounceProtocol()
        const override;
    std::shared_ptr<GossipProtocol> getGossipProtocol() const override;
    std::shared_ptr<PropagateTransactionsProtocol>
    getPropagateTransactionsProtocol() const override;
    std::shared_ptr<SupProtocol> getSupProtocol() const override;
    std::shared_ptr<SyncProtocol> getSyncProtocol() const override;

   private:
    //    template <typename T, typename F>
    //    void readAsyncMsg(std::shared_ptr<Stream> stream, F &&f) const {
    //      auto read_writer = std::make_shared<ScaleMessageReadWriter>(stream);
    //      read_writer->read<T>([wp = weak_from_this(),
    //                            stream = std::move(stream),
    //                            f{std::forward<F>(f)}](auto &&msg_res) mutable
    //                            {
    //        auto self = wp.lock();
    //        if (not self) return;
    //
    //        if (not msg_res) {
    //          self->log_->error("error while reading message: {}",
    //                            msg_res.error().message());
    //          return stream->reset();
    //        }
    //
    //        auto peer_id_res = stream->remotePeerId();
    //        if (not peer_id_res.has_value()) {
    //          self->log_->error("can't get peer_id: {}",
    //          msg_res.error().message()); return stream->reset();
    //        }
    //
    //        if (!std::forward<F>(f)(self, peer_id_res.value(),
    //        msg_res.value())) {
    //          stream->reset();
    //          return;
    //        }
    //
    //        self->readAsyncMsg<T>(stream, std::forward<F>(f));
    //      });
    //    }
    //
    //    template <typename T,
    //              typename Handshake,
    //              typename HandshakeHandler,
    //              typename MessageHandler>
    //    void readAsyncMsgWithHandshake(std::shared_ptr<Stream> stream,
    //                                   Handshake &&handshake,
    //                                   HandshakeHandler &&hh,
    //                                   MessageHandler &&mh) const {
    //      auto read_writer = std::make_shared<ScaleMessageReadWriter>(stream);
    //      read_writer->read<std::decay_t<Handshake>>(
    //          [stream,
    //           handshake{std::move(handshake)},
    //           read_writer,
    //           wself{weak_from_this()},
    //           hh{std::forward<HandshakeHandler>(hh)},
    //           mh{std::forward<MessageHandler>(mh)}](auto &&read_res) mutable
    //           {
    //            auto self = wself.lock();
    //            if (not self) {
    //              return stream->reset();
    //            }
    //
    //            if (not read_res.has_value()) {
    //              self->log_->error("Error while reading handshake: {}",
    //                                read_res.error().message());
    //              return stream->reset();
    //            }
    //
    //            auto res = hh(self,
    //                          stream->remotePeerId().value(),
    //                          std::move(read_res.value()));
    //            if (not res.has_value()) {
    //              self->log_->error("Error while processing handshake: {}",
    //                                read_res.error().message());
    //              return stream->reset();
    //            }
    //
    //            read_writer->write(
    //                std::move(handshake),
    //                [stream, wself, mh{std::forward<MessageHandler>(mh)}](
    //                    auto &&write_res) mutable {
    //                  auto self = wself.lock();
    //                  if (not self) {
    //                    return;
    //                  }
    //
    //                  if (not write_res.has_value()) {
    //                    self->log_->error("Error while writing handshake: {}",
    //                                      write_res.error().message());
    //                    return stream->reset();
    //                  }
    //
    //                  self->readAsyncMsg<std::decay_t<T>>(
    //                      std::move(stream),
    //                      std::forward<MessageHandler>(mh));
    //                });
    //          });
    //    }

    //    void setProtocolHandler(
    //        const libp2p::peer::Protocol &protocol,
    //        void (RouterLibp2p::*method)(std::shared_ptr<Stream>) const);

    //    /**
    //     * Process a received gossip message
    //     */
    //    bool processGossipMessage(const libp2p::peer::PeerId &peer_id,
    //                              const GossipMessage &msg) const;
    //    std::shared_ptr<GossipProtocol> gossip_protocol_;
    //    std::shared_ptr<PropagateTransactionsProtocol>
    //        propagate_transaction_protocol_;
    //    std::shared_ptr<SupProtocol> sup_protocol_;
    //    std::shared_ptr<SyncProtocol> sync_protocol_;

    //    bool processSupMessage(const libp2p::peer::PeerId &peer_id,
    //                           const Status &msg) const;

    std::shared_ptr<application::AppStateManager> app_state_manager_;
    libp2p::Host &host_;
    const application::AppConfiguration &app_config_;
    //    std::shared_ptr<application::ChainSpec> chain_spec_;
    const OwnPeerInfo &own_info_;
    log::Logger log_;
    std::shared_ptr<libp2p::protocol::Ping> ping_proto_;
    std::shared_ptr<network::ProtocolFactory> protocol_factory_;

    std::shared_ptr<BlockAnnounceProtocol> block_announce_protocol_;
    std::shared_ptr<GossipProtocol> gossip_protocol_;
    std::shared_ptr<GrandpaProtocol> grandpa_protocol_;
    std::shared_ptr<PropagateTransactionsProtocol>
        propagate_transaction_protocol_;
    std::shared_ptr<SupProtocol> sup_protocol_;
    std::shared_ptr<SyncProtocol> sync_protocol_;
  };

}  // namespace kagome::network

#endif  // KAGOME_NETWORK_IMPL_ROUTER_LIBP2P_HPP
