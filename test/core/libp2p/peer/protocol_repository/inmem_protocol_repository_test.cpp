/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include <gtest/gtest.h>

#include "common/buffer.hpp"
#include "libp2p/peer/errors.hpp"
#include "libp2p/peer/protocol_repository.hpp"
#include "libp2p/peer/protocol_repository/inmem_protocol_repository.hpp"
#include "testutil/outcome.hpp"

using namespace libp2p::peer;
using namespace libp2p::multi;
using kagome::common::Buffer;

struct InmemProtocolRepository_Test : public ::testing::Test {
  std::unique_ptr<ProtocolRepository> db;

  void SetUp() override {
    db = std::make_unique<InmemProtocolRepository>();
  }

  const Protocol s1 = "/bittorrent.org/1.0";
  const Protocol s2 = "/ipfs/1.0";

  const PeerId p1 = Multihash::createFromHex("1203020304").value();
  const PeerId p2 = Multihash::createFromHex("1203ffffff").value();

  template <typename... T>
  std::vector<Protocol> vec(T &&... arg) {
    return std::vector<Protocol>{arg...};
  }

  template <typename... T>
  std::list<Protocol> lst(T &&... arg) {
    return std::list<Protocol>{arg...};
  }
};

/**
 * @given empty repository
 * @when add two protocols
 * @then two protocols added
 */
TEST_F(InmemProtocolRepository_Test, Add) {
  EXPECT_OUTCOME_TRUE_1(db->addProtocols(p1, vec(s1, s2)));
  {
    EXPECT_OUTCOME_TRUE_2(v, db->getProtocols(p1));
    EXPECT_EQ(v.size(), 2);

    auto r = db->getProtocols(p2);
    EXPECT_FALSE(r);
    EXPECT_EQ(r.error().value(), (int)PeerError::NotFound);
  }
}

/**
 * @given two protocols are added to p1, p2 has no protocols
 * @when collect garbage on empty peers
 * @then they are evicted
 */
TEST_F(InmemProtocolRepository_Test, CollectGarbage) {
  EXPECT_OUTCOME_TRUE_1(db->addProtocols(p1, vec(s1, s2)));
  EXPECT_OUTCOME_TRUE_1(db->addProtocols(p2, vec()));

  // no effect
  db->collectGarbage();

  {
    EXPECT_OUTCOME_TRUE_2(v, db->getProtocols(p1));
    EXPECT_EQ(v.size(), 2);

    auto r = db->getProtocols(p2);
    EXPECT_FALSE(r);
    EXPECT_EQ(r.error().value(), (int)PeerError::NotFound);
  }

  // clear p1. now p1 has 0 protocols
  db->clear(p1);

  // evicts p1 and p2
  db->collectGarbage();

  {
    for (const auto &it : {p1, p2}) {
      auto r = db->getProtocols(it);
      EXPECT_FALSE(r);
      EXPECT_EQ(r.error().value(), (int)PeerError::NotFound);
    }
  }
}

/**
 * @given database with s1 and s2
 * @when supportsProtocols executed with different patterns
 * @then expected protocols are returned
 */
TEST_F(InmemProtocolRepository_Test, Supports) {
  EXPECT_OUTCOME_TRUE_1(db->addProtocols(p1, vec(s1, s2)));

  // one of
  {
    EXPECT_OUTCOME_TRUE_2(v, db->supportsProtocols(p1, vec(s1)));
    EXPECT_EQ(v, lst(s1));
  }

  // forward order
  {
    EXPECT_OUTCOME_TRUE_2(v, db->supportsProtocols(p1, vec(s1, s2)));
    EXPECT_EQ(v, lst(s1, s2));
  }

  // reverse order
  {
    EXPECT_OUTCOME_TRUE_2(v, db->supportsProtocols(p1, vec(s2, s1)));
    EXPECT_EQ(v, lst(s1, s2));
  }

  // non existing
  {
    EXPECT_OUTCOME_TRUE_1(db->removeProtocols(p1, vec(s1)));
    EXPECT_OUTCOME_TRUE_2(v, db->supportsProtocols(p1, vec(s1, s2)));
    EXPECT_EQ(v, lst(s2));
  }
}

/**
 * @given p1 with s1 and s2
 * @when remove s1 and get all protocols
 * @then protocol s1 is removed
 */
TEST_F(InmemProtocolRepository_Test, Remove) {
  EXPECT_OUTCOME_TRUE_1(db->addProtocols(p1, vec(s1, s2)));
  EXPECT_OUTCOME_TRUE_1(db->removeProtocols(p1, vec(s1)));
  EXPECT_OUTCOME_TRUE_2(v, db->getProtocols(p1));
  EXPECT_EQ(v, lst(s2));
}

/**
 * @given peer p1 with s2
 * @when remove non-existing protocol s1
 * @then get no error
 */
TEST_F(InmemProtocolRepository_Test, RemoveNonExisting) {
  EXPECT_OUTCOME_TRUE_1(db->addProtocols(p1, vec(s2)));
  EXPECT_OUTCOME_TRUE_1(db->removeProtocols(p1, vec(s1)));
  EXPECT_OUTCOME_TRUE_2(v, db->getProtocols(p1));
  EXPECT_EQ(v.size(), 1);
}
