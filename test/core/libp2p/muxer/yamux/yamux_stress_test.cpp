#include <utility>

/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include <gtest/gtest.h>
#include <random>
#include "libp2p/muxer/yamux.hpp"
#include "libp2p/security/plaintext.hpp"
#include "libp2p/transport/tcp.hpp"
#include "mock/libp2p/transport/upgrader_mock.hpp"
#include "testutil/libp2p/peer.hpp"
#include "testutil/literals.hpp"
#include "testutil/outcome.hpp"

using namespace libp2p;
using namespace transport;
using namespace connection;
using namespace muxer;
using namespace security;
using namespace multi;
using namespace peer;

using ::testing::_;
using ::testing::NiceMock;
using std::chrono_literals::operator""ms;

static const size_t kServerBufSize = 10000;  // Kb

ACTION_P(Upgrade, do_upgrade) {
  // arg0 - prev conn
  // arg1 - callback(next conn)
  arg1(do_upgrade(arg0));
}

struct Server : public std::enable_shared_from_this<Server> {
  explicit Server(boost::asio::io_context &context) {
    upgrader_ = std::make_shared<NiceMock<UpgraderMock>>();

    EXPECT_CALL(*upgrader_, upgradeToSecure(_, _))
        .WillRepeatedly(Upgrade([&](std::shared_ptr<RawConnection> raw) {
          println("secure inbound");
          return this->security_adaptor_->secureInbound(raw);
        }));
    EXPECT_CALL(*upgrader_, upgradeToMuxed(_, _))
        .WillRepeatedly(Upgrade([&](std::shared_ptr<SecureConnection> sec) {
          println("mux connection");
          return this->muxer_adaptor_->muxConnection(sec);
        }));

    transport_ = std::make_shared<TcpTransport>(context, upgrader_);

    listener_ = transport_->createListener(
        [this](outcome::result<std::shared_ptr<CapableConnection>> rconn) {
          EXPECT_OUTCOME_TRUE(conn, rconn);
          this->println("new connection received");
          this->onConnection(conn);
        });
  }

  void onConnection(const std::shared_ptr<CapableConnection> &conn) {
    conn->onStream([self{shared_from_this()}](
                       outcome::result<std::shared_ptr<Stream>> rstream) {
      EXPECT_OUTCOME_TRUE(stream, rstream);
      self->println("new stream created");
      self->onStream(stream);
    });

    conn->start();
  }

  void onStream(const std::shared_ptr<Stream> &stream) {
    // we should create buffer per stream (session)
    auto buf = std::make_shared<std::vector<uint8_t>>();
    buf->resize(kServerBufSize);

    println("onStream executed");

    stream->readSome(*buf, buf->size(),
                     [buf, stream, self{this->shared_from_this()}](
                         outcome::result<size_t> rread) {
                       EXPECT_OUTCOME_TRUE(read, rread);

                       self->println("readSome ", read, " bytes");

                       // echo back read data
                       stream->write(*buf, read,
                                     [buf, read, stream,
                                      self](outcome::result<size_t> rwrite) {
                                       EXPECT_OUTCOME_TRUE(write, rwrite);
                                       self->println("write ", write, " bytes");
                                       ASSERT_EQ(write, read);
                                     });
                     });
  }

  void listen(const Multiaddress &ma) {
    EXPECT_OUTCOME_TRUE_1(this->listener_->listen(ma));
  }

 private:
  template <typename... Args>
  void println(Args &&... args) {
    std::cout << "[server " << std::this_thread::get_id() << "]\t";
    (std::cout << ... << args);
    std::cout << std::endl;
  }

  std::shared_ptr<UpgraderMock> upgrader_;
  std::shared_ptr<Transport> transport_;

  std::shared_ptr<TransportListener> listener_;

  std::shared_ptr<SecurityAdaptor> security_adaptor_ =
      std::make_shared<Plaintext>();
  std::shared_ptr<MuxerAdaptor> muxer_adaptor_ = std::make_shared<Yamux>();
};

