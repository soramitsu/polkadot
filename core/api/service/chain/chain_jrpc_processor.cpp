/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "api/service/chain/chain_jrpc_processor.hpp"

#include "api/jrpc/jrpc_method.hpp"
#include "api/jrpc/value_converter.hpp"
#include "api/service/chain/requests/get_block_hash.hpp"
#include "api/service/chain/requests/get_header.hpp"
#include "api/service/chain/requests/subscribe_new_heads.hpp"

namespace kagome::api::chain {

  ChainJrpcProcessor::ChainJrpcProcessor(std::shared_ptr<JRpcServer> server,
                                         std::shared_ptr<ChainApi> api)
      : api_{std::move(api)}, server_{std::move(server)} {
    BOOST_ASSERT(api_ != nullptr);
    BOOST_ASSERT(server_ != nullptr);
  }

  template <typename Request>
  using Handler = Method<Request, ChainApi>;

  void ChainJrpcProcessor::registerHandlers() {
    server_->registerHandler("chain_getBlockHash",
                             Handler<request::GetBlockhash>(api_));

    server_->registerHandler("chain_getHeader",
                             Handler<request::GetHeader>(api_));

    server_->registerHandler("chain_subscribeNewHeads",
                             Handler<request::SubscribeNewHeads>(api_));
  }

}  // namespace kagome::api::chain
