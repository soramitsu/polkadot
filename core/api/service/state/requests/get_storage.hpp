/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef KAGOME_API_REQUEST_GET_STORAGE
#define KAGOME_API_REQUEST_GET_STORAGE

#include <jsonrpc-lean/request.h>

#include <boost/optional.hpp>
#include <common/buffer.hpp>
#include <outcome/outcome.hpp>
#include <primitives/block_id.hpp>

#include "api/service/state/state_api.hpp"

namespace kagome::api::state::request {

  class GetStorage final {
   public:
    GetStorage(std::shared_ptr<StateApi> api) : api_(std::move(api)){};

    outcome::result<void> init(const jsonrpc::Request::Parameters &params);

    outcome::result<common::Buffer> execute();

   private:
    std::shared_ptr<StateApi> api_;
    common::Buffer key_;
    boost::optional<kagome::primitives::BlockHash> at_;
  };

}  // namespace kagome::api::state::request

#endif  // KAGOME_STATE_JRPC_PROCESSOR_HPP
