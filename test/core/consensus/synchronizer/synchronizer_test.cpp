/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "consensus/synchronizer/impl/synchronizer_impl.hpp"

#include <functional>

#include <gtest/gtest.h>
#include <blockchain/block_tree_error.hpp>
#include <boost/optional.hpp>
#include "mock/core/blockchain/block_tree_mock.hpp"
#include "mock/core/blockchain/header_backend_mock.hpp"
#include "mock/core/network/peer_client_mock.hpp"
#include "mock/core/network/peer_server_mock.hpp"
#include "network/network_state.hpp"
#include "primitives/block.hpp"
#include "testutil/gmock_actions.hpp"
#include "testutil/literals.hpp"

using namespace kagome;
using namespace blockchain;
using namespace network;
using namespace consensus;
using namespace primitives;
using namespace common;
using namespace libp2p::peer;

using testing::_;
using testing::Ref;
using testing::Return;
using testing::ReturnRef;

class SynchronizerTest : public testing::Test {
 public:
  void SetUp() override {
    block1_.header.parent_hash.fill(2);
    block1_hash_.fill(3);
    block2_.header.parent_hash = block1_hash_;
    block2_hash_.fill(4);

    EXPECT_CALL(*server_, onBlocksRequest(_))
        .WillOnce(testing::SaveArg<0>(&on_blocks_request));

    synchronizer_ =
        std::make_shared<SynchronizerImpl>(tree_, headers_, network_state_);
  }

  std::function<outcome::result<BlockResponse>(const BlockRequest &)>
      on_blocks_request;

  PeerInfo peer1_info_{"peer1"_peerid, {}}, peer2_info_{"peer2"_peerid, {}};

  std::shared_ptr<BlockTreeMock> tree_ = std::make_shared<BlockTreeMock>();
  std::shared_ptr<HeaderRepositoryMock> headers_ =
      std::make_shared<HeaderRepositoryMock>();
  std::shared_ptr<PeerClientMock> peer1_ = std::make_shared<PeerClientMock>(),
                                  peer2_ = std::make_shared<PeerClientMock>();
  std::shared_ptr<PeerServerMock> server_ = std::make_shared<PeerServerMock>();
  std::shared_ptr<NetworkState> network_state_ = std::make_shared<NetworkState>(
      PeerClientsMap{{peer1_info_.id, peer1_}, {peer2_info_.id, peer2_}},
      server_);

  std::shared_ptr<Synchronizer> synchronizer_;

  Block block1_{{{}, 2}, {{{0x11, 0x22}}, {{0x55, 0x66}}}},
      block2_{{{}, 3}, {{{0x13, 0x23}}, {{0x35, 0x63}}}};
  Hash256 block1_hash_{}, block2_hash_;
};

/**
 * @given synchronizer
 * @when announcing a block header
 * @then all peers receive a corresponding message
 */
TEST_F(SynchronizerTest, Announce) {
  // GIVEN
  EXPECT_CALL(*peer1_, blockAnnounce(BlockAnnounce{block1_.header}, _))
      .WillOnce(Arg1CallbackWithArg(outcome::success()));
  EXPECT_CALL(*peer2_, blockAnnounce(BlockAnnounce{block1_.header}, _))
      .WillOnce(Arg1CallbackWithArg(outcome::success()));

  // WHEN
  synchronizer_->announce(block1_.header);

  // THEN
}

/**
 * @given synchronizer
 * @when requesting for blocks without specifying a hash, which is to be in the
 * response chain
 * @then a returned chain consists of maximum number of blocks, returned in the
 * specified order
 */
TEST_F(SynchronizerTest, RequestWithoutHash) {
  // GIVEN
  EXPECT_CALL(*tree_, deepestLeaf()).WillOnce(ReturnRef(block1_hash_));
  BlockRequest expected_request{0,
                                BlockRequest::kBasicAttributes,
                                block1_hash_,
                                boost::none,
                                Direction::DESCENDING,
                                boost::none};

  EXPECT_CALL(*peer1_, blocksRequest(expected_request, _))
      .WillOnce(Arg1CallbackWithArg(
          BlockResponse{0,
                        {{block1_hash_, block1_.header, block1_.body},
                         {block2_hash_, block2_.header, block2_.body}}}));

  EXPECT_CALL(*tree_, addBlock(block1_))
      .WillOnce(Return(BlockTreeError::BLOCK_EXISTS));
  EXPECT_CALL(*tree_, addBlock(block2_)).WillOnce(Return(outcome::success()));

  // WHEN
  auto finished = false;
  synchronizer_->requestBlocks(peer1_info_, [&finished](auto &&res) mutable {
    ASSERT_TRUE(res);
    finished = true;
  });

  // THEN
  ASSERT_TRUE(finished);
}

/**
 * @given synchronizer
 * @when requesting for blocks, specifying a hash, which is to be in the
 * response chain
 * @then a returned chain has that block
 */
TEST_F(SynchronizerTest, RequestWithHash) {
  // GIVEN
  EXPECT_CALL(*tree_, getLastFinalized()).WillOnce(Return(block1_hash_));
  BlockRequest expected_request{0,
                                BlockRequest::kBasicAttributes,
                                block1_hash_,
                                block2_hash_,
                                Direction::DESCENDING,
                                boost::none};

  EXPECT_CALL(*peer2_, blocksRequest(expected_request, _))
      .WillOnce(Arg1CallbackWithArg(
          BlockResponse{0,
                        {{block1_hash_, block1_.header, block1_.body},
                         {block2_hash_, block2_.header, block2_.body}}}));

  EXPECT_CALL(*tree_, addBlock(block1_))
      .WillOnce(Return(BlockTreeError::BLOCK_EXISTS));
  EXPECT_CALL(*tree_, addBlock(block2_)).WillOnce(Return(outcome::success()));

  // WHEN
  auto finished = false;
  synchronizer_->requestBlocks(
      peer2_info_, block2_hash_, [&finished](auto &&res) mutable {
        ASSERT_TRUE(res);
        finished = true;
      });

  // THEN
  ASSERT_TRUE(finished);
}

/**
 * @given synchronizer
 * @when a request for blocks arrives
 * @then an expected response is formed @and sent
 */
TEST_F(SynchronizerTest, ProcessRequest) {
  // GIVEN
  BlockRequest received_request{};

  // WHEN
  auto request_res = on_blocks_request(received_request);

  // THEN
  ASSERT_TRUE(request_res);
}
