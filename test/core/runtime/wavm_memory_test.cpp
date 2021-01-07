/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include <gtest/gtest.h>

#include <WAVM/Inline/Serialization.h>
#include <WAVM/WASM/WASM.h>
#include "core/runtime/runtime_test.hpp"
#include "runtime/wavm/wasm_memory_impl.hpp"

using kagome::runtime::wavm::WasmMemoryImpl;
using namespace kagome;
using namespace WAVM;
using namespace WAVM::IR;
using namespace WAVM::Runtime;

class MemoryHeapTest : public RuntimeTest {
 public:
  static WAVM::IR::Module parseModule(const common::Buffer& code) {
    using namespace WAVM;
    using namespace WAVM::IR;
    using namespace WAVM::Runtime;

    // first parse module
    IR::Module moduleIR;

    bool a = WASM::loadBinaryModule(code.toVector().data(), code.size(), moduleIR);

    return moduleIR;
  }

  void SetUp() override {
    RuntimeTest::SetUp();

    char helloWAST[]
        = "(module\n"
//          "  (import \"\" \"hello\" (func $1 (param i32 i32) (result i32)))\n"
          "  (memory (export \"memory\") 1)\n"
//          "  (global $nextFreeMemoryAddress (mut i32) (i32.const 0))\n"
//          "  (func (export \"malloc\") (param $numBytes i32) (result i32)\n"
//          "    (local $address i32)\n"
//          "    (local.set $address (global.get $nextFreeMemoryAddress))\n"
//          "    (global.set $nextFreeMemoryAddress\n"
//          "      (i32.add (local.get $address) (local.get $numBytes)))\n"
//          "    (local.get $address)\n"
//          "  )\n"
//          "  (func (export \"run\") (param $address i32) (param $num_chars i32) (result i32)\n"
//          "    (call $1 (local.get $address) (local.get $num_chars))\n"
//          "  )\n"
          ")";

    IR::Module irModule;
    std::vector<WAST::Error> wastErrors;
    if(!WAST::parseModule(helloWAST, sizeof(helloWAST), irModule, wastErrors))
    { FAIL(); }

    ModuleRef module = compileModule(irModule);
    instance
        = instantiateModule(compartment, module, {}, "kagome runtime");
    WAVM::Runtime::Memory *raw_memory = getTypedInstanceExport(instance, "memory", MemoryType(false, IndexType::i32, SizeConstraints{1, UINT64_MAX}));
    memory_ = std::make_shared<WasmMemoryImpl>(raw_memory);
  }

 protected:


  const static uint32_t memory_size_ = 4096;  // one page size
  std::shared_ptr<WasmMemoryImpl> memory_;

  GCPointer<Compartment> compartment = createCompartment();
  GCPointer<Instance> instance;
};

/**
 * @given memory of arbitrary size
 * @when trying to allocate memory of size 0
 * @then zero pointer is returned
 */
