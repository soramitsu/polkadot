/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "runtime/wavm/module_provider.hpp"

#include "WAVM/IR/IR.h"
#include "WAVM/IR/Module.h"
#include "WAVM/IR/Types.h"
#include "WAVM/IR/Value.h"
#include "WAVM/Inline/Assert.h"
#include "WAVM/Inline/BasicTypes.h"
#include "WAVM/Inline/Hash.h"
#include "WAVM/Inline/HashMap.h"
#include "WAVM/Inline/Timing.h"
#include "WAVM/LLVMJIT/LLVMJIT.h"
#include "WAVM/Runtime/Runtime.h"
#include "WAVM/Platform/Intrinsic.h"
#include "WAVM/Platform/RWMutex.h"
#include "WAVM/WASM/WASM.h"

#include "runtime/wavm/wasm_memory_impl.hpp"
#include "runtime/wavm/wavm_external_interface.hpp"
//#include "crypto/hasher/hasher_impl.hpp"

namespace kagome::runtime::wavm {

  static boost::optional<WAVM::Uptr> latest_code_hash_;
  static boost::optional<WAVM::Runtime::ModuleRef> latest_precompiled_module_;

  ModuleProvider::ModuleProvider(
      std::shared_ptr<WasmProvider> wasm_provider,
      std::shared_ptr<extensions::ExtensionFactory> extension_factory,
      std::shared_ptr<TrieStorageProvider> trie_storage_provider)
      : wasm_provider_{wasm_provider},
        extension_factory_{extension_factory},
        trie_storage_provider_{trie_storage_provider} {}

  WAVM::Runtime::ModuleRef parseModule(
      const common::Buffer &code) {
    using namespace WAVM;
    using namespace WAVM::IR;
    using namespace WAVM::Runtime;

    auto code_hash = Hash<std::vector<uint8_t>>{}(code.toVector());
//    auto code_hash = hasher_->twox_64(code.toVector());
    if (not latest_precompiled_module_ or latest_code_hash_.value() != code_hash){
      // first parse module
      IR::Module moduleIR;

      bool a =
          WASM::loadBinaryModule(code.toVector().data(), code.size(), moduleIR);

      latest_precompiled_module_ = compileModule(moduleIR);
      latest_code_hash_ = code_hash;
    }

    return *latest_precompiled_module_;
  }

