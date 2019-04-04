/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef KAGOME_SCALE_COMPACT_HPP
#define KAGOME_SCALE_COMPACT_HPP

#include <optional>
#include <outcome/outcome.hpp>

#include "common/buffer.hpp"
#include "common/result.hpp"
#include "scale/types.hpp"

namespace kagome::common::scale::compact {
  /**
   * @brief compact-encodes BigInteger
   * @param value source BigInteger value
   * @return byte array result or error
   */
  outcome::result<void> encodeInteger(const BigInteger &value, Buffer &out);

  /**
   * @brief function decodes compact-encoded integer
   * @param stream source stream
   * @return decoded BigInteger or error
   */
  outcome::result<BigInteger> decodeInteger(Stream &stream);
}  // namespace kagome::common::scale::compact

#endif  // KAGOME_SCALE_HPP
