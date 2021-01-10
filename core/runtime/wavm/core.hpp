/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef KAGOME_CORE_RUNTIME_WAVM_CORE_HPP
#define KAGOME_CORE_RUNTIME_WAVM_CORE_HPP

#include "runtime/core.hpp"

#include <boost/system/error_code.hpp>
#include "runtime/wavm/wavm_runtime_api.hpp"

#include "blockchain/block_header_repository.hpp"
#include "extensions/extension_factory.hpp"
#include "runtime/common/storage_wasm_provider.hpp"
#include "runtime/trie_storage_provider.hpp"
#include "scale/scale.hpp"
#include "storage/changes_trie/changes_tracker.hpp"

namespace kagome::runtime::wavm {

  class CoreWavm : public WavmRuntimeApi, public Core {
   public:
    ~CoreWavm() override = default;

    CoreWavm(
        const std::shared_ptr<WasmProvider>& wasm_provider,
        std::shared_ptr<extensions::ExtensionFactory> extension_factory,
        std::shared_ptr<TrieStorageProvider> trie_storage_provider,
        std::shared_ptr<storage::changes_trie::ChangesTracker> changes_tracker,
        std::shared_ptr<blockchain::BlockHeaderRepository> header_repo);

    outcome::result<primitives::Version> version(
        const boost::optional<primitives::BlockHash> &block_hash) override;

    /**
     * @brief Executes the given block
     * @param block block to execute
     */
    outcome::result<void> execute_block(
        const primitives::Block &block) override;

    /**
     * @brief Initialize a block with the given header.
     * @param header header used for block initialization
     */
    outcome::result<void> initialise_block(
        const primitives::BlockHeader &header) override;

    /**
     * Get current authorities
     * @return collection of authorities
     */
    outcome::result<std::vector<primitives::AuthorityId>> authorities(
        const primitives::BlockId &block_id) override;

   private:
    std::shared_ptr<WasmProvider> wasm_provider_;
    std::shared_ptr<storage::changes_trie::ChangesTracker> changes_tracker_;
    std::shared_ptr<blockchain::BlockHeaderRepository> header_repo_;
  };

}  // namespace kagome::runtime::wavm

#endif  // KAGOME_CORE_RUNTIME_WAVM_CORE_HPP
