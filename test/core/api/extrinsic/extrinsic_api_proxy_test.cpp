/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

// needs to be included at the top, don't move it down
#include "mock/api/extrinsic/extrinsic_api_mock.hpp"

#include "api/extrinsic/extrinsic_api_proxy.hpp"

#include <iostream>
#include <memory>

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include "api/extrinsic/error.hpp"
#include "common/blob.hpp"
#include "common/buffer.hpp"
#include "primitives/extrinsic.hpp"
#include "primitives/extrinsic_api_primitives.hpp"
#include "testutil/literals.hpp"

using kagome::api::ExtrinsicApiError;
using kagome::api::ExtrinsicApiMock;
using kagome::api::ExtrinsicApiProxy;
using kagome::common::Buffer;
using kagome::common::Hash256;
using kagome::primitives::Extrinsic;
using kagome::primitives::ExtrinsicKey;
using kagome::primitives::Metadata;
using kagome::primitives::Subscriber;
using kagome::primitives::SubscriptionId;

using ::testing::_;
using ::testing::DoAll;
using ::testing::Return;

class ExtrinsicSubmissionProxyTest : public ::testing::Test {
  template <class T>
  using sptr = std::shared_ptr<T>;

 protected:
  sptr<ExtrinsicApiMock> api = std::make_shared<ExtrinsicApiMock>();

  ExtrinsicApiProxy proxy{api};
  std::vector<uint8_t> bytes = {0, 1};
  Extrinsic extrinsic{"0001"_hex2buf};
};

/**
 * @given extrinsic submission proxy instance configured with mock api
 * @when submit_extrinsic proxy method is called
 * @then submit_extrinsic api method call is executed
 * and result of proxy method corresponds to result of api method
 */
TEST_F(ExtrinsicSubmissionProxyTest, SubmitExtrinsicSuccess) {
  Hash256 hash{};
  hash.fill(1);
  std::vector<uint8_t> hash_as_vector{hash.begin(), hash.end()};

  EXPECT_CALL(*api, submitExtrinsic(_)).WillOnce(Return(hash));

  std::vector<uint8_t> result;
  ASSERT_NO_THROW(result = proxy.submit_extrinsic(extrinsic.data.toHex()));
  ASSERT_EQ(result, hash_as_vector);
}

/**
 * @given extrinsic submission proxy instance configured with mock api
 * @when submit_extrinsic proxy method is called and mocked api returns error
 * @then submit_extrinsic proxy method throws jsonrpc::Fault exception
 */
TEST_F(ExtrinsicSubmissionProxyTest, SubmitExtrinsicFail) {
  EXPECT_CALL(*api, submitExtrinsic(_))
      .WillOnce(Return(
          outcome::failure(ExtrinsicApiError::INVALID_STATE_TRANSACTION)));

  ASSERT_THROW(proxy.submit_extrinsic(extrinsic.data.toHex()), jsonrpc::Fault);
}
