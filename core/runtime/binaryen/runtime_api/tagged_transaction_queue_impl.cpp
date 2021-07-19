/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "runtime/binaryen/runtime_api/tagged_transaction_queue_impl.hpp"

namespace kagome::runtime::binaryen {
  using primitives::TransactionValidity;

  TaggedTransactionQueueImpl::TaggedTransactionQueueImpl(
      const std::shared_ptr<RuntimeEnvironmentFactory> &runtime_env_factory,
      std::shared_ptr<const blockchain::BlockTree> block_tree)
      : RuntimeApi(runtime_env_factory), block_tree_{block_tree} {
    BOOST_ASSERT(block_tree_);
  }

  outcome::result<primitives::TransactionValidity>
  TaggedTransactionQueueImpl::validate_transaction(
      primitives::TransactionSource source, const primitives::Extrinsic &ext) {
    OUTCOME_TRY(best_header,
                block_tree_->getBlockHeader(block_tree_->deepestLeaf().hash));
    return executeAt<TransactionValidity>(
        "TaggedTransactionQueue_validate_transaction",
        best_header.state_root,
        CallConfig{.persistency = CallPersistency::EPHEMERAL},
        source,
        ext);
  }
}  // namespace kagome::runtime::binaryen
