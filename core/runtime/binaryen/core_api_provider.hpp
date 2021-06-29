/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef KAGOME_CORE_RUNTIME_BINARYEN_CORE_API_PROVIDER_HPP
#define KAGOME_CORE_RUNTIME_BINARYEN_CORE_API_PROVIDER_HPP

#include "runtime/core.hpp"
#include "runtime/core_api_provider.hpp"

namespace kagome::storage::changes_trie {
  class ChangesTracker;
}

namespace kagome::blockchain {
  class BlockHeaderRepository;
}

namespace kagome::runtime {
  class TrieStorageProvider;
  class Memory;
}  // namespace kagome::runtime

namespace kagome::runtime::binaryen {

  class RuntimeEnvironmentFactory;

  class BinaryenCoreApiProvider final : public runtime::CoreApiProvider {
   public:
    BinaryenCoreApiProvider(
        std::shared_ptr<storage::changes_trie::ChangesTracker> changes_tracker,
        std::shared_ptr<blockchain::BlockHeaderRepository> header_repo);

    // to avoid circular dependency
    void setRuntimeFactory(std::shared_ptr<RuntimeEnvironmentFactory> runtime_env_factory) {
      runtime_env_factory_ = std::move(runtime_env_factory);
    }

    std::unique_ptr<Core> makeCoreApi(
        std::shared_ptr<const crypto::Hasher> hasher,
        gsl::span<uint8_t> runtime_code) const override;

   private:
    std::shared_ptr<RuntimeEnvironmentFactory> runtime_env_factory_;
    std::shared_ptr<storage::changes_trie::ChangesTracker> changes_tracker_;
    std::shared_ptr<blockchain::BlockHeaderRepository> header_repo_;
  };

}  // namespace kagome::runtime::binaryen

#endif  // KAGOME_CORE_RUNTIME_BINARYEN_CORE_API_PROVIDER_HPP