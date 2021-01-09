/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef KAGOME_CORE_RUNTIME_WAVM_CORE_HPP
#define KAGOME_CORE_RUNTIME_WAVM_CORE_HPP

#include "runtime/core.hpp"

#include "WAVM/IR/Module.h"
#include "WAVM/IR/Types.h"
#include "WAVM/IR/Value.h"
#include "WAVM/Runtime/Intrinsics.h"
#include "WAVM/Runtime/Runtime.h"
#include "WAVM/WASM/WASM.h"
#include "WAVM/WASTParse/WASTParse.h"

#include <WAVM/IR/Validate.h>
#include <WAVM/Inline/Serialization.h>
#include <boost/system/error_code.hpp>
#include "extensions/extension_factory.hpp"
#include "runtime/common/storage_wasm_provider.hpp"
#include "runtime/trie_storage_provider.hpp"
#include "runtime/wasm_result.hpp"
#include "runtime/wavm/wasm_memory_impl.hpp"
#include "scale/scale.hpp"

namespace kagome::runtime::wavm {

  class CoreWavm : public Core {
   public:
    ~CoreWavm() override = default;

    CoreWavm(std::shared_ptr<WasmProvider> wasm_provider,
             std::shared_ptr<extensions::ExtensionFactory> extenstion_factory,
             std::shared_ptr<TrieStorageProvider> trie_storage_provider);

    WAVM::Runtime::ModuleRef parseModule(const common::Buffer &code);

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
        const primitives::BlockId &block_id);

   private:
    std::shared_ptr<WasmProvider> wasm_provider_;
    std::shared_ptr<extensions::ExtensionFactory> extenstion_factory_;
    std::shared_ptr<TrieStorageProvider> trie_storage_provider_;
  };

}  // namespace kagome::runtime::wavm

#endif  // KAGOME_CORE_RUNTIME_WAVM_CORE_HPP
