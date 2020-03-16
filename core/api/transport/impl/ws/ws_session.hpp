/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <chrono>
#include <cstdlib>
#include <memory>

#include <boost/beast/core/tcp_stream.hpp>
#include <boost/beast/websocket.hpp>

#include "api/transport/session.hpp"
#include "common/logger.hpp"

namespace kagome::api {

  class WsSession : public Session,
                    public std::enable_shared_from_this<WsSession> {
    using Socket = boost::asio::ip::tcp::socket;

    template <typename Body>
    using Request = boost::beast::http::request<Body>;

    //	template<typename Body>
    //	using Request = boost::beast::websocket::

    template <typename Body>
    using Response = boost::beast::http::response<Body>;

    using StringBody = boost::beast::http::string_body;

    template <class Body>
    using RequestParser = boost::beast::http::request_parser<Body>;

    using HttpField = boost::beast::http::field;

    using HttpError = boost::beast::http::error;
    using WsError = boost::beast::websocket::error;
    using Logger = common::Logger;

   public:
    struct Configuration {
      static constexpr size_t kDefaultRequestSize = 10000u;
      static constexpr Duration kDefaultTimeout = std::chrono::seconds(30);

      size_t max_request_size{kDefaultRequestSize};
      Duration operation_timeout{kDefaultTimeout};
    };

    ~WsSession() override = default;

    /**
     * @brief constructor
     * @param socket socket instance
     * @param config session configuration
     */
    WsSession(Socket socket, Configuration config);

    /**
     * @brief starts session
     */
    void start() override;

    /**
     * @brief sends response wrapped by http message
     * @param response message to send
     */
    void respond(std::string_view response) override;

   private:
    /**
     * @brief stops session
     */
    void stop();

    /**
     * @brief process http request, compose and execute response
     * @tparam Body request body type
     * @param request request
     */
    void handleRequest(std::string_view data);

    /**
     * @brief asynchronously read
     */
    void acyncRead();

    /**
     * @brief asynchronously write
     */
    void asyncWrite();

    /**
     * @brief connected callback
     */
    void onRun();

    /**
     * @brief handshake completion callback
     */
    void onAccept(boost::system::error_code ec);

    /**
     * @brief read completion callback
     */
    void onRead(boost::system::error_code ec, std::size_t size);

    /**
     * @brief write completion callback
     */
    void onWrite(boost::system::error_code ec, std::size_t bytes_transferred);

    /**
     * @brief reports error code and message
     * @param ec error code
     * @param message error message
     */
    void reportError(boost::system::error_code ec, std::string_view message);

    static constexpr std::string_view kServerName = "Kagome extrinsic api";

    Configuration config_;  ///< session configuration
    boost::beast::websocket::stream<boost::beast::tcp_stream> ws_;  ///< stream
    boost::beast::flat_buffer rbuffer_;  ///< read buffer
    boost::beast::flat_buffer wbuffer_;  ///< write buffer

    Logger logger_ =
        common::createLogger("websocket session");  ///< logger instance
  };

}  // namespace kagome::api
