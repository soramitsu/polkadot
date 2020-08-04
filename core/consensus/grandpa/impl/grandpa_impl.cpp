/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "consensus/grandpa/impl/grandpa_impl.hpp"

#include <boost/asio/post.hpp>

#include "consensus/grandpa/impl/vote_crypto_provider_impl.hpp"
#include "consensus/grandpa/impl/vote_tracker_impl.hpp"
#include "consensus/grandpa/impl/voting_round_impl.hpp"
#include "consensus/grandpa/vote_graph/vote_graph_impl.hpp"
#include "scale/scale.hpp"
#include "storage/predefined_keys.hpp"

namespace kagome::consensus::grandpa {

  static size_t round_id = 0;

  GrandpaImpl::GrandpaImpl(
      std::shared_ptr<application::AppStateManager> app_state_manager,
      std::shared_ptr<Environment> environment,
      std::shared_ptr<storage::BufferStorage> storage,
      std::shared_ptr<crypto::ED25519Provider> crypto_provider,
      std::shared_ptr<runtime::GrandpaApi> grandpa_api,
      const crypto::ED25519Keypair &keypair,
      std::shared_ptr<Clock> clock,
      std::shared_ptr<boost::asio::io_context> io_context)
      : app_state_manager_(std::move(app_state_manager)),
        environment_{std::move(environment)},
        storage_{std::move(storage)},
        crypto_provider_{std::move(crypto_provider)},
        grandpa_api_{std::move(grandpa_api)},
        keypair_{keypair},
        clock_{std::move(clock)},
        io_context_{std::move(io_context)},
        liveness_checker_{*io_context_} {
    BOOST_ASSERT(app_state_manager_ != nullptr);
    BOOST_ASSERT(environment_ != nullptr);
    BOOST_ASSERT(storage_ != nullptr);
    BOOST_ASSERT(crypto_provider_ != nullptr);
    BOOST_ASSERT(grandpa_api_ != nullptr);
    BOOST_ASSERT(clock_ != nullptr);
    BOOST_ASSERT(io_context_ != nullptr);

    app_state_manager_->takeControl(*this);
  }

  bool GrandpaImpl::prepare() {
    // Lambda which is executed when voting round is completed.
    environment_->doOnCompleted(
        [wp = weak_from_this()](
            outcome::result<CompletedRound> completed_round_res) {
          if (auto self = wp.lock()) {
            self->onCompletedRound(completed_round_res);
          }
        });
    return true;
  }

  bool GrandpaImpl::start() {
    boost::asio::post(*io_context_,
                      [self{shared_from_this()}] { self->executeNextRound(); });

    startLivenessChecker();
    return true;
  }

  void GrandpaImpl::stop() {}

  outcome::result<std::shared_ptr<VoterSet>> GrandpaImpl::getVoters() const {
    /*
     * TODO(kamilsa): PRE-356 Check if voters were updated:
     * We should check if voters received from runtime (through
     * grandpa->grandpa_authorities() runtime entry call) differ from the ones
     * that we obtained from the storage. If so, we should return voter set with
     * incremented voter set and consisting of new voters. Also round number
     * should be reset to 0
     */
    OUTCOME_TRY(voters_encoded, storage_->get(storage::kAuthoritySetKey));
    OUTCOME_TRY(voter_set, scale::decode<VoterSet>(voters_encoded));
    return std::make_shared<VoterSet>(voter_set);
  }

  outcome::result<CompletedRound> GrandpaImpl::getLastCompletedRound() const {
    OUTCOME_TRY(last_round_encoded, storage_->get(storage::kSetStateKey));

    return scale::decode<CompletedRound>(last_round_encoded);
  }

  void GrandpaImpl::executeNextRound() {
    // obtain grandpa voters
    auto voters_res = getVoters();
    BOOST_ASSERT_MSG(
        voters_res.has_value(),
        "Voters does not exist in storage. Stopping grandpa execution");
    const auto &voters = voters_res.value();
    BOOST_ASSERT_MSG(voters->size() != 0,
                     "Voters are empty. Stopping grandpa execution");

    // obtain last completed round
    auto last_round_res = getLastCompletedRound();
    BOOST_ASSERT_MSG(last_round_res, "Last round does not exist");
    const auto &last_round = last_round_res.value();
    auto [round_number, last_round_state] = last_round;
    round_number++;
    using std::chrono_literals::operator""ms;
    auto duration = Duration(3333ms);

    auto prevote_tracker = std::make_shared<VoteTrackerImpl>();
    auto precommit_tracker = std::make_shared<VoteTrackerImpl>();

    auto vote_graph = std::make_shared<VoteGraphImpl>(
        last_round_state.finalized.value(), environment_);

    GrandpaConfig config{.voters = voters,
                         .round_number = round_number,
                         .duration = duration,
                         .peer_id = keypair_.public_key};
    auto &&vote_crypto_provider = std::make_shared<VoteCryptoProviderImpl>(
        keypair_, crypto_provider_, round_number, voters);

    current_round_ =
        std::make_shared<VotingRoundImpl>(std::move(config),
                                          environment_,
                                          std::move(vote_crypto_provider),
                                          std::move(prevote_tracker),
                                          std::move(precommit_tracker),
                                          std::move(vote_graph),
                                          clock_,
                                          io_context_);
    logger_->debug("Starting grandpa round: {}", round_number);

    current_round_->primaryPropose(last_round_state);
    current_round_->prevote(last_round_state);
    current_round_->precommit(last_round_state);
  }