  WAVM::Runtime::ImportBindings getImports(
      WAVM::Runtime::Instance *intrinsicsInstance) {
    using namespace WAVM;
    using namespace WAVM::IR;
    using namespace WAVM::Runtime;

    using vt = ValueType;
    // clang-format off
    return {
        asObject(getTypedInstanceExport(intrinsicsInstance, "memory", MemoryType(false, IndexType::i32, SizeConstraints{20, UINT64_MAX}))),
        asObject(getTypedInstanceExport(intrinsicsInstance, "ext_logging_log_version_1", {TypeTuple{}, {vt::i32, vt::i64, vt::i64}})),
        asObject(getTypedInstanceExport(intrinsicsInstance, "ext_sandbox_instance_teardown_version_1", {TypeTuple{}, {vt::i32}})),
        asObject(getTypedInstanceExport(intrinsicsInstance, "ext_sandbox_instantiate_version_1", {{vt::i32}, {vt::i32, vt::i64, vt::i64, vt::i32}})),
        asObject(getTypedInstanceExport(intrinsicsInstance, "ext_sandbox_invoke_version_1", {{vt::i32}, {vt::i32, vt::i64, vt::i64, vt::i32, vt::i32, vt::i32}})),
        asObject(getTypedInstanceExport(intrinsicsInstance, "ext_sandbox_memory_get_version_1", {{vt::i32}, {vt::i32, vt::i32, vt::i32, vt::i32}})),
        asObject(getTypedInstanceExport(intrinsicsInstance, "ext_sandbox_memory_new_version_1", {{vt::i32}, {vt::i32, vt::i32}})),
        asObject(getTypedInstanceExport(intrinsicsInstance, "ext_sandbox_memory_set_version_1", {{vt::i32}, {vt::i32, vt::i32, vt::i32, vt::i32}})),
        asObject(getTypedInstanceExport(intrinsicsInstance, "ext_sandbox_memory_teardown_version_1", {TypeTuple{}, {vt::i32}})),
        asObject(getTypedInstanceExport(intrinsicsInstance, "ext_crypto_ed25519_generate_version_1", {{vt::i32}, {vt::i32, vt::i64}})),
        asObject(getTypedInstanceExport(intrinsicsInstance, "ext_crypto_ed25519_verify_version_1", {{vt::i32}, {vt::i32, vt::i64, vt::i32}})),
        asObject(getTypedInstanceExport(intrinsicsInstance, "ext_crypto_finish_batch_verify_version_1", {{vt::i32}, TypeTuple{}})),
        asObject(getTypedInstanceExport(intrinsicsInstance, "ext_crypto_secp256k1_ecdsa_recover_compressed_version_1", {{vt::i64}, {vt::i32, vt::i32}})),
        asObject(getTypedInstanceExport(intrinsicsInstance, "ext_crypto_sr25519_generate_version_1", {{vt::i32}, {vt::i32, vt::i64}})),
        asObject(getTypedInstanceExport(intrinsicsInstance, "ext_crypto_sr25519_public_keys_version_1", {{vt::i64}, {vt::i32}})),
        asObject(getTypedInstanceExport(intrinsicsInstance, "ext_crypto_sr25519_sign_version_1", {{vt::i64}, {vt::i32, vt::i32, vt::i64}})),
        asObject(getTypedInstanceExport(intrinsicsInstance, "ext_crypto_sr25519_verify_version_2", {{vt::i32}, {vt::i32, vt::i64, vt::i32}})),
        asObject(getTypedInstanceExport(intrinsicsInstance, "ext_crypto_start_batch_verify_version_1", {TypeTuple{}, TypeTuple{}})),
        asObject(getTypedInstanceExport(intrinsicsInstance, "ext_trie_blake2_256_ordered_root_version_1", {{vt::i32}, {vt::i64}})),
        asObject(getTypedInstanceExport(intrinsicsInstance, "ext_misc_print_hex_version_1", {TypeTuple{}, {vt::i64}})),
        asObject(getTypedInstanceExport(intrinsicsInstance, "ext_misc_print_num_version_1", {TypeTuple{}, {vt::i64}})),
        asObject(getTypedInstanceExport(intrinsicsInstance, "ext_misc_print_utf8_version_1", {TypeTuple{}, {vt::i64}})),
        asObject(getTypedInstanceExport(intrinsicsInstance, "ext_misc_runtime_version_version_1", {{vt::i64}, {vt::i64}})),
        asObject(getTypedInstanceExport(intrinsicsInstance, "ext_default_child_storage_clear_version_1", {TypeTuple{}, {vt::i64, vt::i64}})),
        asObject(getTypedInstanceExport(intrinsicsInstance, "ext_default_child_storage_get_version_1", {{vt::i64}, {vt::i64, vt::i64}})),
        asObject(getTypedInstanceExport(intrinsicsInstance, "ext_default_child_storage_root_version_1", {{vt::i64}, {vt::i64}})),
        asObject(getTypedInstanceExport(intrinsicsInstance, "ext_default_child_storage_set_version_1", {TypeTuple{}, {vt::i64, vt::i64, vt::i64}})),
        asObject(getTypedInstanceExport(intrinsicsInstance, "ext_default_child_storage_storage_kill_version_1", {TypeTuple{}, {vt::i64}})),
        asObject(getTypedInstanceExport(intrinsicsInstance, "ext_allocator_free_version_1", {TypeTuple{}, {vt::i32}})),
        asObject(getTypedInstanceExport(intrinsicsInstance, "ext_allocator_malloc_version_1", {{vt::i32}, {vt::i32}})),
        asObject(getTypedInstanceExport(intrinsicsInstance, "ext_hashing_blake2_128_version_1", {{vt::i32}, {vt::i64}})),
        asObject(getTypedInstanceExport(intrinsicsInstance, "ext_hashing_blake2_256_version_1", {{vt::i32}, {vt::i64}})),
        asObject(getTypedInstanceExport(intrinsicsInstance, "ext_hashing_keccak_256_version_1", {{vt::i32}, {vt::i64}})),
        asObject(getTypedInstanceExport(intrinsicsInstance, "ext_hashing_sha2_256_version_1", {{vt::i32}, {vt::i64}})),
        asObject(getTypedInstanceExport(intrinsicsInstance, "ext_hashing_twox_128_version_1", {{vt::i32}, {vt::i64}})),
        asObject(getTypedInstanceExport(intrinsicsInstance, "ext_hashing_twox_64_version_1", {{vt::i32}, {vt::i64}})),
        asObject(getTypedInstanceExport(intrinsicsInstance, "ext_offchain_is_validator_version_1", {{vt::i32}, TypeTuple{}})),
        asObject(getTypedInstanceExport(intrinsicsInstance, "ext_offchain_local_storage_compare_and_set_version_1", {{vt::i32}, {vt::i32, vt::i64, vt::i64, vt::i64}})),
        asObject(getTypedInstanceExport(intrinsicsInstance, "ext_offchain_local_storage_get_version_1", {{vt::i64}, {vt::i32, vt::i64}})),
        asObject(getTypedInstanceExport(intrinsicsInstance, "ext_offchain_local_storage_set_version_1", {TypeTuple{}, {vt::i32, vt::i64, vt::i64}})),
        asObject(getTypedInstanceExport(intrinsicsInstance, "ext_offchain_network_state_version_1", {{vt::i64}, TypeTuple{}})),
        asObject(getTypedInstanceExport(intrinsicsInstance, "ext_offchain_random_seed_version_1", {{vt::i32}, TypeTuple{}})),
        asObject(getTypedInstanceExport(intrinsicsInstance, "ext_offchain_submit_transaction_version_1", {{vt::i64}, {vt::i64}})),
        asObject(getTypedInstanceExport(intrinsicsInstance, "ext_storage_append_version_1", {TypeTuple{}, {vt::i64, vt::i64}})),
        asObject(getTypedInstanceExport(intrinsicsInstance, "ext_storage_changes_root_version_1", {{vt::i64}, {vt::i64}})),
        asObject(getTypedInstanceExport(intrinsicsInstance, "ext_storage_clear_version_1", {TypeTuple{}, {vt::i64}})),
        asObject(getTypedInstanceExport(intrinsicsInstance, "ext_storage_clear_prefix_version_1", {TypeTuple{}, {vt::i64}})),
        asObject(getTypedInstanceExport(intrinsicsInstance, "ext_storage_commit_transaction_version_1", {TypeTuple{}, TypeTuple{}})),
        asObject(getTypedInstanceExport(intrinsicsInstance, "ext_storage_get_version_1", {{vt::i64}, {vt::i64}})),
        asObject(getTypedInstanceExport(intrinsicsInstance, "ext_storage_next_key_version_1", {{vt::i64}, {vt::i64}})),
        asObject(getTypedInstanceExport(intrinsicsInstance, "ext_storage_read_version_1", {{vt::i64}, {vt::i64, vt::i64, vt::i32}})),
        asObject(getTypedInstanceExport(intrinsicsInstance, "ext_storage_rollback_transaction_version_1", {TypeTuple{}, TypeTuple{}})),
        asObject(getTypedInstanceExport(intrinsicsInstance, "ext_storage_root_version_1", {{vt::i64}, TypeTuple{}})),
        asObject(getTypedInstanceExport(intrinsicsInstance, "ext_storage_set_version_1", {TypeTuple{}, {vt::i64, vt::i64}})),
        asObject(getTypedInstanceExport(intrinsicsInstance, "ext_storage_start_transaction_version_1", {TypeTuple{}, TypeTuple{}})),
        asObject(getTypedInstanceExport(intrinsicsInstance, "ext_offchain_index_set_version_1", {TypeTuple{}, {vt::i64, vt::i64}})),
    };
    // clang-format on
  }

