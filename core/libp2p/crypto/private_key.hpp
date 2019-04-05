/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef KAGOME_PRIVATE_KEY_HPP
#define KAGOME_PRIVATE_KEY_HPP

#include "libp2p/crypto/key.hpp"

namespace libp2p::crypto {
  class PublicKey;
  /**
   * Represents private key
   */
  class PrivateKey : public Key {
   public:
    virtual ~PrivateKey() = default;
    /**
     * Get a public key, derived from this private one
     * @return a public key
     */
    virtual std::shared_ptr<PublicKey> publicKey() const = 0;
  };
}  // namespace libp2p::crypto

#endif  // KAGOME_PRIVATE_KEY_HPP
