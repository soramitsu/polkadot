/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef KAGOME_BIP39_TYPES_HPP
#define KAGOME_BIP39_TYPES_HPP

#include "common/blob.hpp"
#include "outcome/outcome.hpp"

namespace kagome::crypto::bip39 {
  namespace constants {
    constexpr size_t BIP39_SEED_LEN_512 = 64u;
  }  // namespace constants

  using Bip39Seed = common::Blob<constants::BIP39_SEED_LEN_512>;

  enum class Bip39ProviderError {
    INVALID_MNEMONIC = 1,
  };
}  // namespace kagome::crypto::bip39

OUTCOME_HPP_DECLARE_ERROR(kagome::crypto::bip39, Bip39ProviderError);

#endif  // KAGOME_BIP39_TYPES_HPP
