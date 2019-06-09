/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef KAGOME_CORE_RUNTIME_IMPL_OFFCHAIN_WORKER_IMPL_HPP
#define KAGOME_CORE_RUNTIME_IMPL_OFFCHAIN_WORKER_IMPL_HPP

#include "extensions/extension.hpp"
#include "runtime/offchain_worker.hpp"

namespace kagome::runtime {
  class RuntimeApi;

  class OffchainWorkerImpl : public OffchainWorker {
   public:
    OffchainWorkerImpl(common::Buffer state_code,
                       std::shared_ptr<extensions::Extension> extension);

    ~OffchainWorkerImpl() override;

    outcome::result<void> offchain_worker(BlockNumber bn) override;

   private:
    std::unique_ptr<RuntimeApi> runtime_;
  };
}  // namespace kagome::runtime

#endif  // KAGOME_CORE_RUNTIME_IMPL_OFFCHAIN_WORKER_IMPL_HPP
