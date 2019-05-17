/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "libp2p/transport/impl/transport_parser.hpp"
#include <gtest/gtest.h>
#include <outcome/outcome.hpp>
#include <testutil/outcome.hpp>

using libp2p::multi::Multiaddress;
using libp2p::multi::Protocol;
using libp2p::transport::TransportParser;

using ::testing::Test;
using ::testing::TestWithParam;

struct TransportParserTest : public TestWithParam<Multiaddress> {};

/**
 * @given Transport parser and a multiaddress
 * @when parse the address
 * @then the chosen protocols by parser are the protocols of the multiaddress
 */
TEST_P(TransportParserTest, ParseSuccessfully) {
  auto r_ok = TransportParser::parse(GetParam());
  std::vector<Protocol::Code> proto_codes;
  auto multiaddr_protos = GetParam().getProtocols();
  auto callback = [](Protocol const &proto) { return proto.code; };
  std::transform(multiaddr_protos.begin(), multiaddr_protos.end(),
                 std::back_inserter(proto_codes), callback);
  ASSERT_EQ(r_ok.value().chosen_protos_, proto_codes);
}

auto addresses = {Multiaddress::create("/ip4/127.0.0.1/tcp/5050").value()};
INSTANTIATE_TEST_CASE_P(TestSupported, TransportParserTest,
                        ::testing::ValuesIn(addresses));

/**
 * @given Transport parser and a multiaddress
 * @when parse the address
 * @then the parse result variant contains information corresponding to the
 * content of the multiaddress
 */
TEST_F(TransportParserTest, CorrectAlternative) {
  auto r = TransportParser::parse(
      Multiaddress::create("/ip4/127.0.0.1/tcp/5050").value());

  using IpTcp = std::pair<TransportParser::IpAddress, uint16_t>;

  // TODO(Harrm) Change to std::visit when CI for Mac OS X is upgraded
  ASSERT_TRUE(std::get_if<IpTcp>(&r.value().data_) != nullptr);
}
