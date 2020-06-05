/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef KAGOME_STATE_API_IMPL_HPP
#define KAGOME_STATE_API_IMPL_HPP

#include "api/service/state/state_api.hpp"
#include "api/service/state/readonly_trie_builder.hpp"
#include "blockchain/block_header_repository.hpp"
#include "blockchain/block_tree.hpp"
#include "storage/trie/trie_storage.hpp"

namespace kagome::api {

  class StateApiImpl : public StateApi {
   public:
    StateApiImpl(std::shared_ptr<blockchain::BlockHeaderRepository> block_repo,
                 std::shared_ptr<const storage::trie::TrieStorage> trie_storage,
                 std::shared_ptr<blockchain::BlockTree> block_tree);

    outcome::result<common::Buffer> getStorage(
        const common::Buffer &key) const override;
    outcome::result<common::Buffer> getStorage(
        const common::Buffer &key, const primitives::BlockHash &at) const override;

   private:
    std::shared_ptr<blockchain::BlockHeaderRepository> block_repo_;
    std::shared_ptr<const storage::trie::TrieStorage> storage_;
    std::shared_ptr<blockchain::BlockTree> block_tree_;
  };

}  // namespace kagome::api

#endif  // KAGOME_STATE_API_IMPL_HPP
