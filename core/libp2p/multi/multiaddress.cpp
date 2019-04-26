/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "libp2p/multi/multiaddress.hpp"

#include <algorithm>
#include <numeric>
#include <stdexcept>

#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/split.hpp>
#include "libp2p/multi/converters/converter_utils.hpp"

using std::string_literals::operator""s;

namespace {

  /**
   * Find all occurrences of the string in other string
   * @param string to search in
   * @param substring to be searched for
   * @return vector with positions of all occurrences of that substring
   */
  std::vector<size_t> findSubstringOccurrences(std::string_view string,
                                               std::string_view substring) {
    std::vector<size_t> occurrences;
    auto occurrence = string.find(substring);
    while (occurrence != std::string_view::npos) {
      occurrences.push_back(occurrence);
      occurrence = string.find(substring, occurrence + substring.size());
    }
    return occurrences;
  }
}  // namespace

OUTCOME_CPP_DEFINE_CATEGORY(libp2p::multi, Multiaddress::Error, e) {
  using libp2p::multi::Multiaddress;
  switch (e) {
    case Multiaddress::Error::INVALID_INPUT:
      return "invalid multiaddress input";
    case Multiaddress::Error::INVALID_PROTOCOL_VALUE:
      return "protocol value can not be casted to T";
    case Multiaddress::Error::PROTOCOL_NOT_FOUND:
      return "multiaddress does not contain given protocol";
    default:
      return "unknown";
  }
}

namespace libp2p::multi {

  Multiaddress::FactoryResult Multiaddress::create(std::string_view address) {
    // convert string address to bytes and make sure they represent valid
    // address
    auto result = converters::multiaddrToBytes(address);
    if (!result) {
      return Error::INVALID_INPUT;
    }
    auto &&bytes = result.value();

    return Multiaddress{
        std::string{address},
        ByteBuffer{std::vector<uint8_t>{bytes.begin(), bytes.end()}}};
  }

  Multiaddress::FactoryResult Multiaddress::create(const ByteBuffer &bytes) {
    // convert bytes address to string and make sure it represents valid address
    auto conversion_res = converters::bytesToMultiaddrString(bytes);
    if (!conversion_res) {
      return Error::INVALID_INPUT;
    }

    std::string s = conversion_res.value();
    return Multiaddress{std::move(s), ByteBuffer{bytes}};
  }

  Multiaddress::Multiaddress(std::string &&address, ByteBuffer &&bytes)
      : stringified_address_{std::move(address)}, bytes_{std::move(bytes)} {
    calculatePeerId();
  }

  void Multiaddress::encapsulate(const Multiaddress &address) {
    // both string addresses begin and end with '/', which should be cut
    stringified_address_ += address.stringified_address_.substr(1);

    // but '/' is not encoded to bytes, we don't cut the vector
    const auto &other_bytes = address.bytes_.toVector();
    bytes_.put(std::vector<uint8_t>{other_bytes.begin(), other_bytes.end()});

    calculatePeerId();
  }

  bool Multiaddress::decapsulate(const Multiaddress &address) {
    auto str_pos = stringified_address_.rfind(address.stringified_address_);
    if (str_pos == std::string::npos) {
      return false;
    }
    // don't erase '/' in the end of the left address
    stringified_address_.erase(str_pos + 1);

    const auto &this_bytes = bytes_.toVector();
    const auto &other_bytes = address.bytes_.toVector();
    auto bytes_pos = std::search(this_bytes.begin(), this_bytes.end(),
                                 other_bytes.begin(), other_bytes.end());
    bytes_ = ByteBuffer{std::vector<uint8_t>{this_bytes.begin(), bytes_pos}};

    calculatePeerId();
    return true;
  }

  std::string_view Multiaddress::getStringAddress() const {
    return stringified_address_;
  }

  const Multiaddress::ByteBuffer &Multiaddress::getBytesAddress() const {
    return bytes_;
  }

  std::optional<std::string> Multiaddress::getPeerId() const {
    return peer_id_;
  }

  std::vector<std::string> Multiaddress::getValuesForProtocol(
      Protocol::Code proto) const {
    std::vector<std::string> values;
    auto protocol = ProtocolList::get(proto);
    if (protocol == nullptr) {
      return {};
    }
    auto proto_str = "/"s + std::string(protocol->name);
    auto proto_positions =
        findSubstringOccurrences(stringified_address_, proto_str);

    for (const auto &pos : proto_positions) {
      auto value_pos = stringified_address_.find_first_of('/', pos + 1) + 1;
      auto value_end = stringified_address_.find_first_of('/', value_pos);
      values.push_back(
          stringified_address_.substr(value_pos, value_end - value_pos));
    }

    return values;
  }

  std::list<Protocol> Multiaddress::getProtocols() const {
    std::string_view addr{stringified_address_};
    addr.remove_prefix(1);

    std::list<std::string> tokens;

    boost::algorithm::split(tokens, addr, boost::algorithm::is_any_of("/"));

    std::list<Protocol> protocols;
    for (auto &token : tokens) {
      auto p = ProtocolList::get(token);
      if (p != nullptr) {
        protocols.emplace_back(*p);
      }
    }
    return protocols;
  }

  std::list<std::pair<Protocol, std::string>>
  Multiaddress::getProtocolsWithValues() const {
    std::string_view addr{stringified_address_};
    addr.remove_prefix(1);
    if (addr.back() == '/') {
      addr.remove_suffix(1);
    }

    std::list<std::string> tokens;

    boost::algorithm::split(tokens, addr, boost::algorithm::is_any_of("/"));

    std::list<std::pair<Protocol, std::string>> pvs;
    for (auto &token : tokens) {
      auto p = ProtocolList::get(token);
      if (p != nullptr) {
        pvs.emplace_back(*p, "");
      } else {
        auto &s = pvs.back().second;
        if (!s.empty()) {
          s += "/";
        }
        s += token;
      }
    }
    return pvs;
  }

  void Multiaddress::calculatePeerId() {
    auto ipfsName =
        "/"s + std::string(ProtocolList::get(Protocol::Code::ipfs)->name);
    auto ipfs_beginning = stringified_address_.find(ipfsName);
    if (ipfs_beginning == std::string_view::npos) {
      peer_id_ = std::nullopt;
      return;
    }

    auto id_beginning = ipfs_beginning + 6;
    auto id_size = stringified_address_.find_first_of('/', id_beginning + 1)
        - id_beginning;

    peer_id_ = stringified_address_.substr(id_beginning, id_size);
  }

  bool Multiaddress::operator==(const Multiaddress &other) const {
    return this->stringified_address_ == other.stringified_address_
        && this->bytes_ == other.bytes_;
  }

  outcome::result<std::string> Multiaddress::getFirstValueForProtocol(
      Protocol::Code proto) const {
    // TODO(@warchant): refactor it to be more performant. this isn't best
    // solution
    auto vec = getValuesForProtocol(proto);
    if (vec.empty()) {
      return Error::PROTOCOL_NOT_FOUND;
    }

    return vec[0];
  }

  bool Multiaddress::operator<(const Multiaddress &other) const {
    return this->stringified_address_ < other.stringified_address_;
  }

}  // namespace libp2p::multi

size_t std::hash<libp2p::multi::Multiaddress>::operator()(
    const libp2p::multi::Multiaddress &x) const {
  return std::hash<std::string_view>()(x.getStringAddress());
}
