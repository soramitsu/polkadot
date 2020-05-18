/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "storage/trie/impl/trie_storage_impl.hpp"

#include <memory>

#include "outcome/outcome.hpp"
#include "storage/trie/impl/ephemeral_trie_batch_impl.hpp"
#include "storage/trie/impl/persistent_trie_batch_impl.hpp"

namespace kagome::storage::trie {

  outcome::result<std::unique_ptr<TrieStorageImpl>>
  TrieStorageImpl::createEmpty(
      std::shared_ptr<PolkadotTrieFactory> trie_factory,
      std::shared_ptr<Codec> codec,
      std::shared_ptr<TrieSerializer> serializer,
      boost::optional<std::shared_ptr<changes_trie::ChangesTracker>> changes) {
    // will never be used, so content of the callback doesn't matter
    auto empty_trie = trie_factory->createEmpty(
        [](auto branch, auto idx) { return nullptr; });
    // ensure retrieval of empty trie succeeds
    OUTCOME_TRY(empty_root, serializer->storeTrie(*empty_trie));
    return std::unique_ptr<TrieStorageImpl>(
        new TrieStorageImpl(std::move(empty_root),
                            std::move(codec),
                            std::move(serializer),
                            std::move(changes)));
  }

  outcome::result<std::unique_ptr<TrieStorageImpl>>
  TrieStorageImpl::createFromStorage(
      const common::Buffer &root_hash,
      std::shared_ptr<Codec> codec,
      std::shared_ptr<TrieSerializer> serializer,
      boost::optional<std::shared_ptr<changes_trie::ChangesTracker>> changes) {
    return std::unique_ptr<TrieStorageImpl>(
        new TrieStorageImpl(root_hash,
                            std::move(codec),
                            std::move(serializer),
                            std::move(changes)));
  }

  TrieStorageImpl::TrieStorageImpl(
      common::Buffer root_hash,
      std::shared_ptr<Codec> codec,
      std::shared_ptr<TrieSerializer> serializer,
      boost::optional<std::shared_ptr<changes_trie::ChangesTracker>> changes)
      : root_hash_{std::move(root_hash)},
        codec_{std::move(codec)},
        serializer_{std::move(serializer)},
        changes_{std::move(changes)} {
    BOOST_ASSERT(codec_ != nullptr);
    BOOST_ASSERT(serializer_ != nullptr);
    BOOST_ASSERT((changes_.has_value() and changes_.value() != nullptr)
                 or not changes_.has_value());
  }

  outcome::result<std::unique_ptr<PersistentTrieBatch>>
  TrieStorageImpl::getPersistentBatch() {
    OUTCOME_TRY(trie, serializer_->retrieveTrie(Buffer{root_hash_}));
    return std::make_unique<PersistentTrieBatchImpl>(
        codec_,
        serializer_,
        changes_,
        std::move(trie),
        [this](auto const &new_root) { root_hash_ = new_root; });
  }

  outcome::result<std::unique_ptr<EphemeralTrieBatch>>
  TrieStorageImpl::getEphemeralBatch() const {
    OUTCOME_TRY(trie, serializer_->retrieveTrie(Buffer{root_hash_}));
    return std::make_unique<EphemeralTrieBatchImpl>(codec_, std::move(trie));
  }

  outcome::result<std::unique_ptr<PersistentTrieBatch>>
  TrieStorageImpl::getPersistentBatchAt(const common::Hash256 &root) {
    OUTCOME_TRY(trie, serializer_->retrieveTrie(Buffer{root}));
    return std::make_unique<PersistentTrieBatchImpl>(
        codec_,
        serializer_,
        changes_,
        std::move(trie),
        [this](auto const &new_root) { root_hash_ = new_root; });
  }

  outcome::result<std::unique_ptr<EphemeralTrieBatch>>
  TrieStorageImpl::getEphemeralBatchAt(const common::Hash256 &root) const {
    OUTCOME_TRY(trie, serializer_->retrieveTrie(Buffer{root}));
    return std::make_unique<EphemeralTrieBatchImpl>(codec_, std::move(trie));
  }

  common::Buffer TrieStorageImpl::getRootHash() const {
    return root_hash_;
  }
}  // namespace kagome::storage::trie
