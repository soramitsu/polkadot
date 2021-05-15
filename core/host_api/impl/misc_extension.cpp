/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "host_api/impl/misc_extension.hpp"

#include "primitives/version.hpp"
#include "runtime/core.hpp"
#include "runtime/wavm/executor.hpp"
#include "runtime/wavm/impl/intrinsic_resolver.hpp"
#include "runtime/wavm/impl/memory.hpp"
#include "runtime/wavm/module_repository.hpp"
#include "scale/scale.hpp"

namespace kagome::host_api {

  MiscExtension::MiscExtension(
      uint64_t chain_id,
      std::shared_ptr<runtime::wavm::ModuleRepository> module_repo,
      std::shared_ptr<runtime::wavm::Memory> memory,
      std::shared_ptr<runtime::wavm::IntrinsicResolver> intrinsic_resolver,
      std::shared_ptr<runtime::TrieStorageProvider> storage_provider,
      std::shared_ptr<blockchain::BlockHeaderRepository> block_header_repo)
      : module_repo_{std::move(module_repo)},
        memory_{std::move(memory)},
        intrinsic_resolver_{std::move(intrinsic_resolver)},
        storage_provider_{std::move(storage_provider)},
        block_header_repo_{std::move(block_header_repo)},
        logger_{log::createLogger("MiscExtension", "host_api")},
        chain_id_{chain_id} {
    BOOST_ASSERT(module_repo_);
    BOOST_ASSERT(memory_);
    BOOST_ASSERT(intrinsic_resolver_);
    BOOST_ASSERT(storage_provider_);
    BOOST_ASSERT(block_header_repo_);
  }

  runtime::WasmSpan MiscExtension::ext_misc_runtime_version_version_1(
      runtime::WasmSpan data) const {
    SL_TRACE(logger_, "call {}", __FUNCTION__);
    auto [ptr, len] = runtime::splitSpan(data);
    auto code = memory_->loadN(ptr, len);

    auto executor = runtime::wavm::Executor{
        storage_provider_, memory_, module_repo_, block_header_repo_};
    auto version_res =
        executor.callAtLatest<boost::optional<primitives::Version>>(
            "Core_version",
            boost::optional<primitives::BlockHash>(boost::none));

    static const auto kErrorRes =
        scale::encode<boost::optional<primitives::Version>>(boost::none)
            .value();

    if (version_res.has_value()) {
      auto enc_version_res = scale::encode(
          boost::make_optional(scale::encode(version_res.value()).value()));
      if (enc_version_res.has_error()) {
        logger_->error(
            "Error encoding ext_misc_runtime_version_version_1 result: {}",
            enc_version_res.error().message());
        return memory_->storeBuffer(kErrorRes);
      }
      auto res_span = memory_->storeBuffer(enc_version_res.value());
      return res_span;
    }
    logger_->error("Error inside Core_version: {}",
                   version_res.error().message());
    return memory_->storeBuffer(kErrorRes);
  }

  void MiscExtension::ext_misc_print_hex_version_1(
      runtime::WasmSpan data) const {
    auto [ptr, len] = runtime::splitSpan(data);
    auto buf = memory_->loadN(ptr, len);
    logger_->info("hex: {}", buf.toHex());
  }

  void MiscExtension::ext_misc_print_num_version_1(uint64_t value) const {
    logger_->info("num: {}", value);
  }

  void MiscExtension::ext_misc_print_utf8_version_1(
      runtime::WasmSpan data) const {
    auto [ptr, len] = runtime::splitSpan(data);
    auto buf = memory_->loadN(ptr, len);
    logger_->info("utf8: {}", buf.toString());
  }

}  // namespace kagome::host_api
