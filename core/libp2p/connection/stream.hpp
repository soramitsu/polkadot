/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef KAGOME_CONNECTION_STREAM_HPP
#define KAGOME_CONNECTION_STREAM_HPP

#include <outcome/outcome.hpp>
#include "libp2p/basic/readwritecloser.hpp"

namespace libp2p::connection {

  /**
   * Stream over some connection, allowing to write/read to/from that connection
   * @note the user MUST WAIT for the completion of method from this list
   * before calling another method from this list:
   *    - write
   *    - writeSome
   *    - close
   *    - adjustWindowSize
   *    - reset
   * Also, 'read' & 'readSome' are in another tuple. This behaviour results in
   * possibility to read and write simultaneously, but double read or write is
   * forbidden
   */
  struct Stream : public basic::ReadWriteCloser {
    using Handler = void(std::shared_ptr<Stream>);

    ~Stream() override = default;

    /**
     * Check, if this stream is closed from this side of the connection and
     * thus cannot be read from
     * @return true, if stream cannot be read from, false otherwise
     */
    virtual bool isClosedForRead() const = 0;

    /**
     * Check, if this stream is closed from the other side of the connection and
     * thus cannot be written to
     * @return true, if stream cannot be written to, false otherwise
     */
    virtual bool isClosedForWrite() const = 0;

    using CloseCallbackFunc = std::function<void(outcome::result<void>)>;

    /**
     * Close a stream, indicating we are not going to write to it anymore; the
     * other side, however, can write to it, if it was not closed from there
     * before
     * @param cb to be called, when the stream is closed, or error happens
     */
    virtual void close(CloseCallbackFunc cb) = 0;

    /**
     * @brief Close this stream entirely; this normally means an error happened,
     * so it should not be used just to close the stream
     * @param cb to be called, when the operation succeeds of fails
     */
    virtual void reset(std::function<void(outcome::result<void>)> cb) = 0;

    /**
     * Set a new receive window size of this stream - how much unacknowledged
     * (not read) bytes can we on our side of the stream
     * @param new_size for the window
     * @param cb to be called, when the operation succeeds of fails
     */
    virtual void adjustWindowSize(
        uint32_t new_size, std::function<void(outcome::result<void>)> cb) = 0;

   private:
    /// this method is not to be used, as Stream supports only close with a
    /// callback
    outcome::result<void> close() override {
      return std::error_code();
    }
  };

}  // namespace libp2p::connection

#endif  // KAGOME_CONNECTION_STREAM_HPP
