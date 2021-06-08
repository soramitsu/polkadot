/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "storage/in_memory/in_memory_storage.hpp"

#include "storage/database_error.hpp"
#include "storage/in_memory/in_memory_batch.hpp"

using kagome::common::Buffer;

constexpr const char *kInMemoryDbSizeGaugeName = "kagome_in_memory_db_size";

namespace kagome::storage {

  InMemoryStorage::InMemoryStorage() {
    registry_->registerGaugeFamily(kInMemoryDbSizeGaugeName,
                                   "In-memory database size");
    in_memory_db_size_ = registry_->registerGaugeMetric(
        kInMemoryDbSizeGaugeName, {{"category", "memory"}, {"units", "Bytes"}});
    in_memory_db_index_size_ = registry_->registerGaugeMetric(
        kInMemoryDbSizeGaugeName, {{"category", "index"}, {"units", ""}});
  }

  outcome::result<Buffer> InMemoryStorage::get(const Buffer &key) const {
    if (storage.find(key.toHex()) != storage.end()) {
      return storage.at(key.toHex());
    }

    return DatabaseError::NOT_FOUND;
  }

  outcome::result<void> InMemoryStorage::put(const Buffer &key,
                                             const Buffer &value) {
    if (storage.find(key.toHex()) == storage.end()) {
      in_memory_db_index_size_->inc();
      in_memory_db_size_->inc(key.toHex().size() + value.size());
    } else {
      in_memory_db_size_->inc(value.size());
    }
    storage[key.toHex()] = value;
    return outcome::success();
  }

  outcome::result<void> InMemoryStorage::put(const Buffer &key,
                                             Buffer &&value) {
    if (storage.find(key.toHex()) == storage.end()) {
      in_memory_db_index_size_->inc();
      in_memory_db_size_->inc(key.toHex().size() + value.size());
    } else {
      in_memory_db_size_->inc(value.size());
    }
    storage[key.toHex()] = std::move(value);
    return outcome::success();
  }

  bool InMemoryStorage::contains(const Buffer &key) const {
    return storage.find(key.toHex()) != storage.end();
  }

  bool InMemoryStorage::empty() const {
    return storage.empty();
  }

  outcome::result<void> InMemoryStorage::remove(const Buffer &key) {
    if (storage.find(key.toHex()) != storage.end()) {
      in_memory_db_index_size_->dec();
      in_memory_db_size_->dec(key.toHex().size() + storage[key.toHex()].size());
    }
    storage.erase(key.toHex());
    return outcome::success();
  }

  std::unique_ptr<kagome::storage::face::WriteBatch<Buffer, Buffer>>
  InMemoryStorage::batch() {
    return std::make_unique<InMemoryBatch>(*this);
  }

  std::unique_ptr<kagome::storage::face::MapCursor<Buffer, Buffer>>
  InMemoryStorage::cursor() {
    return nullptr;
  }
}  // namespace kagome::storage
