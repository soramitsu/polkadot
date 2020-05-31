/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "extensions/impl/crypto_extension.hpp"

#include <algorithm>
#include <exception>
#include <gsl/span>

#include "crypto/ed25519_provider.hpp"
#include "crypto/hasher.hpp"
#include "crypto/key_type.hpp"
#include "crypto/sr25519_provider.hpp"
#include "crypto/typed_key_storage.hpp"
#include "scale/scale.hpp"

namespace kagome::extensions {
  namespace sr25519_constants = crypto::constants::sr25519;
  namespace ed25519_constants = crypto::constants::ed25519;

  namespace {
    static constexpr uint32_t kGeneralSuccess = 0;
    static constexpr uint32_t kGeneralFail = 5;
  }  // namespace

  using crypto::PublicKey;

  CryptoExtension::CryptoExtension(
      std::shared_ptr<runtime::WasmMemory> memory,
      std::shared_ptr<crypto::SR25519Provider> sr25519_provider,
      std::shared_ptr<crypto::ED25519Provider> ed25519_provider,
      std::shared_ptr<crypto::Hasher> hasher,
      std::shared_ptr<crypto::storage::TypedKeyStorage> key_storage)
      : memory_(std::move(memory)),
        sr25519_provider_(std::move(sr25519_provider)),
        ed25519_provider_(std::move(ed25519_provider)),
        hasher_(std::move(hasher)),
        key_storage_(std::move(key_storage)),
        logger_{common::createLogger("CryptoExtension")} {
    BOOST_ASSERT(memory_ != nullptr);
    BOOST_ASSERT(sr25519_provider_ != nullptr);
    BOOST_ASSERT(ed25519_provider_ != nullptr);
    BOOST_ASSERT(hasher_ != nullptr);
    BOOST_ASSERT(logger_ != nullptr);
  }

  void CryptoExtension::ext_blake2_128(runtime::WasmPointer data,
                                       runtime::SizeType len,
                                       runtime::WasmPointer out_ptr) {
    const auto &buf = memory_->loadN(data, len);

    auto hash = hasher_->blake2b_128(buf);

    memory_->storeBuffer(out_ptr, common::Buffer(hash));
  }

  void CryptoExtension::ext_blake2_256(runtime::WasmPointer data,
                                       runtime::SizeType len,
                                       runtime::WasmPointer out_ptr) {
    const auto &buf = memory_->loadN(data, len);

    auto hash = hasher_->blake2b_256(buf);

    memory_->storeBuffer(out_ptr, common::Buffer(hash));
  }

  void CryptoExtension::ext_keccak_256(runtime::WasmPointer data,
                                       runtime::SizeType len,
                                       runtime::WasmPointer out_ptr) {
    const auto &buf = memory_->loadN(data, len);

    auto hash = hasher_->keccak_256(buf);

    memory_->storeBuffer(out_ptr, common::Buffer(hash));
  }

  runtime::SizeType CryptoExtension::ext_ed25519_verify(
      runtime::WasmPointer msg_data,
      runtime::SizeType msg_len,
      runtime::WasmPointer sig_data,
      runtime::WasmPointer pubkey_data) {
    // for some reason, 0 and 5 are used in the reference implementation, so
    // it's better to stick to them in ours, at least for now
    static constexpr uint32_t kVerifySuccess = 0;
    static constexpr uint32_t kVerifyFail = 5;

    auto msg = memory_->loadN(msg_data, msg_len);
    auto sig_bytes =
        memory_->loadN(sig_data, ed25519_constants::SIGNATURE_SIZE).toVector();

    auto signature_res = crypto::ED25519Signature::fromSpan(sig_bytes);
    if (!signature_res) {
      BOOST_UNREACHABLE_RETURN(kVerifyFail);
    }
    auto &&signature = signature_res.value();

    auto pubkey_bytes =
        memory_->loadN(pubkey_data, ed25519_constants::PUBKEY_SIZE).toVector();
    auto pubkey_res = crypto::ED25519PublicKey::fromSpan(pubkey_bytes);
    if (!pubkey_res) {
      BOOST_UNREACHABLE_RETURN(kVerifyFail);
    }
    auto &&pubkey = pubkey_res.value();

    auto result = ed25519_provider_->verify(signature, msg, pubkey);
    auto is_succeeded = result && result.value();

    return is_succeeded ? kVerifySuccess : kVerifyFail;
  }

