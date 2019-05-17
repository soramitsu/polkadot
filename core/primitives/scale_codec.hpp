/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef KAGOME_SCALE_PRIMITIVES_CODEC_HPP
#define KAGOME_SCALE_PRIMITIVES_CODEC_HPP

#include <outcome/outcome.hpp>
#include "common/buffer.hpp"
#include "common/byte_stream.hpp"
#include "primitives/block_id.hpp"
#include "primitives/common.hpp"
#include "primitives/digest.hpp"
#include "primitives/parachain_host.hpp"
#include "primitives/scheduled_change.hpp"
#include "primitives/transaction_validity.hpp"
#include "scale/types.hpp"

namespace kagome::primitives {
  class Block;  ///< forward declaration of class Block

  class BlockHeader;  ///< forward declaration of class BlockHeader

  class Extrinsic;  ///< forward declaration of class Extrinsic

  class Version;  ///< forward declaration of class Version

  /**
   * class ScaleCodec is an interface declaring methods
   * for encoding and decoding primitives
   */
  class ScaleCodec {
   protected:
    using Stream = common::ByteStream;
    using Buffer = common::Buffer;

   public:
    /**
     * @brief virtual destuctor
     */
    virtual ~ScaleCodec() = default;

    /**
     * @brief scale-encodes Block instance
     * @param block value which should be encoded
     * @return scale-encoded value or error
     */
    virtual outcome::result<Buffer> encodeBlock(const Block &block) const = 0;

    /**
     * @brief decodes scale-encoded block from stream
     * @param stream source stream containing encoded bytes
     * @return decoded block or error
     */
    virtual outcome::result<Block> decodeBlock(Stream &stream) const = 0;

    /**
     * @brief scale-encodes BlockHeader instance
     * @param block_header value which should be encoded
     * @return scale-encoded value or error
     */
    virtual outcome::result<Buffer> encodeBlockHeader(
        const BlockHeader &block_header) const = 0;

    /**
     * @brief decodes scale-encoded block header
     * @param stream source stream containing encoded bytes
     * @return decoded block header or error
     */
    virtual outcome::result<BlockHeader> decodeBlockHeader(
        Stream &stream) const = 0;

    /**
     * @brief scale-encodes Extrinsic instance
     * @param extrinsic value which should be encoded
     * @return scale-encoded value or error
     */
    virtual outcome::result<Buffer> encodeExtrinsic(
        const Extrinsic &extrinsic) const = 0;

    /**
     * @brief decodes scale-encoded extrinsic
     * @param stream source stream containing encoded bytes
     * @return decoded extrinsic or error
     */
    virtual outcome::result<Extrinsic> decodeExtrinsic(
        Stream &stream) const = 0;

    /**
     * @brief scale-encodes Version instance
     * @param version value which should be encoded
     * @return scale-encoded value or error
     */
    virtual outcome::result<Buffer> encodeVersion(
        const Version &version) const = 0;

    /**
     * @brief decodes scale-encoded Version instance
     * @param stream source stream containing encoded bytes
     * @return decoded Version instance or error
     */
    virtual outcome::result<Version> decodeVersion(Stream &stream) const = 0;

    /**
     * @brief scale-encodes block id
     * @param blockId value which should be encoded
     * @return scale-encoded value or error
     */
    virtual outcome::result<Buffer> encodeBlockId(
        const BlockId &blockId) const = 0;

    /**
     * @brief decodes scale-encoded BlockId instance
     * @param stream source stream containing encoded bytes
     * @return decoded BlockId instance or error
     */
    virtual outcome::result<BlockId> decodeBlockId(Stream &stream) const = 0;

    /**
     * @brief scale-encodes TransactionValidity instance
     * @param transactionValidity value which should be encoded
     * @return encoded value or error
     */
    virtual outcome::result<Buffer> encodeTransactionValidity(
        const TransactionValidity &transactionValidity) const = 0;

    /**
     * @brief decodes scale-encoded TransactionValidity instance
     * @param stream source stream containing encoded bytes
     * @return decoded TransactionValidity instance or error
     */
    virtual outcome::result<TransactionValidity> decodeTransactionValidity(
        Stream &stream) const = 0;

    /**
     * @brief scale-encodes AuthorityIds instance
     * @param ids value which should be encoded
     * @return encoded value or error
     */
    virtual outcome::result<Buffer> encodeAuthorityIds(
        const std::vector<AuthorityId> &ids) const = 0;

    /**
     * @brief decodes scale-encoded
     * @param stream source stream containing encoded bytes
     * @return decoded Vector of AuthorityIds or error
     */
    virtual outcome::result<std::vector<AuthorityId>> decodeAuthorityIds(
        Stream &stream) const = 0;

    /**
     * @brief scale-encodes DutyRoster instance
     * @param ids value which should be encoded
     * @return encoded value or error
     */
    virtual outcome::result<Buffer> encodeDutyRoster(
        const parachain::DutyRoster &duty_roster) const = 0;

    /**
     * @brief decodes DutyRoster item
     * @param stream source stream containing encoded bytes
     * @return decoded DutyRoster item
     */
    virtual outcome::result<parachain::DutyRoster> decodeDutyRoster(
        Stream &stream) const = 0;

    /**
     * @brief scale-encodes Digest instance
     * @param digest value which should be encoded
     * @return encoded value or error
     */
    virtual outcome::result<Buffer> encodeDigest(
        const Digest &digest) const = 0;

    /**
     * @brief decodes Digest item
     * @param stream source stream containing encoded bytes
     * @return decoded DutyRoster item
     */
    virtual outcome::result<Digest> decodeDigest(Stream &stream) const = 0;

    /**
     * @brief scale-encodes ScheduledChange instance
     * @param value ScheduledChange instance which should be encoded
     * @return encoded value or error
     */
    virtual outcome::result<Buffer> encodeScheduledChange(
        const ScheduledChange &value) const = 0;

    /**
     * @brief decodes ScheduledChange item
     * @param stream source stream containing encoded bytes
     * @return decoded ScheduledChange item
     */
    virtual outcome::result<std::optional<ScheduledChange>>
    decodeScheduledChange(Stream &stream) const = 0;

    /**
     * encodeForcedChange is meaningless and not required
     */

    /**
     * @brief decodes forced_change function result type
     * @param stream source stream containing encoded bytes
     * @return decoded result
     */
    virtual outcome::result<std::optional<primitives::ForcedChangeType>>
    decodeForcedChange(Stream &stream) const = 0;

    /**
     * @brief decodes collection of WeightedAuthority items
     * @param stream source stream containing encoded bytes
     * @return decoded result
     */
    virtual outcome::result<std::vector<primitives::WeightedAuthority>>
    decodeGrandpaAuthorities(Stream &stream) const = 0;
  };
}  // namespace kagome::primitives

#endif  // KAGOME_SCALE_PRIMITIVES_CODEC_HPP
