/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef KAGOME_OPTIONAL_HPP
#define KAGOME_OPTIONAL_HPP

#include "common/buffer.hpp"
#include "scale/scale_error.hpp"
#include "scale/type_decoder.hpp"
#include "scale/type_encoder.hpp"
#include "scale/types.hpp"

namespace kagome::scale::optional {
  /**
   * @brief encodeOptional function encodes optional value
   * @tparam T optional value content type
   * @param optional value
   * @return encoded optional value or error
   */
  template <class T>
  outcome::result<void> encodeOptional(const std::optional<T> &optional,
                                       common::Buffer &out) {
    if (!optional.has_value()) {
      out.putUint8(0);
      return outcome::success();
    }

    TypeEncoder<T> codec{};
    common::Buffer tmp;
    OUTCOME_TRY(codec.encode(*optional, tmp));

    out.putUint8(1);
    out.putBuffer(tmp);

    return outcome::success();
  }

  /**
   * @brief decodeOptional function decodes optional value from stream
   * @tparam T optional value content type
   * @param stream source stream
   * @return decoded optional value or error
   */
  template <class T>
  outcome::result<std::optional<T>> decodeOptional(common::ByteStream &stream) {
    auto flag = stream.nextByte();
    if (!flag.has_value()) {
      return outcome::failure(DecodeError::NOT_ENOUGH_DATA);
    }

    if (*flag != 1) {
      return std::nullopt;
    }

    TypeDecoder<T> codec{};
    OUTCOME_TRY(result, codec.decode(stream));

    return std::optional<T>{result};
  }

  /**
   * @brief specialization of encodeOptional function for optional bool
   * @param optional bool value
   * @return encoded value
   */
  template <>
  outcome::result<void> encodeOptional<bool>(
      const std::optional<bool> &optional, common::Buffer &out) {
    uint8_t result = 2;  // true

    if (!optional.has_value()) {  // none
      result = 0;
    } else if (!*optional) {  // false
      result = 1;
    }

    out.putUint8(result);
    return outcome::success();
  }

  /**
   * @brief specialization of decodeOptional function for optional bool
   * @param optional bool value
   * @return decoded value or error
   */
  template <>
  outcome::result<std::optional<bool>> decodeOptional(
      common::ByteStream &stream) {
    auto byte = stream.nextByte();
    if (!byte.has_value()) {
      return outcome::failure(DecodeError::NOT_ENOUGH_DATA);
    }

    switch (*byte) {
      case 0:
        return outcome::success(std::nullopt);
      case 1:
        return std::optional<bool>{false};
      case 2:
        return std::optional<bool>{true};
      default:
        break;
    }

    return outcome::failure(DecodeError::UNEXPECTED_VALUE);
  }

}  // namespace kagome::scale::optional

#endif  // KAGOME_OPTIONAL_HPP