TEST_F(MemoryHeapTest, Return0WhenSize0) {
  ASSERT_EQ(memory_->allocate(0), 0);
  auto addr = memory_->allocate(2 * 1024 * 1024);
  memory_->store<size_t>(addr, 42);
  ASSERT_EQ(memory_->load64u(addr), 42);

  memory_->deallocate(addr);
  memory_->store<size_t>(addr, 43);
  ASSERT_EQ(memory_->load64u(addr), 43);

  memory_->reset();
}
//
///**
// * @given memory of size memory_size_
// * @when trying to allocate memory of size bigger than memory_size_ but less
// * than max memory size
// * @then -1 is not returned by allocate method indicating that memory was
// * allocated
// */
//TEST_F(MemoryHeapTest, AllocatedMoreThanMemorySize) {
//  const auto allocated_memory = memory_size_ + 1;
//  ASSERT_NE(memory_.allocate(allocated_memory), -1);
//}
//
///**
// * @given memory of size memory_size_ that is fully allocated
// * @when trying to allocate memory of size bigger than
// * (kMaxMemorySize-memory_size_)
// * @then -1 is not returned by allocate method indicating that memory was not
// * allocated
// */
//TEST_F(MemoryHeapTest, AllocatedTooBigMemoryFailed) {
//  // fully allocate memory
//  auto ptr1 = memory_.allocate(memory_size_);
//  // check that ptr1 is not -1, thus memory was allocated
//  ASSERT_NE(ptr1, -1);
//
//  // The memory size that can be allocated is within interval (0, kMaxMemorySize
//  // - memory_size_]. Trying to allocate more
//  auto big_memory_size = WasmMemoryImpl::kMaxMemorySize - memory_size_ + 1;
//  ASSERT_EQ(memory_.allocate(big_memory_size), 0);
//}
//
///**
// * @given memory with already allocated memory of size1
// * @when allocate memory with size2
// * @then the pointer pointing to the end of the first memory chunk is returned
// */
//TEST_F(MemoryHeapTest, ReturnOffsetWhenAllocated) {
//  const size_t size1 = 2049;
//  const size_t size2 = 2045;
//
//  // allocate memory of size 1
//  auto ptr1 = memory_.allocate(size1);
//  // first memory chunk is always allocated at 1
//  ASSERT_EQ(ptr1, 1);
//
//  // allocated second memory chunk
//  auto ptr2 = memory_.allocate(size2);
//  // second memory chunk is placed right after the first one (alligned by 4)
//  ASSERT_EQ(ptr2, kagome::runtime::binaryen::roundUpAlign(size1 + ptr1));
//}
//
///**
// * @given memory with allocated memory chunk
// * @when this memory is deallocated
// * @then the size of this memory chunk is returned
// */
//TEST_F(MemoryHeapTest, DeallocateExisingMemoryChunk) {
//  const size_t size1 = 3;
//
//  auto ptr1 = memory_.allocate(size1);
//
//  auto opt_deallocated_size = memory_.deallocate(ptr1);
//  ASSERT_TRUE(opt_deallocated_size.has_value());
//  ASSERT_EQ(*opt_deallocated_size, size1);
//}
//
///**
// * @given memory with memory chunk allocated at the beginning
// * @when deallocate is invoked with ptr that does not point to any memory
// * chunk
// * @then deallocate returns none
// */
//TEST_F(MemoryHeapTest, DeallocateNonexistingMemoryChunk) {
//  const size_t size1 = 2047;
//
//  memory_.allocate(size1);
//
//  auto ptr_to_nonexisting_chunk = 2;
//  auto opt_deallocated_size = memory_.deallocate(ptr_to_nonexisting_chunk);
//  ASSERT_FALSE(opt_deallocated_size.has_value());
//}
//
///**
// * @given memory with two memory chunk filling entire memory
// * @when first memory chunk of size size1 is deallocated @and new memory chunk
// * of the same size is trying to be allocated on that memory
// * @then it is allocated on the place of the first memory chunk
// */
//TEST_F(MemoryHeapTest, AllocateAfterDeallocate) {
//  // two memory sizes totalling to the total memory size
//  const size_t size1 = 2045;
//  const size_t size2 = 2047;
//
//  // allocate two memory chunks with total size equal to the memory size
//  auto ptr1 = memory_.allocate(size1);
//  memory_.allocate(size2);
//
//  // deallocate first memory chunk
//  memory_.deallocate(ptr1);
//
//  // allocate new memory chunk
//  auto ptr1_1 = memory_.allocate(size1);
//  // expected that it will be allocated on the same place as the first memory
//  // chunk that was deallocated
//  ASSERT_EQ(ptr1, ptr1_1);
//}
//
///**
// * @given full memory with deallocated memory chunk of size1
// * @when allocate memory chunk of size bigger than size1
// * @then allocate returns memory of size bigger
// */
//TEST_F(MemoryHeapTest, AllocateTooBigMemoryAfterDeallocate) {
//  // two memory sizes totalling to the total memory size
//  const size_t size1 = 2047;
//  const size_t size2 = 2049;
//
//  // allocate two memory chunks with total size equal to the memory size
//  auto ptr1 = memory_.allocate(size1);
//  auto ptr2 = memory_.allocate(size2);
//
//  // calculate memory offset after two allocations
//  auto mem_offset = ptr2 + size2;
//
//  // deallocate first memory chunk
//  memory_.deallocate(ptr1);
//
//  // allocate new memory chunk with bigger size than the space left in the
//  // memory
//  auto ptr3 = memory_.allocate(size1 + 1);
//
//  // memory is allocated on mem offset (aligned by 4)
//  ASSERT_EQ(ptr3, kagome::runtime::binaryen::roundUpAlign(mem_offset));
//}
//
///**
// * @given arbitrary buffer of size N
// * @when this buffer is stored in memory heap @and then load of N bytes is done
// * @then the same buffer is returned
// */
//TEST_F(MemoryHeapTest, LoadNTest) {
//  const size_t N = 3;
//
//  kagome::common::Buffer b(N, 'c');
//
//  auto ptr = memory_.allocate(N);
//
//  memory_.storeBuffer(ptr, b);
//
//  auto res_b = memory_.loadN(ptr, N);
//  ASSERT_EQ(b, res_b);
//}
//
///**
// * @given Some memory is allocated
// * @when Memory is reset
// * @then Allocated memory's offset is 1
// */
//TEST_F(MemoryHeapTest, ResetTest) {
//  const size_t N = 42;
//  ASSERT_EQ(memory_.allocate(N), 1);
//  memory_.reset();
//  ASSERT_EQ(memory_.allocate(N), 1);
//}