  outcome::result<std::vector<uint8_t>> ModuleProvider::execute(
      std::string_view name,
      const boost::optional<common::Hash256> &state_root,
      CallPersistency persistency,
      const std::vector<uint8_t> &scale_args) {
    auto module = parseModule(wasm_provider_->getStateCode());

    using namespace WAVM;
    using namespace WAVM::IR;
    using namespace WAVM::Runtime;

    GCPointer<Compartment> compartment = createCompartment();
    GCPointer<Context> context = createContext(compartment);

    // Create an instance that encapsulates the intrinsic function in a way
    // that allows it to be imported by WASM instances.
    Instance *intrinsicsInstance = WAVM::Intrinsics::instantiateModule(
        compartment, {WAVM_INTRINSIC_MODULE_REF(env)}, "Kagome runtime");

    // Instantiate the WASM module using the intrinsic function as its import.
    const FunctionType i32_to_void({}, {ValueType::i32});
    GCPointer<Instance> instance = instantiateModule(
        compartment, module, getImports(intrinsicsInstance), "hello");
    WAVM::Runtime::Memory *raw_memory = getDefaultMemory(instance);
    //    WAVM::Runtime::Memory *raw_memory = getTypedInstanceExport(
    //        instance,
    //        "memory",
    //        MemoryType(false, IndexType::i32, SizeConstraints{1,
    //        UINT64_MAX}));
    auto memory = std::make_shared<WasmMemoryImpl>(raw_memory);

    static const FunctionType i32_i32_to_i64({ValueType::i64},
                                             {ValueType::i32, ValueType::i32});
    Function *export_function =
        getTypedInstanceExport(instance, std::string(name), i32_i32_to_i64);

    UntaggedValue version_args[2]{U32(0), U32(0)};
    UntaggedValue version_result[1]{U64(0)};

    if (not scale_args.empty()) {
      uint32_t len = scale_args.size();
      uint32_t ptr = memory->allocate(len);
      memory->storeBuffer(ptr, common::Buffer(scale_args));
      version_args[0] = ptr;
      version_args[1] = len;
    }

    std::shared_ptr<extensions::Extension> extension =
        extension_factory_->createExtension(memory, trie_storage_provider_);
    [[maybe_unused]] auto a = trie_storage_provider_->setToEphemeral();
    WavmInterfaceKeeper keeper{extension};

    invokeFunction(
        context, export_function, i32_i32_to_i64, version_args, version_result);

    if (version_result[0].u64 != 0) {
      WasmResult r(version_result[0].u64);
      auto result_buf = memory->loadN(r.address, r.length);

      memory->reset();
      return result_buf.toVector();
    }

    memory->reset();
    return std::vector<uint8_t>{};
  }

}  // namespace kagome::runtime::wavm
