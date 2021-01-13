/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef KAGOME_CORE_RUNTIME_WAVM_GRANDPA_API_HPP
#define KAGOME_CORE_RUNTIME_WAVM_GRANDPA_API_HPP

#include "runtime/grandpa_api.hpp"

#include <boost/system/error_code.hpp>
#include "runtime/wavm/wavm_runtime_api.hpp"

#include "blockchain/block_header_repository.hpp"
#include "extensions/extension_factory.hpp"
#include "runtime/common/storage_wasm_provider.hpp"
#include "runtime/trie_storage_provider.hpp"
#include "scale/scale.hpp"
#include "storage/changes_trie/changes_tracker.hpp"

namespace kagome::runtime::wavm {

  class GrandpaApiWavm : public WavmRuntimeApi, public GrandpaApi {
   public:
    GrandpaApiWavm(
        const std::shared_ptr<WasmProvider>& wasm_provider,
        std::shared_ptr<extensions::ExtensionFactory> extension_factory,
        std::shared_ptr<TrieStorageProvider> trie_storage_provider,
        std::shared_ptr<blockchain::BlockHeaderRepository> header_repo);

    ~GrandpaApiWavm() override = default;

    outcome::result<boost::optional<ScheduledChange>> pending_change(
        const Digest &digest) override;

    outcome::result<boost::optional<ForcedChange>> forced_change(
        const Digest &digest) override;

    outcome::result<primitives::AuthorityList> authorities(
        const primitives::BlockId &block_id) override;

   private:
    std::shared_ptr<blockchain::BlockHeaderRepository> header_repo_;
  };

}

#endif  // KAGOME_CORE_RUNTIME_WAVM_GRANDPA_API_HPP