  runtime::SizeType CryptoExtension::ext_sr25519_verify(
      runtime::WasmPointer msg_data,
      runtime::SizeType msg_len,
      runtime::WasmPointer sig_data,
      runtime::WasmPointer pubkey_data) {
    // for some reason, 0 and 5 are used in the reference implementation, so
    // it's better to stick to them in ours, at least for now
    static constexpr uint32_t kVerifySuccess = 0;
    static constexpr uint32_t kVerifyFail = 5;

    auto msg = memory_->loadN(msg_data, msg_len);
    auto signature_buffer =
        memory_->loadN(sig_data, sr25519_constants::SIGNATURE_SIZE);

    auto pubkey_buffer =
        memory_->loadN(pubkey_data, sr25519_constants::PUBLIC_SIZE);
    auto key_res = crypto::SR25519PublicKey::fromSpan(pubkey_buffer);
    if (!key_res) {
      BOOST_UNREACHABLE_RETURN(kVerifyFail);
    }
    auto &&key = key_res.value();

    crypto::SR25519Signature signature{};
    std::copy_n(signature_buffer.begin(),
                sr25519_constants::SIGNATURE_SIZE,
                signature.begin());

    auto res = sr25519_provider_->verify(signature, msg, key);
    bool is_succeeded = res && res.value();

    return is_succeeded ? kVerifySuccess : kVerifyFail;
  }

  void CryptoExtension::ext_twox_64(runtime::WasmPointer data,
                                    runtime::SizeType len,
                                    runtime::WasmPointer out_ptr) {
    const auto &buf = memory_->loadN(data, len);

    auto hash = hasher_->twox_64(buf);
    logger_->trace("twox64. Data: {}, Data hex: {}, hash: {}",
                   buf.data(),
                   buf.toHex(),
                   hash.toHex());

    memory_->storeBuffer(out_ptr, common::Buffer(hash));
  }

  void CryptoExtension::ext_twox_128(runtime::WasmPointer data,
                                     runtime::SizeType len,
                                     runtime::WasmPointer out_ptr) {
    const auto &buf = memory_->loadN(data, len);

    auto hash = hasher_->twox_128(buf);
    logger_->trace("twox128. Data: {}, Data hex: {}, hash: {}",
                   buf.data(),
                   buf.toHex(),
                   hash.toHex());

    memory_->storeBuffer(out_ptr, common::Buffer(hash));
  }

  void CryptoExtension::ext_twox_256(runtime::WasmPointer data,
                                     runtime::SizeType len,
                                     runtime::WasmPointer out_ptr) {
    const auto &buf = memory_->loadN(data, len);

    auto hash = hasher_->twox_256(buf);

    memory_->storeBuffer(out_ptr, common::Buffer(hash));
  }

  runtime::SizeType CryptoExtension::ext_ed25519_public_keys(
      runtime::SizeType key_type, runtime::WasmPointer out_ptr) {
    auto &&key_type_id = crypto::decodeKeyTypeId(key_type);
    if (!key_type_id) {
      logger_->error("failed to decode key type: {}",
                     key_type_id.error().message());
      return kGeneralFail;
    }

    auto &&pairs = key_storage_->getEdKeys(key_type_id.value());
    constexpr size_t kKeySize = 32ul;
    for (auto &p : pairs) {
      if (p.publicKey.data.size() != kKeySize) {
        logger_->error("wrong key size");
        return kGeneralFail;
      }
      if (p.publicKey.type != PublicKey::Type::Ed25519) {
        logger_->error("wrong key type");
        return kGeneralFail;
      }
    }

    using KeyBuffer = common::Blob<kKeySize>;
    std::vector<KeyBuffer> public_keys;
    public_keys.reserve(pairs.size());
    std::transform(pairs.begin(),
                   pairs.end(),
                   std::back_inserter(public_keys),
                   [](const auto &key_pair) -> KeyBuffer {
                     auto data = key_pair.publicKey.data;
                     return KeyBuffer::fromSpan(gsl::make_span(data)).value();
                   });
    auto &&encoded = scale::encode(public_keys);
    if (!encoded) {
      logger_->error("failed to scale-encode vector of public keys: {}",
                     encoded.error().message());
      return kGeneralFail;
    }

    common::Buffer buffer(std::move(encoded.value()));
    memory_->storeBuffer(out_ptr, buffer);
    return kGeneralSuccess;
  }

