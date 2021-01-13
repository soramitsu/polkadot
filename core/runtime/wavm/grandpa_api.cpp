/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "runtime/wavm/grandpa_api.hpp"

namespace kagome::runtime::wavm {

  GrandpaApiWavm::GrandpaApiWavm(
      const std::shared_ptr<WasmProvider>& wasm_provider,
      std::shared_ptr<extensions::ExtensionFactory> extension_factory,
      std::shared_ptr<TrieStorageProvider> trie_storage_provider,
      std::shared_ptr<blockchain::BlockHeaderRepository> header_repo)
      : WavmRuntimeApi(wasm_provider,
                       std::move(extension_factory),
                       std::move(trie_storage_provider)),
        header_repo_{std::move(header_repo)} {
    BOOST_ASSERT(header_repo_ != nullptr);
  }

  using common::Buffer;
  using primitives::Authority;
  using primitives::Digest;
  using primitives::ForcedChange;
  using primitives::ScheduledChange;
  using primitives::GrandpaSessionKey;

  outcome::result<boost::optional<ScheduledChange>>
  GrandpaApiWavm::pending_change(const Digest &digest) {
    return execute<boost::optional<ScheduledChange>>(
        "GrandpaApi_grandpa_pending_change",
        CallPersistency::EPHEMERAL,
        digest);
  }

  outcome::result<boost::optional<ForcedChange>> GrandpaApiWavm::forced_change(
      const Digest &digest) {
    return execute<boost::optional<ForcedChange>>(
        "GrandpaApi_grandpa_forced_change", CallPersistency::EPHEMERAL, digest);
  }

  outcome::result<primitives::AuthorityList> GrandpaApiWavm::authorities(
      const primitives::BlockId &block_id) {
    OUTCOME_TRY(header, header_repo_->getBlockHeader(block_id));
    return executeAt<primitives::AuthorityList>(
        "GrandpaApi_grandpa_authorities",
        header.state_root,
        CallPersistency::EPHEMERAL);
  }

}
