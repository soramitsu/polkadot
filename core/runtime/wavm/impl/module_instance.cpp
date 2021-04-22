/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "module_instance.hpp"

#include <WAVM/Runtime/Runtime.h>

namespace kagome::runtime::wavm {

  ModuleInstance::ModuleInstance(WAVM::Runtime::Instance *instance,
                                 WAVM::Runtime::Compartment *compartment)
      : instance_{instance}, compartment_{std::move(compartment)} {}

  WasmResult ModuleInstance::callExportFunction(std::string_view name,
                                                WasmResult args) {
    WAVM::Runtime::Context *context =
        WAVM::Runtime::createContext(compartment_);
    WAVM::Runtime::Function *function = WAVM::Runtime::asFunctionNullable(
        WAVM::Runtime::getInstanceExport(instance_, name.data()));
    std::vector<WAVM::IR::Value> invokeArgs;
    if (!function) {
      // log error
      // return EXIT_FAILURE;
    }
    const WAVM::IR::FunctionType functionType =
        WAVM::Runtime::getFunctionType(function);
    if (functionType.params().size() != 2) {  // address and size
      // log error
      // return EXIT_FAILURE;
    }
    invokeArgs.emplace_back(static_cast<WAVM::U32>(args.address));
    invokeArgs.emplace_back(static_cast<WAVM::U32>(args.length));
    WAVM_ASSERT(function);

    std::vector<WAVM::IR::ValueType> invokeArgTypes;
    std::vector<WAVM::IR::UntaggedValue> untaggedInvokeArgs;
    for (const WAVM::IR::Value &arg : invokeArgs) {
      invokeArgTypes.push_back(arg.type);
      untaggedInvokeArgs.push_back(arg);
    }
    // Infer the expected type of the function from the number and type of the
    // invoke's arguments and the function's actual result types.
    const WAVM::IR::FunctionType invokeSig(
        WAVM::Runtime::getFunctionType(function).results(),
        WAVM::IR::TypeTuple(invokeArgTypes));
    // Allocate an array to receive the invoke results.
    std::vector<WAVM::IR::UntaggedValue> untaggedInvokeResults;
    untaggedInvokeResults.resize(invokeSig.results().size());
    WAVM::Runtime::invokeFunction(context,
                                  function,
                                  invokeSig,
                                  untaggedInvokeArgs.data(),
                                  untaggedInvokeResults.data());
    return WasmResult{untaggedInvokeResults[0].u32,
                      untaggedInvokeResults[1].u32};
  }

}  // namespace kagome::runtime::wavm