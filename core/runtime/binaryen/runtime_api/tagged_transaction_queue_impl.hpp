/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef KAGOME_RUNTIME_BINARYEN_TAGGED_TRANSACTION_QUEUE_IMPL_HPP
#define KAGOME_RUNTIME_BINARYEN_TAGGED_TRANSACTION_QUEUE_IMPL_HPP

#include "runtime/binaryen/runtime_api/runtime_api.hpp"
#include "runtime/tagged_transaction_queue.hpp"

#include "blockchain/block_header_repository.hpp"
#include "blockchain/block_tree.hpp"

namespace kagome::runtime::binaryen {

  class TaggedTransactionQueueImpl : public RuntimeApi,
                                     public TaggedTransactionQueue {
   public:
    TaggedTransactionQueueImpl(
        const std::shared_ptr<RuntimeEnvironmentFactory> &runtime_env_factory,
        std::shared_ptr<const blockchain::BlockTree> block_tree);

    ~TaggedTransactionQueueImpl() override = default;

    outcome::result<primitives::TransactionValidity> validate_transaction(
        primitives::TransactionSource source,
        const primitives::Extrinsic &ext) override;

   private:
    std::shared_ptr<const blockchain::BlockTree> block_tree_;
  };
}  // namespace kagome::runtime::binaryen

#endif  // KAGOME_RUNTIME_BINARYEN_TAGGED_TRANSACTION_QUEUE_IMPL_HPP
