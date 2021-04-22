/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef KAGOME_CORE_RUNTIME_WAVM_IMPL_MODULE_HPP
#define KAGOME_CORE_RUNTIME_WAVM_IMPL_MODULE_HPP

#include <memory>

#include <WAVM/Runtime/Runtime.h>

#include "log/logger.hpp"

namespace WAVM::Runtime {
  struct Compartment;
}

namespace kagome::runtime::wavm {

  class ModuleInstance;
  class IntrinsicResolver;

  class Module final {
   public:
    Module(WAVM::Runtime::Compartment* compartment,
           std::shared_ptr<WAVM::Runtime::Module> module);

    std::unique_ptr<ModuleInstance> instantiate(IntrinsicResolver &resolver);

   private:
    WAVM::Runtime::ImportBindings link(IntrinsicResolver &resolver);

    WAVM::Runtime::Compartment* compartment_;
    std::shared_ptr<WAVM::Runtime::Module> module_;
    log::Logger logger_;
  };

}  // namespace kagome::runtime::wavm

#endif  // KAGOME_CORE_RUNTIME_WAVM_IMPL_MODULE_HPP