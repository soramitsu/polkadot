/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef KAGOME_CORE_EXTRINSICS_SUBMISSION_SERVICE_EXTRINSIC_SUBMISSION_API_IMPL_HPP
#define KAGOME_CORE_EXTRINSICS_SUBMISSION_SERVICE_EXTRINSIC_SUBMISSION_API_IMPL_HPP

/**
 * ExtrinsicSubmissionApi based on auth api implemented in substrate here
 * https://github.com/paritytech/substrate/blob/e8739300ae3f7f2e7b72f64668573275f2806ea5/core/rpc/src/author/mod.rs#L50-L49
 */

#include <boost/variant.hpp>
#include <outcome/outcome.hpp>
#include "common/visitor.hpp"
#include "crypto/hasher.hpp"
#include "extrinsics_submission_service/error.hpp"
#include "extrinsics_submission_service/extrinsic_submission_api.hpp"

namespace kagome::transaction_pool {
  class TransactionPool;
}

namespace kagome::runtime {
  class TaggedTransactionQueue;
}

namespace kagome::service {
  class ExtrinsicSubmissionApiImpl : public ExtrinsicSubmissionApi {
    template <class T>
    using sptr = std::shared_ptr<T>;

   public:
    /**
     * @constructor
     * @param api ttq instance shared ptr
     * @param pool transaction pool instance shared ptr
     * @param hasher hasher instance shared ptr
     */
    ExtrinsicSubmissionApiImpl(
        std::shared_ptr<runtime::TaggedTransactionQueue> api,
        std::shared_ptr<transaction_pool::TransactionPool> pool,
        std::shared_ptr<hash::Hasher> hasher);

    ~ExtrinsicSubmissionApiImpl() override = default;

    outcome::result<common::Hash256> submit_extrinsic(
        const primitives::Extrinsic &extrinsic) override;

    outcome::result<std::vector<primitives::Extrinsic>> pending_extrinsics()
        override;

    // TODO(yuraz): probably will be documented later (no task yet)
    outcome::result<std::vector<common::Hash256>> remove_extrinsic(
        const std::vector<primitives::ExtrinsicKey>
            &bytes_or_hash) override;

    // TODO(yuraz): probably will be documented later (no task yet)
    void watch_extrinsic(const primitives::Metadata &metadata,
                         const primitives::Subscriber &subscriber,
                         const common::Buffer &data) override;

    // TODO(yuraz): probably will be documented later (no task yet)
    outcome::result<bool> unwatch_extrinsic(
        const std::optional<primitives::Metadata> &metadata,
        const primitives::SubscriptionId &id) override;

   private:
    sptr<runtime::TaggedTransactionQueue> api_;  ///< pointer to ttq api
    sptr<transaction_pool::TransactionPool>
        pool_;                   ///< pointer to transaction pool apo
    sptr<hash::Hasher> hasher_;  ///< pointer to hasher
  };
}  // namespace kagome::service

#endif  // KAGOME_CORE_EXTRINSICS_SUBMISSION_SERVICE_EXTRINSIC_SUBMISSION_API_IMPL_HPP
