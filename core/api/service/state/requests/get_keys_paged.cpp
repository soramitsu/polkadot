/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "api/service/state/requests/get_keys_paged.hpp"

#include "scale/scale.hpp"

namespace kagome::api::state::request {

  outcome::result<void> GetKeysPaged::init(
      const jsonrpc::Request::Parameters &params) {
    if (params.size() > 4 or params.size() == 1 or params.empty()) {
      throw jsonrpc::InvalidParametersFault("Incorrect number of params");
    }
    auto &param0 = params[0];
    if (not param0.IsString()) {
      throw jsonrpc::InvalidParametersFault(
          "Parameter '[prefix]' must be a hex string");
    }
    auto &&prefix_str = param0.AsString();
    if (prefix_str == "null") {
      prefix_ = common::Buffer();
    } else {
      OUTCOME_TRY(key, common::unhexWith0x(prefix_str));
      prefix_ = common::Buffer(std::move(key));
    }

    if (not params[1].IsInteger32()) {
      throw jsonrpc::InvalidParametersFault(
          "Parameter '[key_amount]' must be a uint32_t");
    }

    keys_amount_ = params[1].AsInteger32();
    if (params.size() == 2) {
      return outcome::success();
    }

    // process prev_key param
    if (not params[2].IsString()) {
      throw jsonrpc::InvalidParametersFault(
          "Parameter '[prev_key]' must be a hex string of encoded optional of "
          "bytes");
    }
    auto prev_key_str = params[2].AsString();
    OUTCOME_TRY(prev_key, common::unhexWith0x(prev_key_str));
    prev_key_ = common::Buffer{prev_key};

    if (params.size() == 3) {
      return outcome::success();
    }

    // process at param
    if (not params[3].IsString()) {
      throw jsonrpc::InvalidParametersFault(
          "Parameter '[at]' must be a hex string of encoded optional of bytes");
    }
    auto at_str = params[3].AsString();
    OUTCOME_TRY(at_span, common::unhexWith0x(at_str));
    OUTCOME_TRY(at, primitives::BlockHash::fromSpan(at_span));
    at_ = at;

    return outcome::success();
  }

  outcome::result<std::vector<common::Buffer>> GetKeysPaged::execute() {
    return api_->getKeysPaged(prefix_, keys_amount_, prev_key_, at_);
  }

}  // namespace kagome::api::state::request
