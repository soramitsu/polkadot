/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "consensus/babe/impl/babe_impl.hpp"

#include <chrono>
#include <memory>

#include <gtest/gtest.h>
#include <boost/asio/basic_waitable_timer.hpp>
#include <boost/asio/io_context.hpp>
#include "clock/impl/clock_impl.hpp"
#include "mock/core/authorship/proposer_mock.hpp"
#include "mock/core/blockchain/block_tree_mock.hpp"
#include "mock/core/clock/clock_mock.hpp"
#include "mock/core/consensus/babe_lottery_mock.hpp"
#include "mock/core/consensus/consensus_network_mock.hpp"
#include "mock/core/crypto/hasher_mock.hpp"
#include "primitives/block.hpp"
#include "testutil/sr25519_utils.hpp"

using namespace kagome;
using namespace consensus;
using namespace blockchain;
using namespace authorship;
using namespace crypto;
using namespace primitives;
using namespace clock;
using namespace common;

using testing::_;
using testing::Ref;
using testing::Return;
using testing::ReturnRef;
using std::chrono_literals::operator""ms;

Hash256 createHash(uint8_t byte) {
  Hash256 h;
  h.fill(byte);
  return h;
}

class BabeTest : public testing::Test {
 public:
  boost::asio::io_context io_context_;

  std::shared_ptr<BabeLotteryMock> lottery_ =
      std::make_shared<BabeLotteryMock>();
  std::shared_ptr<ProposerMock> proposer_ = std::make_shared<ProposerMock>();
  std::shared_ptr<BlockTreeMock> block_tree_ =
      std::make_shared<BlockTreeMock>();
  std::shared_ptr<ConsensusNetworkMock> network_ =
      std::make_shared<ConsensusNetworkMock>();
  SR25519Keypair keypair_{generateSR25519Keypair()};
  AuthorityIndex authority_id_ = 1;
  std::shared_ptr<SystemClockMock> clock_ = std::make_shared<SystemClockMock>();
  std::shared_ptr<HasherMock> hasher_ = std::make_shared<HasherMock>();
  libp2p::event::Bus event_bus_;

  BabeImpl babe_{
      lottery_,
      proposer_,
      block_tree_,
      network_,
      keypair_,
      authority_id_,
      clock_,
      hasher_,
      boost::asio::basic_waitable_timer<std::chrono::system_clock>{io_context_},
      event_bus_};

  Epoch epoch_{0, 0, 2, 60ms, {{}}, 100, {}};

  VRFOutput leader_vrf_output_{
      50, {0x11, 0x22, 0x33, 0x44, 0x11, 0x22, 0x33, 0x44, 0x11, 0x22, 0x33,
           0x44, 0x11, 0x22, 0x33, 0x44, 0x11, 0x22, 0x33, 0x44, 0x11, 0x22,
           0x33, 0x44, 0x11, 0x22, 0x33, 0x44, 0x11, 0x22, 0x33, 0x44}};
  BabeLottery::SlotsLeadership leadership_{boost::none, leader_vrf_output_};

  BlockHash best_block_hash_{{0x41, 0x22, 0x33, 0x44, 0x11, 0x22, 0x33, 0x44,
                              0x11, 0x22, 0x33, 0x44, 0x11, 0x22, 0x33, 0x54,
                              0x11, 0x22, 0x33, 0x44, 0x11, 0x22, 0x33, 0x44,
                              0x11, 0x22, 0x33, 0x44, 0x11, 0x24, 0x33, 0x44}};

  BlockHeader block_header_{
      createHash(0), 2, createHash(1), createHash(2), {{5}}};
  Extrinsic extrinsic_{{1, 2, 3}};
  Block created_block_{block_header_, {extrinsic_}};

  Hash256 created_block_hash_{createHash(3)};
};

ACTION_P(CheckBlock, block) {
  auto block_to_check = arg0;
  ASSERT_EQ(block_to_check.header.digests.size(), 2);
  block_to_check.header.digests.pop_back();
  ASSERT_EQ(block_to_check, block);
}

/**
 * @given BABE production
 * @when running it in epoch with two slots @and out node is a leader in one of
 * them
 * @then block is emitted in the leader slot @and after two slots BABE moves to
 * the next epoch
 */
TEST_F(BabeTest, Success) {
  // GIVEN
  SystemClockImpl real_clock{};
  auto test_begin = real_clock.now();

  // runEpoch
  epoch_.randomness.fill(0);
  EXPECT_CALL(*lottery_, slotsLeadership(epoch_, keypair_))
      .WillOnce(Return(leadership_));

  // runSlot (3 times)
  EXPECT_CALL(*clock_, now())
      .WillOnce(Return(test_begin))
      .WillOnce(Return(test_begin + 60ms))
      .WillOnce(Return(test_begin + 120ms));

  // processSlotLeadership
  // we are not leader of the first slot, but leader of the second
  EXPECT_CALL(*block_tree_, deepestLeaf())
      .WillOnce(ReturnRef(best_block_hash_));
  EXPECT_CALL(*proposer_, propose(BlockId{best_block_hash_}, _, _))
      .WillOnce(Return(created_block_));
  EXPECT_CALL(*hasher_, blake2b_256(_)).WillOnce(Return(created_block_hash_));
  EXPECT_CALL(*network_, broadcast(_)).WillOnce(CheckBlock(created_block_));

  // finishEpoch
  auto new_epoch = epoch_;
  ++new_epoch.epoch_index;
  new_epoch.randomness.fill(5);
  EXPECT_CALL(*lottery_,
              computeRandomness(epoch_.randomness, new_epoch.epoch_index))
      .WillOnce(Return(new_epoch.randomness));

  // runEpoch
  EXPECT_CALL(*lottery_, slotsLeadership(new_epoch, keypair_))
      .WillOnce(Return(leadership_));

  babe_.runEpoch(epoch_, test_begin + 60ms);
  io_context_.run_for(150ms);
}

/**
 * @given BABE production, which is configured to the already finished slot in
 * the current epoch
 * @when launching it
 * @then it synchronizes successfully
 */
TEST_F(BabeTest, SyncSuccess) {}

/**
 * @given BABE production, which is configured to the already finished slot in
 * the previous epoch
 * @when launching it
 * @then it fails to synchronize
 */
TEST_F(BabeTest, BigDelay) {}
