/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "libp2p/multi/converters/ip_v4_converter.hpp"

#include <string>

#include <boost/asio/ip/address_v4.hpp>
#include <outcome/outcome.hpp>
#include "common/hexutil.hpp"
#include "libp2p/multi/converters/conversion_error.hpp"
#include "libp2p/multi/utils/protocol_list.hpp"

namespace libp2p::multi::converters {

  auto IPv4Converter::addressToHex(std::string_view addr)
      -> outcome::result<std::string> {
    boost::system::error_code ec;
    auto address = boost::asio::ip::make_address_v4(addr, ec);
    if (ec) {
      return ec;  // error
    }
    uint64_t iip = address.to_ulong();
    auto hex = kagome::common::int_to_hex(iip);
    hex = std::string(8 - hex.length(), '0') + hex;
    return hex;
  }

}  // namespace libp2p::multi::converters