  void GrandpaImpl::startLivenessChecker() {
    // Check if round id was updated.
    // If it was not, that means that grandpa is not working
    auto current_round_id = round_id;

    using std::chrono_literals::operator""ms;
    liveness_checker_.expires_after(20000ms);

    liveness_checker_.async_wait([this, current_round_id](const auto &ec) {
      if (ec and ec != boost::asio::error::operation_aborted) {
        logger_->error("Error happened during liveness timer: {}",
                       ec.message());
        return;
      }
      // if round id was not updated, that means execution of round was
      // completed properly, execute again
      if (current_round_id == round_id) {
        logger_->warn("Round was not completed properly");
        start();
        return;
      }
      startLivenessChecker();
    });
  }

  void GrandpaImpl::onVoteMessage(const VoteMessage &msg) {
    auto current_round = current_round_;
    auto current_round_number = current_round->roundNumber();

    // ensure we are in current round
    if (msg.round_number != current_round_number) {
      return;
    }

    // get block info
    auto blockInfo = visit_in_place(msg.vote.message, [](const auto &vote) {
      return BlockInfo(vote.block_number, vote.block_hash);
    });

    // get authorities
    const auto &weighted_authorities_res =
        grandpa_api_->authorities(primitives::BlockId(blockInfo.block_number));
    if (!weighted_authorities_res.has_value()) {
      logger_->error("Can't get authorities");
      return;
    };
    auto &weighted_authorities = weighted_authorities_res.value();

    // find signer in authorities
    auto weighted_authority_it =
        std::find_if(weighted_authorities.begin(),
                     weighted_authorities.end(),
                     [&id = msg.vote.id](const auto &weighted_authority) {
                       return weighted_authority.id.id == id;
                     });

    if (weighted_authority_it == weighted_authorities.end()) {
      logger_->warn("Vote signed by unknown validator");
      return;
    };

    visit_in_place(
        msg.vote.message,
        [&current_round, &msg](const PrimaryPropose &primary_propose) {
          current_round->onPrimaryPropose(msg.vote);
        },
        [&current_round, &msg](const Prevote &prevote) {
          current_round->onPrevote(msg.vote);
        },
        [&current_round, &msg](const Precommit &precommit) {
          current_round->onPrecommit(msg.vote);
        });
  }

  void GrandpaImpl::onFinalize(const Fin &f) {
    logger_->debug("Received fin message for round: {}", f.round_number);
    if (f.round_number == current_round_->roundNumber()) {
      current_round_->onFinalize(f);
    }
  }

  void GrandpaImpl::onCompletedRound(
      outcome::result<CompletedRound> completed_round_res) {
    round_id++;

    if (not completed_round_res) {
      current_round_.reset();
      logger_->debug("Grandpa round was not finalized: {}",
                     completed_round_res.error().message());
    } else {
      const auto &completed_round = completed_round_res.value();

      const auto last_completed_round_res = getLastCompletedRound();
      if (not last_completed_round_res) {
        logger_->warn(last_completed_round_res.error().message());
        return;
      }
      const auto &last_completed_round = last_completed_round_res.value();

      // update last completed round if it is greater than previous last
      // completed round
      if (completed_round.round_number > last_completed_round.round_number) {
        if (auto put_res = storage_->put(
                storage::kSetStateKey,
                common::Buffer(scale::encode(completed_round).value()));
            not put_res) {
          logger_->error("New round state was not added to the storage");
          return;
        }
        BOOST_ASSERT(storage_->get(storage::kSetStateKey));
      }
    }
    boost::asio::post(*io_context_,
                      [self{shared_from_this()}] { self->executeNextRound(); });
  }

}  // namespace kagome::consensus::grandpa
