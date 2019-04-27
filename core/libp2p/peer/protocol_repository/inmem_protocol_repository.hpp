/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef KAGOME_INMEM_PROTOCOL_REPOSITORY_HPP
#define KAGOME_INMEM_PROTOCOL_REPOSITORY_HPP

#include <set>
#include <unordered_map>

#include "libp2p/peer/protocol_repository.hpp"

namespace libp2p::peer {

  class InmemProtocolRepository : public ProtocolRepository {
   public:
    ~InmemProtocolRepository() override = default;

    outcome::result<void> addProtocols(const PeerId &p,
                                       gsl::span<const Protocol> ms) override;

    outcome::result<void> removeProtocols(
        const PeerId &p, gsl::span<const Protocol> ms) override;

    outcome::result<std::list<Protocol>> getProtocols(
        const PeerId &p) const override;

    outcome::result<std::list<Protocol>> supportsProtocols(
        const PeerId &p, gsl::span<const Protocol> protocols) const override;

    void clear(const PeerId &p) override;

    void collectGarbage() override;

   private:
    using set = std::set<Protocol>;
    using set_ptr = std::shared_ptr<set>;

    outcome::result<set_ptr> get_set(const PeerId &p) const;
    set_ptr must_get_set(const PeerId &p);

    std::unordered_map<PeerId, set_ptr> db_;
  };

}  // namespace libp2p::peer

#endif  // KAGOME_INMEM_PROTOCOL_REPOSITORY_HPP
