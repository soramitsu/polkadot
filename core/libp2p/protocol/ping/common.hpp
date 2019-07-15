/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef KAGOME_COMMON_HPP
#define KAGOME_COMMON_HPP

#include <cstdint>
#include <string_view>

namespace libp2p::protocol::detail {
  constexpr std::string_view kPingProto = "/ipfs/ping/1.0.0";

  constexpr uint8_t kPingMsgSize = 32;
}  // namespace libp2p::protocol::detail

#endif  // KAGOME_COMMON_HPP
