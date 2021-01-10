/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "runtime/wavm/wavm_runtime_api.hpp"

namespace kagome::runtime::wavm {

  WavmRuntimeApi::WavmRuntimeApi(
      std::shared_ptr<WasmProvider> wasm_provider,
      std::shared_ptr<extensions::ExtensionFactory> extension_factory,
      std::shared_ptr<TrieStorageProvider> trie_storage_provider)
      : module_provider_(std::make_shared<ModuleProvider>(
          wasm_provider, extension_factory, trie_storage_provider)) {}

}  // namespace kagome::runtime::wavm