  runtime::SizeType CryptoExtension::ext_ed25519_generate(
      runtime::SizeType key_type,
      runtime::WasmPointer seed_data,
      runtime::SizeType seed_len,
      runtime::WasmPointer out_ptr) {
    auto &&key_type_id = crypto::decodeKeyTypeId(key_type);
    if (!key_type_id) {
      logger_->error("failed to decode key type: {}",
                     key_type_id.error().message());
      return kGeneralFail;
    }

    auto &&pair = ed25519_provider_->generateKeypair();

    // TODO (yuraz) implement
    return {};
  }

  runtime::SizeType CryptoExtension::ext_ed25519_sign(
      runtime::SizeType key_type,
      runtime::WasmPointer key,
      runtime::WasmPointer msg_data,
      runtime::SizeType msg_len,
      runtime::WasmPointer out_ptr) {
    auto &&key_type_id = crypto::decodeKeyTypeId(key_type);
    if (!key_type_id) {
      logger_->error("failed to decode key type: {}",
                     key_type_id.error().message());
      return kGeneralFail;
    }
    // TODO (yuraz) implement
    return {};
  }

  runtime::SizeType CryptoExtension::ext_sr25519_public_keys(
      runtime::SizeType key_type, runtime::WasmPointer out_ptr) {
    auto &&key_type_id = crypto::decodeKeyTypeId(key_type);
    if (!key_type_id) {
      logger_->error("failed to decode key type: {}",
                     key_type_id.error().message());
      return kGeneralFail;
    }
    auto &&pairs = key_storage_->getSrKeys(key_type_id.value());
    constexpr size_t kKeySize = 32ul;
    for (auto &p : pairs) {
      if (p.publicKey.data.size() != kKeySize) {
        logger_->error("wrong key size");
        return kGeneralFail;
      }
      if (p.publicKey.type != PublicKey::Type::Ed25519) {
        logger_->error("wrong key type");
        return kGeneralFail;
      }
    }

    using KeyBuffer = common::Blob<kKeySize>;
    std::vector<KeyBuffer> public_keys;
    public_keys.reserve(pairs.size());
    std::transform(pairs.begin(),
                   pairs.end(),
                   std::back_inserter(public_keys),
                   [](const auto &key_pair) -> KeyBuffer {
                     auto data = key_pair.publicKey.data;
                     return KeyBuffer::fromSpan(gsl::make_span(data)).value();
                   });
    auto &&encoded = scale::encode(public_keys);
    if (!encoded) {
      logger_->error("failed to scale-encode vector of public keys: {}",
                     encoded.error().message());
      return kGeneralFail;
    }

    common::Buffer buffer(std::move(encoded.value()));
    memory_->storeBuffer(out_ptr, buffer);
    return kGeneralSuccess;
  }

  runtime::SizeType CryptoExtension::ext_sr25519_generate(
      runtime::SizeType key_type,
      runtime::WasmPointer seed_data,
      runtime::SizeType seed_len,
      runtime::WasmPointer out_ptr) {
    auto &&key_type_id = crypto::decodeKeyTypeId(key_type);
    if (!key_type_id) {
      logger_->error("failed to decode key type: {}",
                     key_type_id.error().message());
      return kGeneralFail;
    }
    // TODO (yuraz) implement
    return {};
  }

  runtime::SizeType CryptoExtension::ext_sr25519_sign(
      runtime::SizeType key_type,
      runtime::WasmPointer key,
      runtime::WasmPointer msg_data,
      runtime::SizeType msg_len,
      runtime::WasmPointer out_ptr) {
    auto &&key_type_id = crypto::decodeKeyTypeId(key_type);
    if (!key_type_id) {
      logger_->error("failed to decode key type: {}",
                     key_type_id.error().message());
      return kGeneralFail;
    }
    // TODO (yuraz) implement
    return {};
  }
}  // namespace kagome::extensions
