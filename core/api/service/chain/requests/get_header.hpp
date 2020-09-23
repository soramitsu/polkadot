/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef KAGOME_CHAIN_GET_HEADER_HPP
#define KAGOME_CHAIN_GET_HEADER_HPP

#include "api/service/chain/requests/base_request.hpp"

namespace kagome::api::chain::request {

  struct GetHeader final : RequestType<int32_t, boost::optional<std::string>> {
    using BaseType = RequestType<int32_t, boost::optional<std::string>>;

    explicit GetHeader(std::shared_ptr<ChainApi> &api)
        : BaseType(api), api_(api) {}

    outcome::result<int32_t> execute() override {
      if (auto &param_0 = getParam<0>())
        return api_->getHeader(*param_0);

      return api_->getHeader();
    }

   private:
    std::shared_ptr<ChainApi> api_;
  };

}  // namespace kagome::api::chain::request

#endif  // KAGOME_CHAIN_GET_HEADER_HPP
