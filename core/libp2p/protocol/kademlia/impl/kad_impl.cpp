/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "libp2p/protocol/kademlia/impl/kad_impl.hpp"

#include "common/blob.hpp"
#include "crypto/sha/sha256.hpp"
#include "libp2p/protocol/kademlia/query.hpp"

namespace {

  using kagome::common::Hash256;
  using kagome::crypto::sha256;
  using libp2p::peer::PeerId;
  using libp2p::protocol::kademlia::NodeId;

  NodeId hash(const PeerId &pid) {
    return NodeId(sha256(pid.toVector()));
  }

}  // namespace

namespace libp2p::protocol::kademlia {

  void KadImpl::findPeer(peer::PeerId id, PeerRouting::FindPeerResultFunc f) {
    auto local = findLocal(id);
    auto connectedness = network_.getConnectionManager().connectedness(local);
    using E = network::ConnectionManager::Connectedness;
    switch (connectedness) {
      case E::CAN_CONNECT:
      case E::CONNECTED:
        // we know addresses of that peer, return local view
        return f(local);
      default:
        // we don't know addresses of that peer. continue.
        break;
    }

    return table_->getNearestPeers(
        hash(id),       /// NodeId
        config_.ALPHA,  /// concurrency
        [this, id,
         f{std::move(f)}](outcome::result<PeerIdVec> rpeers) mutable -> void {
          if (!rpeers) {
            // lookup failure
            return f(rpeers.error());
          }

          PeerIdVec &peers = rpeers.value();
          for (peer::PeerId &p : peers) {
            if (p == id) {
              // found target peer in list of closest peers
              return f(findLocal(id));
            }
          }

          // create query
          Query query{
              id.toVector(),  /// find this peer
              [this, id](
                  const Key &key,
                  std::function<void(outcome::result<QueryResult>)> handle)
                  -> void {
                mrw_->findPeerSingle(
                    key,  /// ask this peer (serialized NodeId)
                    id,   /// where peer 'id' is
                    [id, handle{std::move(handle)}](
                        outcome::result<PeerInfoVec> r) {
                      if (!r) {
                        return handle(r.error());
                      }

                      // see if we got the peer here
                      auto &&list = r.value();
                      for (auto &p : list) {
                        if (p.id == id) {
                          // yey!
                          return handle(
                              QueryResult{.peer = p, .success = true});
                        }
                      }

                      return handle(QueryResult{.closerPeers = list});
                    });
              }};

          return runner_->run(std::move(query), std::move(peers),
                              std::bind(f, std::placeholders::_1));
        });
  }

  peer::PeerInfo KadImpl::findLocal(const peer::PeerId &id) {
    auto r = repo_->getAddressRepository().getAddresses(id);
    if (r) {
      return {id, r.value()};
    }

    return {id, {}};
  }

  KadImpl::KadImpl(network::Network &network,
                   std::shared_ptr<peer::PeerRepository> repo,
                   std::shared_ptr<RoutingTable> table,
                   std::shared_ptr<MessageReadWriter> mrw,
                   std::shared_ptr<QueryRunner> runner, KademliaConfig config)
      : network_(network),
        repo_(std::move(repo)),
        table_(std::move(table)),
        mrw_(std::move(mrw)),
        runner_(std::move(runner)),
        config_(config) {
    BOOST_ASSERT(repo_ != nullptr);
    BOOST_ASSERT(table_ != nullptr);
    BOOST_ASSERT(mrw_ != nullptr);
    BOOST_ASSERT(runner_ != nullptr);
  }
}  // namespace libp2p::protocol::kademlia
