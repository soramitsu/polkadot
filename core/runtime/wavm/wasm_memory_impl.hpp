/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef KAGOME_CORE_RUNTIME_WAVM_WASM_MEMORY_IMPL_HPP
#define KAGOME_CORE_RUNTIME_WAVM_WASM_MEMORY_IMPL_HPP

#include "runtime/wasm_memory.hpp"

#include <unordered_map>

#include "WAVM/IR/Module.h"
#include "WAVM/IR/Types.h"
#include "WAVM/IR/Value.h"
#include "WAVM/Runtime/Intrinsics.h"
#include "WAVM/Runtime/Runtime.h"
#include "WAVM/WASTParse/WASTParse.h"
#include "WAVM/wavm-c/wavm-c.h"
#include "common/logger.hpp"
#include "primitives/math.hpp"

namespace kagome::runtime::wavm {

  // Alignment for pointers, same with substrate:
  // https://github.com/paritytech/substrate/blob/743981a083f244a090b40ccfb5ce902199b55334/primitives/allocator/src/freeing_bump.rs#L56
  inline const uint8_t kAlignment = 8;

  class WasmMemoryImpl : public WasmMemory {
   public:
    ~WasmMemoryImpl() override = default;

    explicit WasmMemoryImpl(WAVM::Runtime::Memory *memory);

    void reset() override;

    WasmSize size() const override {
      return WAVM::Runtime::getMemoryNumPages(memory_) * MEMORY_PAGE_SIZE;
    }

    void resize(WasmSize newSize) override {
      auto new_page_number = (newSize / MEMORY_PAGE_SIZE) + 1;
      WAVM::Runtime::growMemory(memory_, new_page_number);
    }

    WasmPointer allocate(WasmSize size) override;
    boost::optional<WasmSize> deallocate(WasmPointer ptr) override;

    template <typename T>
    inline T load(WasmPointer addr) const {
      return *WAVM::Runtime::memoryArrayPtr<T>(memory_, addr, sizeof(T));
    }

    int8_t load8s(WasmPointer addr) const override;
    uint8_t load8u(WasmPointer addr) const override;
    int16_t load16s(WasmPointer addr) const override;
    uint16_t load16u(WasmPointer addr) const override;
    int32_t load32s(WasmPointer addr) const override;
    uint32_t load32u(WasmPointer addr) const override;
    int64_t load64s(WasmPointer addr) const override;
    uint64_t load64u(WasmPointer addr) const override;
    std::array<uint8_t, 16> load128(WasmPointer addr) const override;
    common::Buffer loadN(kagome::runtime::WasmPointer addr,
                         kagome::runtime::WasmSize n) const override;
    std::string loadStr(kagome::runtime::WasmPointer addr,
                        kagome::runtime::WasmSize n) const override;

    template <typename T>
    void store(WasmPointer addr, T value) {
      memcpy(WAVM::Runtime::memoryArrayPtr<char>(memory_, addr, sizeof(value)), &value, sizeof(value));
    }

    void store8(WasmPointer addr, int8_t value) override;
    void store16(WasmPointer addr, int16_t value) override;
    void store32(WasmPointer addr, int32_t value) override;
    void store64(WasmPointer addr, int64_t value) override;
    void store128(WasmPointer addr,
                  const std::array<uint8_t, 16> &value) override;
    void storeBuffer(kagome::runtime::WasmPointer addr,
                     gsl::span<const uint8_t> value) override;

    WasmSpan storeBuffer(gsl::span<const uint8_t> value) override;

   private:
    WAVM::Runtime::Memory *memory_;
    WasmPointer offset_;

    common::Logger logger_{};


    // map containing addresses of allocated MemoryImpl chunks
    std::unordered_map<WasmPointer, WasmSize> allocated_{};

    // map containing addresses to the deallocated MemoryImpl chunks
    std::unordered_map<WasmPointer, WasmSize> deallocated_{};

    /**
     * Finds memory segment of given size among deallocated pieces of memory
     * and allocates a memory there
     * @param size of target memory
     * @return address of memory of given size, or -1 if it is impossible to
     * allocate this amount of memory
     */
    WasmPointer freealloc(WasmSize size);

    /**
     * Finds memory segment of given size among deallocated pieces of memory
     * @param size of target memory
     * @return address of memory of given size, or 0 if it is impossible to
     * allocate this amount of memory
     */
    WasmPointer findContaining(WasmSize size);

    /**
     * Resize memory and allocate memory segment of given size
     * @param size memory size to be allocated
     * @return pointer to the allocated memory @or 0 if it is impossible to
     * allocate this amount of memory
     */
    WasmPointer growAlloc(WasmSize size);

    template <typename T>
    inline T roundUpAlign(T t) {
      return math::roundUp<kAlignment>(t);
    }
  };

}  // namespace kagome::runtime::wavm

#endif  // KAGOME_CORE_RUNTIME_WAVM_WASM_MEMORY_IMPL_HPP
