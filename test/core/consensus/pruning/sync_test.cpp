#include <gtest/gtest.h>
#include <boost/di.hpp>
#include <boost/di/extension/scopes/shared.hpp>
#include <libp2p/injector/host_injector.hpp>
#include <soralog/impl/configurator_from_yaml.hpp>
#include "application/impl/app_configuration_impl.hpp"
#include "application/impl/chain_spec_impl.hpp"
#include "consensus/babe/impl/babe_synchronizer_impl.hpp"
#include "log/configurator.hpp"
#include "log/logger.hpp"
#include "sync_observer.hpp"
#include "sync_router.hpp"

namespace {
  std::string embedded_config(R"(
# ----------------
sinks:
  - name: console
    type: console
    thread: none
    color: false
    latency: 0
groups:
  - name: main
    sink: console
    level: debug
    is_fallback: true
    children:
      - name: kagome
        children:
          - name: metrics
# ----------------
)");
}

class Configurator : public soralog::ConfiguratorFromYAML {
 public:
  Configurator() : ConfiguratorFromYAML(embedded_config) {}
};

template <typename C>
auto useConfig(C c) {
  return boost::di::bind<std::decay_t<C>>().template to(
      std::move(c))[boost::di::override];
}

TEST(SyncTest, SimpleSync) {
  using namespace kagome;
  namespace di = boost::di;

  auto logging_system = std::make_shared<soralog::LoggingSystem>(
      std::make_shared<Configurator>());
  logging_system->configure();
  log::setLoggingSystem(logging_system);

  auto logger = log::createLogger("AppConfiguration", "main");
  application::AppConfigurationImpl configuration{logger};

  auto chain_spec_res = application::ChainSpecImpl::loadFrom(
      "/home/alex/project/kagome/examples/polkadot/polkadot.json");
  if (not chain_spec_res.has_value()) {
    logger->critical("Can't load chain spec");
    exit(EXIT_FAILURE);
  }

  libp2p::protocol::PingConfig ping_config{};

  auto injector = di::make_injector(
      di::bind<consensus::BabeSynchronizer>.template to<consensus::BabeSynchronizerImpl>(),
      di::bind<network::Router>.template to<network::SyncRouter>(),
      di::bind<network::SyncProtocolObserver>.template to<network::SyncObserver>(),
      di::bind<::boost::asio::io_context>.in(
          di::extension::shared)[boost::di::override],
      useConfig(ping_config),
      libp2p::injector::makeHostInjector(
          libp2p::injector::useSecurityAdaptors<
              libp2p::security::Noise>()[di::override]),
      di::bind<libp2p::crypto::random::RandomGenerator>.template to<libp2p::crypto::random::BoostRandomGenerator>()
          [di::override],
      di::bind<application::ChainSpec>.to(
          [chain_spec_res](const auto &) { return chain_spec_res.value(); }),
      di::bind<application::AppConfiguration>.to(configuration));

  auto router =
      injector.template create<std::shared_ptr<network::SyncRouter>>();
  router->prepare();
  router->start();
  auto sync =
      injector.template create<std::shared_ptr<consensus::BabeSynchronizer>>();
}
