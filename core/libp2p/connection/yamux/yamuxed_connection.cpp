/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "libp2p/connection/yamux/yamuxed_connection.hpp"

#include <future>

#include "libp2p/connection/yamux/yamux_frame.hpp"
#include "libp2p/connection/yamux/yamux_stream.hpp"

OUTCOME_CPP_DEFINE_CATEGORY(libp2p::connection, YamuxedConnection::Error, e) {
  using ErrorType = libp2p::connection::YamuxedConnection::Error;
  switch (e) {
    case ErrorType::NO_SUCH_STREAM:
      return "no such stream was found; maybe, it is closed";
    case ErrorType::YAMUX_IS_CLOSED:
      return "this Yamux instance is closed";
    case ErrorType::FORBIDDEN_CALL:
      return "forbidden method was invoked";
    case ErrorType::OTHER_SIDE_ERROR:
      return "error happened on other side's behalf";
    case ErrorType::INTERNAL_ERROR:
      return "internal error happened";
  }
  return "unknown";
}

namespace libp2p::connection {
  YamuxedConnection::YamuxedConnection(
      std::shared_ptr<SecureConnection> connection,
      NewStreamHandler stream_handler, kagome::common::Logger logger)
      : connection_{std::move(connection)},
        new_stream_handler_{std::move(stream_handler)},
        is_active_{true},
        logger_{std::move(logger)} {
    // client uses odd numbers, server - even
    last_created_stream_id_ = connection_->isInitiator() ? 1 : 0;
  }

  outcome::result<void> YamuxedConnection::start() {
    return readerLoop();
  }

  outcome::result<std::shared_ptr<Stream>> YamuxedConnection::newStream() {
    auto stream_id = getNewStreamId();
    OUTCOME_TRY(connection_->write(newStreamMsg(stream_id)));

    auto created_stream =
        std::make_shared<YamuxStream>(shared_from_this(), stream_id);
    streams_.insert({stream_id, created_stream});
    return created_stream;
  }

  outcome::result<peer::PeerId> YamuxedConnection::localPeer() const {
    return connection_->localPeer();
  }

  outcome::result<peer::PeerId> YamuxedConnection::remotePeer() const {
    return connection_->remotePeer();
  }

  outcome::result<crypto::PublicKey> YamuxedConnection::remotePublicKey()
      const {
    return connection_->remotePublicKey();
  }

  bool YamuxedConnection::isInitiator() const noexcept {
    return connection_->isInitiator();
  }

  outcome::result<multi::Multiaddress> YamuxedConnection::localMultiaddr() {
    return connection_->localMultiaddr();
  }

  outcome::result<multi::Multiaddress> YamuxedConnection::remoteMultiaddr() {
    return connection_->remoteMultiaddr();
  }

  bool YamuxedConnection::isClosed() const {
    return connection_->isClosed();
  }

  outcome::result<void> YamuxedConnection::close() {
    OUTCOME_TRY(connection_->close());
    is_active_ = false;
    for (const auto &stream : streams_) {
      stream.second->resetStream();
    }
    return outcome::success();
  }

  outcome::result<size_t> YamuxedConnection::write(gsl::span<const uint8_t> s) {
    return Error::FORBIDDEN_CALL;
  }

  outcome::result<size_t> YamuxedConnection::writeSome(
      gsl::span<const uint8_t> s) {
    return Error::FORBIDDEN_CALL;
  }

  outcome::result<std::vector<uint8_t>> YamuxedConnection::read(size_t s) {
    return Error::FORBIDDEN_CALL;
  }

  outcome::result<std::vector<uint8_t>> YamuxedConnection::readSome(size_t s) {
    return Error::FORBIDDEN_CALL;
  }

  outcome::result<size_t> YamuxedConnection::read(gsl::span<uint8_t> s) {
    return Error::FORBIDDEN_CALL;
  }

  outcome::result<size_t> YamuxedConnection::readSome(gsl::span<uint8_t> s) {
    return Error::FORBIDDEN_CALL;
  }

