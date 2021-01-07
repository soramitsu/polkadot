/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef KAGOME_CORE_RUNTIME_WAVM_CORE_HPP
#define KAGOME_CORE_RUNTIME_WAVM_CORE_HPP

#include "runtime/core.hpp"

#include "WAVM/IR/Module.h"
#include "WAVM/IR/Types.h"
#include "WAVM/IR/Value.h"
#include "WAVM/Runtime/Intrinsics.h"
#include "WAVM/Runtime/Runtime.h"
#include "WAVM/WASTParse/WASTParse.h"
#include "WAVM/WASM/WASM.h"

#include <WAVM/IR/Validate.h>
#include <WAVM/Inline/Serialization.h>
#include <boost/system/error_code.hpp>
#include "runtime/common/storage_wasm_provider.hpp"

namespace kagome::runtime::wavm {

  class CoreWavm : public Core {
   public:
    ~CoreWavm() override = default;

    CoreWavm(std::shared_ptr<WasmProvider> wasm_provider)
        : wasm_provider_{wasm_provider} {}

    WAVM::IR::Module parseModule(const common::Buffer& code) {
      using namespace WAVM;
      using namespace WAVM::IR;
      using namespace WAVM::Runtime;

      // first parse module
      IR::Module moduleIR;

      bool a = WASM::loadBinaryModule(code.toVector().data(), code.size(), moduleIR);

      return moduleIR;
    }

    outcome::result<primitives::Version> version(
        const boost::optional<primitives::BlockHash> &block_hash) {
      using namespace WAVM;
      using namespace WAVM::IR;
      using namespace WAVM::Runtime;

      IR::Module irModule = parseModule(wasm_provider_->getStateCode());
      std::vector<WAST::Error> wastErrors;

      ModuleRef module = compileModule(irModule);

      module->
    }

   private:
    std::shared_ptr<WasmProvider> wasm_provider_;
  };

}  // namespace kagome::runtime::wavm

#endif  // KAGOME_CORE_RUNTIME_WAVM_CORE_HPP
