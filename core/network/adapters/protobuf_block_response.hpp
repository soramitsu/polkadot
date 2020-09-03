/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef KAGOME_ADAPTERS_PROTOBUF_BLOCK_RESPONSE
#define KAGOME_ADAPTERS_PROTOBUF_BLOCK_RESPONSE

#include "network/adapters/protobuf.hpp"

namespace kagome::network {

  /*
  template <>
  struct ProtobufMessageAdapter<network::BlocksRequest> {
    static size_t size(const network::BlocksRequest &t) {
      return 0;
    }
    static std::vector<uint8_t>::iterator write(
        const network::BlocksRequest &t,
        std::vector<uint8_t> &out,
        std::vector<uint8_t>::iterator loaded) {
      api::v1::BlockRequest msg;
      msg.set_fields(LE_BE_SWAP32(t.fields.attributes.to_ulong()));

      if (t.max) msg.set_max_blocks(*t.max);
      if (t.to) msg.set_to_block(t.to->toHex());

      msg.set_direction(static_cast<api::v1::Direction>(t.direction));
      kagome::visit_in_place(
          t.from,
          [&](const primitives::BlockHash &v) { msg.set_hash(v.toHex()); },
          [&](const primitives::BlockNumber &v) {
            msg.set_number(std::to_string(v));
          });

      const size_t distance_was = std::distance(out.begin(), loaded);
      const size_t was_size = out.size();

      out.resize(was_size + msg.ByteSizeLong());
      msg.SerializeToArray(&out[was_size], msg.ByteSizeLong());

      auto res_it = out.begin();
      std::advance(res_it, std::min(distance_was, was_size));
      return std::move(res_it);
    }

    static libp2p::outcome::result<std::vector<uint8_t>::const_iterator> read(
        network::BlocksRequest &out,
        const std::vector<uint8_t> &src,
        std::vector<uint8_t>::const_iterator from) {
      const auto remains = src.size() - std::distance(src.begin(), from);
      assert(remains > 0);

      api::v1::BlockRequest msg;
      if (!msg.ParseFromArray(from.base(), remains))
        return outcome::failure(boost::system::error_code{});

      out.fields.load(LE_BE_SWAP32(msg.fields()));
      out.direction = static_cast<decltype(out.direction)>(msg.direction());

      switch (msg.from_block_case()) {
        case msg.kHash: {
          OUTCOME_TRY(data, primitives::BlockHash::fromHex(msg.hash()));
          out.from = data;
        } break;

        case msg.kNumber: {
          out.from = std::stoull(msg.number());
        } break;

        default:
          return outcome::failure(boost::system::error_code{});
      }

      OUTCOME_TRY(to_block, primitives::BlockHash::fromHex(msg.to_block()));
      out.to = to_block;
      out.max = msg.max_blocks();

      return outcome::success();
    }
  };
*/
}  // namespace kagome::network

#endif  // KAGOME_ADAPTERS_PROTOBUF_BLOCK_RESPONSE