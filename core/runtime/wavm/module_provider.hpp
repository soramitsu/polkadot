/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef KAGOME_CORE_RUNTIME_WAVM_MODULE_PROVIDER_HPP
#define KAGOME_CORE_RUNTIME_WAVM_MODULE_PROVIDER_HPP

#include "runtime/wasm_provider.hpp"
#include "extensions/extension_factory.hpp"
#include "runtime/trie_storage_provider.hpp"
#include "common/logger.hpp"

namespace kagome::runtime::wavm {

  enum class CallPersistency {
    PERSISTENT,  // the changes made by this call will be applied to the state
    // trie storage
    EPHEMERAL  // the changes made by this call will vanish once it's
    // completed
  };

  class ModuleProvider {
   public:
    ModuleProvider(
        std::shared_ptr<WasmProvider> wasm_provider,
        std::shared_ptr<extensions::ExtensionFactory> extension_factory,
        std::shared_ptr<TrieStorageProvider> trie_storage_provider);

//    WAVM::Runtime::ModuleRef parseModule(const common::Buffer &code);
//
//    WAVM::Runtime::ImportBindings getImports(
//        WAVM::Runtime::Instance *intrinsicsInstance);

    outcome::result<std::vector<uint8_t>> execute(
        std::string_view name,
        const boost::optional<common::Hash256> &state_root,
        CallPersistency persistency,
        const std::vector<uint8_t> &scale_args);

   private:
    std::shared_ptr<WasmProvider> wasm_provider_;
    std::shared_ptr<extensions::ExtensionFactory> extension_factory_;
    std::shared_ptr<TrieStorageProvider> trie_storage_provider_;
    common::Logger logger_;
  };

}  // namespace kagome::runtime::wavm

#endif  // KAGOME_CORE_RUNTIME_WAVM_MODULE_PROVIDER_HPP