  outcome::result<void> YamuxedConnection::readerLoop() {
    using FrameType = YamuxFrame::FrameType;

    while (is_active_ && !connection_->isClosed()) {
      OUTCOME_TRY(header_bytes, connection_->read(YamuxFrame::kHeaderLength));
      auto header_opt = parseFrame(header_bytes);
      if (!header_opt) {
        // could not parse the frame => client sent some nonsense
        OUTCOME_TRY(connection_->write(
            goAwayMsg(YamuxFrame::GoAwayError::PROTOCOL_ERROR)));
        return Error::OTHER_SIDE_ERROR;
      }

      switch (header_opt->type_) {
        case FrameType::DATA: {
          OUTCOME_TRY(processDataFrame(*header_opt));
          break;
        }
        case FrameType::WINDOW_UPDATE: {
          OUTCOME_TRY(processWindowUpdateFrame(*header_opt));
          break;
        }
        case FrameType::PING: {
          OUTCOME_TRY(processPingFrame(*header_opt));
          break;
        }
        case FrameType::GO_AWAY: {
          OUTCOME_TRY(processGoAwayFrame(*header_opt));
          break;
        }
        default:
          logger_->critical("garbage in parsed frame's type");
          return Error::INTERNAL_ERROR;
      }
    }

    return outcome::success();
  }

  outcome::result<void> YamuxedConnection::processDataFrame(
      const YamuxFrame &frame) {
    using Flag = YamuxFrame::Flag;

    auto stream_id = frame.stream_id_;
    switch (frame.flag_) {
      case Flag::SYN: {
        // can be start of a new stream, just data or both
        auto stream = findStream(stream_id);
        if (!stream) {
          // it is at least a new stream request; register it and send ack
          // message
          OUTCOME_TRY(new_stream, registerNewStream(stream_id));
          stream = std::move(new_stream);
        }
        // process data in this frame, if there is one
        return processData(stream, frame);
      }
      case Flag::ACK: {
        // can be ack of a new stream, just data or both
        OUTCOME_TRY(stream, processAck(stream_id));
        if (stream) {
          return processData(stream, frame);
        }
        break;
      }
      case Flag::FIN:
        closeStreamForRead(stream_id);
        break;
      case Flag::RST:
        removeStream(stream_id);
        break;
      default:
        logger_->critical("garbage in parsed frame's flag");
        return Error::INTERNAL_ERROR;
    }
    return outcome::success();
  }

  outcome::result<void> YamuxedConnection::processWindowUpdateFrame(
      const YamuxFrame &frame) {
    using Flag = YamuxFrame::Flag;

    auto stream_id = frame.stream_id_;
    switch (frame.flag_) {
      case Flag::SYN: {
        // can be start of a new stream or update of a window size
        if (auto stream = findStream(stream_id)) {
          // this stream is already opened => delta window update
          processWindowUpdate(stream, frame.length_);
          break;
        }
        // no such stream found => it's a creation of a new stream
        OUTCOME_TRY(stream, registerNewStream(stream_id));
        processWindowUpdate(stream, frame.length_);
        break;
      }
      case Flag::ACK: {
        if (auto stream = findStream(stream_id)) {
          processWindowUpdate(stream, frame.length_);
          break;
        }
        // if no such stream found, some error happened - reset the stream on
        // the other side just in case
        OUTCOME_TRY(connection_->write(resetStreamMsg(stream_id)));
        break;
      }
      case Flag::FIN: {
        if (auto stream = findStream(stream_id)) {
          processWindowUpdate(stream, frame.length_);
        }
        closeStreamForRead(stream_id);
        break;
      }
      case Flag::RST:
        removeStream(stream_id);
        break;
      default:
        logger_->critical("garbage in parsed frame's flag");
        return Error::INTERNAL_ERROR;
    }
    return outcome::success();
  }

  outcome::result<void> YamuxedConnection::processPingFrame(
      const YamuxFrame &frame) {
    OUTCOME_TRY(connection_->write(pingResponseMsg(frame.length_)));
    return outcome::success();
  }

  outcome::result<void> YamuxedConnection::processGoAwayFrame(
      const YamuxFrame &frame) {
    OUTCOME_TRY(close());
    return outcome::success();
  }

  std::shared_ptr<YamuxStream> YamuxedConnection::findStream(
      StreamId stream_id) {
    auto stream = streams_.find(stream_id);
    if (stream == streams_.end()) {
      return nullptr;
    }
    return stream->second;
  }

  outcome::result<std::shared_ptr<YamuxStream>>
  YamuxedConnection::registerNewStream(StreamId stream_id) {
    OUTCOME_TRY(connection_->write(ackStreamMsg(stream_id)));

    auto new_stream =
        std::make_shared<YamuxStream>(shared_from_this(), stream_id);
    streams_[stream_id] = new_stream;
    new_stream_handler_(new_stream);
    return new_stream;
  }

