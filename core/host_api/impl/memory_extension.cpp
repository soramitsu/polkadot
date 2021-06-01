/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include <exception>

#include <boost/assert.hpp>

#include "host_api/impl/memory_extension.hpp"
#include "runtime/wasm_memory.hpp"

namespace kagome::host_api {
  MemoryExtension::MemoryExtension(std::shared_ptr<runtime::Memory> memory)
      : memory_(std::move(memory)),
        logger_{log::createLogger("MemoryExtension", "host_api")} {
    BOOST_ASSERT_MSG(memory_ != nullptr, "memory is nullptr");
  }

  void MemoryExtension::reset() {
    memory_->reset();
  }

  runtime::WasmPointer MemoryExtension::ext_allocator_malloc_version_1(
      runtime::WasmSize size) {
    return memory_->allocate(size);
  }

  void MemoryExtension::ext_allocator_free_version_1(runtime::WasmPointer ptr) {
    auto opt_size = memory_->deallocate(ptr);
    if (not opt_size) {
      logger_->warn(
          "Ptr {} does not point to any memory chunk in wasm memory. Nothing "
          "deallocated",
          ptr);
      return;
    }
  }
}  // namespace kagome::host_api