struct Client : public std::enable_shared_from_this<Client> {
  Client(boost::asio::io_context &context, PeerId p, size_t streams)
      : peer_id_(std::move(p)),
        streams_(streams),
        distribution(1, kServerBufSize) {
    generator.seed(rand());  // intentional

    upgrader_ = std::make_shared<UpgraderMock>();

    EXPECT_CALL(*upgrader_, upgradeToSecure(_, _))
        .WillRepeatedly(Upgrade([&](std::shared_ptr<RawConnection> raw) {
          println("secure outbound");
          return this->security_adaptor_->secureOutbound(raw, this->peer_id_);
        }));
    EXPECT_CALL(*upgrader_, upgradeToMuxed(_, _))
        .WillRepeatedly(Upgrade([&](std::shared_ptr<SecureConnection> sec) {
          // client has its own muxer, therefore, upgrader
          println("mux connection");
          return this->muxer_adaptor_->muxConnection(sec);
        }));

    transport_ = std::make_shared<TcpTransport>(context, upgrader_);
  }

  void connect(const Multiaddress &server) {
    // create new stream
    transport_->dial(
        server,
        [self{this->shared_from_this()}](
            outcome::result<std::shared_ptr<CapableConnection>> rconn) {
          EXPECT_OUTCOME_TRUE(conn, rconn);
          self->println("connected");
          self->onConnection(conn);
        });
  }

  void onConnection(const std::shared_ptr<CapableConnection> &conn) {
    for (size_t i = 0; i < streams_; i++) {
      conn->newStream([i, conn, self{this->shared_from_this()}](
                          outcome::result<std::shared_ptr<Stream>> rstream) {
        EXPECT_OUTCOME_TRUE(stream, rstream);
        self->println("new stream number ", i, " created");
        self->onStream(stream);
      });
    }
  }

  void onStream(const std::shared_ptr<Stream> &stream) {
    auto buf = randomBuffer();
    stream->write(
        *buf, buf->size(),
        [buf, stream,
         self{this->shared_from_this()}](outcome::result<size_t> rwrite) {
          EXPECT_OUTCOME_TRUE(write, rwrite);
          self->println("write ", write, " bytes");

          auto readbuf = std::make_shared<std::vector<uint8_t>>();
          readbuf->resize(write);

          stream->readSome(*readbuf, readbuf->size(),
                           [write, buf, readbuf, stream,
                            self](outcome::result<size_t> rread) {
                             EXPECT_OUTCOME_TRUE(read, rread);
                             self->println("readSome ", read, " bytes");

                             ASSERT_EQ(write, read);
                             ASSERT_EQ(*buf, *readbuf);
                           });
        });
  }

 private:
  template <typename... Args>
  void println(Args &&... args) {
    std::cout << "[client " << std::this_thread::get_id() << "]\t";
    (std::cout << ... << args);
    std::cout << std::endl;
  }

  size_t rand() {
    return distribution(generator);
  }

  std::shared_ptr<std::vector<uint8_t>> randomBuffer() {
    auto buf = std::make_shared<std::vector<uint8_t>>();
    buf->resize(this->rand());
    this->println("random buffer of size ", buf->size(), " generated");
    std::generate(buf->begin(), buf->end(), [self{this->shared_from_this()}]() {
      return self->rand() & 0xff;
    });
    return buf;
  }

  PeerId peer_id_;

  size_t streams_;

  std::default_random_engine generator;
  std::uniform_int_distribution<int> distribution;

  std::shared_ptr<UpgraderMock> upgrader_;
  std::shared_ptr<Transport> transport_;

  std::shared_ptr<SecurityAdaptor> security_adaptor_ =
      std::make_shared<Plaintext>();
  std::shared_ptr<MuxerAdaptor> muxer_adaptor_ = std::make_shared<Yamux>();
};

TEST(Yamux, StressTest) {
  // total number of parallel clients
  const int totalClients = 3;
  // total number of streams per connection
  const int streams = 1;
  // number, which makes tests reproducible
  const int seed = 0;

  boost::asio::io_context context(1);
  srand(seed);  // intentional

  auto serverAddr = "/ip4/127.0.0.1/tcp/40312"_multiaddr;

  auto server = std::make_shared<Server>(context);
  server->listen(serverAddr);

  std::vector<std::thread> clients;
  clients.reserve(totalClients);
  for (int i = 0; i < totalClients; i++) {
    auto &thread = clients.emplace_back([&]() {
      boost::asio::io_context context(1);
      auto pid = testutil::randomPeerId();

      auto client = std::make_shared<Client>(context, pid, streams);
      client->connect(serverAddr);

      context.run_for(5000ms);
    });

    thread.detach();
  }

  context.run_for(10000ms);

  for (auto &c : clients) {
    if (c.joinable()) {
      c.join();
    }
  }
}