  outcome::result<void> YamuxedConnection::processData(
      const std::shared_ptr<YamuxStream> &stream, const YamuxFrame &frame) {
    auto data_length = frame.length_;
    if (data_length == 0) {
      return outcome::success();
    }

    // read the data, commit it to the stream and call handler, if exists
    OUTCOME_TRY(data_bytes, connection_->read(data_length));
    OUTCOME_TRY(stream->commitData(std::move(data_bytes)));
    if (auto stream_read_handlers =
            streams_read_handlers_.find(frame.stream_id_);
        stream_read_handlers != streams_read_handlers_.end()) {
      auto &handlers = stream_read_handlers->second;
      if (!handlers.empty() && handlers.front()()) {
        // if handler returns true, it means that it should be removed
        handlers.pop();
      }
    }

    return outcome::success();
  }

  outcome::result<std::shared_ptr<YamuxStream>> YamuxedConnection::processAck(
      StreamId stream_id) {
    // acknowledge of start of a new stream; if we don't have such a stream,
    // a reset should be sent in order to notify the other side about the
    // problem
    if (auto stream = findStream(stream_id)) {
      return stream;
    }
    OUTCOME_TRY(connection_->write(resetStreamMsg(stream_id)));
    return nullptr;
  }

  void YamuxedConnection::processWindowUpdate(
      const std::shared_ptr<YamuxStream> &stream, uint32_t window_delta) {
    stream->send_window_size_ += window_delta;
    if (auto write_handlers = streams_write_handlers_.find(stream->stream_id_);
        write_handlers != streams_write_handlers_.end()) {
      auto &handlers = write_handlers->second;
      if (!handlers.empty() && handlers.front()()) {
        handlers.pop();
      }
    }
  }

  void YamuxedConnection::closeStreamForRead(StreamId stream_id) {
    if (auto stream = findStream(stream_id)) {
      if (!stream->is_writable_) {
        removeStream(stream_id);
        return;
      }
      stream->is_readable_ = false;
    }
  }

  outcome::result<void> YamuxedConnection::closeStreamForWrite(
      StreamId stream_id) {
    if (auto stream = findStream(stream_id)) {
      OUTCOME_TRY(connection_->write(closeStreamMsg(stream_id)));
      if (!stream->is_readable_) {
        removeStream(stream_id);
      } else {
        stream->is_writable_ = false;
      }
      return outcome::success();
    }
    return Error::NO_SUCH_STREAM;
  }

  void YamuxedConnection::removeStream(StreamId stream_id) {
    if (auto stream = findStream(stream_id)) {
      streams_.erase(stream_id);
      stream->resetStream();
    }
  }

  YamuxedConnection::StreamId YamuxedConnection::getNewStreamId() {
    last_created_stream_id_ += 2;
    return last_created_stream_id_;
  }

  /// YAMUX STREAM API

  outcome::result<void> YamuxedConnection::streamProcessNextFrame() {
    return readerLoop();
  }

  void YamuxedConnection::streamAddWindowUpdateHandler(
      StreamId stream_id, ReadWriteCompletionHandler handler) {
    streams_write_handlers_[stream_id].push(std::move(handler));
  }

  outcome::result<size_t> YamuxedConnection::streamWrite(
      StreamId stream_id, gsl::span<const uint8_t> msg, bool some) {
    if (!is_active_) {
      return Error::YAMUX_IS_CLOSED;
    }

    auto stream_opt = findStream(stream_id);
    if (!stream_opt) {
      return Error::NO_SUCH_STREAM;
    }

    if (!some) {
      OUTCOME_TRY(written, connection_->write(dataMsg(stream_id, msg)));
      return written - YamuxFrame::kHeaderLength;
    }
    OUTCOME_TRY(written, connection_->writeSome(dataMsg(stream_id, msg)));
    return written - YamuxFrame::kHeaderLength;
  }

  void YamuxedConnection::streamRead(StreamId stream_id,
                                     ReadWriteCompletionHandler handler) {
    streams_read_handlers_[stream_id].push(std::move(handler));
  }

  outcome::result<void> YamuxedConnection::streamAckBytes(StreamId stream_id,
                                                          uint32_t bytes) {
    OUTCOME_TRY(connection_->write(windowUpdateMsg(stream_id, bytes)));
    return outcome::success();
  }

  outcome::result<void> YamuxedConnection::streamClose(StreamId stream_id) {
    return closeStreamForWrite(stream_id);
  }

  void YamuxedConnection::streamReset(StreamId stream_id) {
    auto written_res = connection_->write(resetStreamMsg(stream_id));
    if (!written_res) {
      logger_->error("cannot reset a stream: {}",
                     written_res.error().message());
    }
    removeStream(stream_id);
  }

}  // namespace libp2p::connection
