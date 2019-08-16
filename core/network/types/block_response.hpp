/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef KAGOME_BLOCK_RESPONSE_HPP
#define KAGOME_BLOCK_RESPONSE_HPP

#include <vector>

#include <boost/optional.hpp>
#include "common/buffer.hpp"
#include "primitives/block.hpp"
#include "primitives/common.hpp"
#include "primitives/justification.hpp"

namespace kagome::network {
  /**
   * Data, describing the requested block
   */
  struct BlockData {
    primitives::BlockHash hash;
    boost::optional<primitives::BlockHeader> header;
    boost::optional<primitives::BlockBody> body;
    boost::optional<common::Buffer> receipt;
    boost::optional<common::Buffer> message_queue;
    boost::optional<primitives::Justification> justification;
  };

  /**
   * @brief outputs object of type BlockData to stream
   * @tparam Stream output stream type
   * @param s stream reference
   * @param v value to output
   * @return reference to stream
   */
  template <class Stream,
            typename = std::enable_if_t<Stream::is_encoder_stream>>
  Stream &operator<<(Stream &s, const BlockData &v) {
    return s << v.hash << v.header << v.body << v.receipt << v.message_queue << v.justification;
  }

  /**
   * @brief decodes object of type BlockData from stream
   * @tparam Stream input stream type
   * @param s stream reference
   * @param v value to decode
   * @return reference to stream
   */
  template <class Stream,
            typename = std::enable_if_t<Stream::is_decoder_stream>>
  Stream &operator>>(Stream &s, BlockData &v) {
    return s >> v.hash >> v.header >> v.body >> v.receipt >> v.message_queue >> v.justification;
  }

  /**
   * Response to the BlockRequest
   */
  struct BlockResponse {
    primitives::BlockRequestId id;
    std::vector<BlockData> blocks;
  };

  /**
   * @brief outputs object of type BlockResponse to stream
   * @tparam Stream output stream type
   * @param s stream reference
   * @param v value to output
   * @return reference to stream
   */
  template <class Stream,
            typename = std::enable_if_t<Stream::is_encoder_stream>>
  Stream &operator<<(Stream &s, const BlockResponse &v) {
    return s << v.id << v.blocks;
  }

  /**
   * @brief decodes object of type BlockResponse from stream
   * @tparam Stream input stream type
   * @param s stream reference
   * @param v value to decode
   * @return reference to stream
   */
  template <class Stream,
            typename = std::enable_if_t<Stream::is_decoder_stream>>
  Stream &operator>>(Stream &s, BlockResponse &v) {
    return s >> v.id >> v.blocks;
  }
}  // namespace kagome::network

#endif  // KAGOME_BLOCK_RESPONSE_HPP
