/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include <gtest/gtest.h>

#include <fstream>

#include "core/runtime/runtime_test.hpp"
#include "mock/core/blockchain/block_header_repository_mock.hpp"
#include "mock/core/storage/trie/trie_storage_mock.hpp"
#include "runtime/wavm/core.hpp"

using kagome::blockchain::BlockHeaderRepositoryMock;
using kagome::common::Buffer;
using kagome::extensions::ExtensionFactoryImpl;
using kagome::primitives::Block;
using kagome::primitives::BlockHeader;
using kagome::primitives::BlockId;
using kagome::primitives::BlockNumber;
using kagome::primitives::Extrinsic;
using kagome::runtime::WasmMemory;
using kagome::runtime::wavm::CoreWavm;
using kagome::runtime::wavm::WasmMemoryImpl;

using ::testing::_;
using ::testing::Return;

namespace fs = boost::filesystem;

class CoreTest : public RuntimeTest {
 public:
  using RuntimeTest::RuntimeTest;
  void SetUp() override {
    RuntimeTest::SetUp();

    core_ = std::make_shared<CoreWavm>(
        wasm_provider_,
        extension_factory,
        storage_provider);
  }

 protected:
  std::shared_ptr<CoreWavm> core_;
};

/**
 * @given initialized core api
 * @when version is invoked
 * @then successful result is returned
 */
TEST_F(CoreTest, VersionTest) {
  auto version_res = core_->version(boost::none);
  ASSERT_TRUE(version_res);
  FAIL() << version_res.value().impl_name;
}
