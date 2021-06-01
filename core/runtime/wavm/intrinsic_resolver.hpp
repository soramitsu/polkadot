/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef KAGOME_CORE_RUNTIME_WAVM_INTRINSIC_RESOLVER_HPP
#define KAGOME_CORE_RUNTIME_WAVM_INTRINSIC_RESOLVER_HPP

#include <WAVM/Runtime/Linker.h>

#include <memory>
#include <string>

#include "runtime/wasm_memory.hpp"

namespace kagome::runtime::wavm {

  class IntrinsicResolver : public WAVM::Runtime::Resolver {
   public:
    virtual bool resolve(const std::string &moduleName,
                         const std::string &exportName,
                         WAVM::IR::ExternType type,
                         WAVM::Runtime::Object *&outObject) = 0;

    virtual std::shared_ptr<runtime::Memory> getMemory() const = 0;

    virtual std::unique_ptr<IntrinsicResolver> clone() const = 0;
  };
}

#endif  // KAGOME_CORE_RUNTIME_WAVM_INTRINSIC_RESOLVER_HPP