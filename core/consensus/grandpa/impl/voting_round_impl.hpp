/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef KAGOME_CORE_CONSENSUS_GRANDPA_IMPL_VOTING_ROUND_IMPL_HPP
#define KAGOME_CORE_CONSENSUS_GRANDPA_IMPL_VOTING_ROUND_IMPL_HPP

#include "consensus/grandpa/voting_round.hpp"

#include <boost/asio/basic_waitable_timer.hpp>
#include <boost/signals2.hpp>
#include "common/logger.hpp"
#include "consensus/grandpa/environment.hpp"
#include "consensus/grandpa/grandpa_config.hpp"
#include "consensus/grandpa/vote_crypto_provider.hpp"
#include "consensus/grandpa/vote_graph.hpp"
#include "consensus/grandpa/vote_tracker.hpp"

namespace kagome::consensus::grandpa {

  class VotingRoundImpl : public VotingRound {
    using OnCompleted = boost::signals2::signal<void(const CompletedRound &)>;
    using OnCompletedSlotType = OnCompleted::slot_type;

   public:
    ~VotingRoundImpl() override = default;

    VotingRoundImpl(GrandpaConfig config,
                    std::shared_ptr<Environment> env,
                    std::shared_ptr<VoteCryptoProvider> vote_crypto_provider,
                    std::shared_ptr<VoteTracker<Prevote>> prevotes,
                    std::shared_ptr<VoteTracker<Precommit>> precommits,
                    std::shared_ptr<VoteGraph> graph,
                    std::shared_ptr<Clock> clock,
                    std::shared_ptr<boost::asio::io_context> io_context);

    void onFin(const Fin &f) override;

    void onPrimaryPropose(const SignedPrimaryPropose &primary_propose) override;

    void onPrevote(const SignedPrevote &prevore) override;

    void onPrecommit(const SignedPrecommit &precommit) override;

    bool tryFinalize() override;

    RoundNumber roundNumber() const override;

    void primaryPropose(const RoundState &last_round_state) override;

    void prevote(const RoundState &last_round_state) override;

    void precommit(const RoundState &last_round_state) override;

    inline const RoundState &getCurrentState() const {
      return cur_round_state_;
    }

    bool completable() const;

   private:
    /// Check if peer \param id is primary
    bool isPrimary(const Id &id) const;

    /// Check if current peer is primary
    bool isPrimary() const;

    /// Calculate threshold from the total weights of voters
    size_t getThreshold(const std::shared_ptr<VoterSet> &voters);

    /// Triggered when we receive \param signed_prevote for the current peer
    void onSignedPrevote(const SignedPrevote &signed_prevote);

    /// Triggered when we receive \param signed_precommit for the current peer
    void onSignedPrecommit(const SignedPrecommit &signed_precommit);

    /// Updates current round's prevote ghost. Invoked after each
    /// onSingedPrevote
    void updatePrevoteGhost();

    /// Update current state of the round. In particular we update:
    /// 1. If round is completable
    /// 2. If round has something ready to finalize
    /// 3. New best rounds estimate
    void update();

    // notify about new finalized round. False if new state does not differ from
    // old one
    bool notify(const RoundState &last_round_state);

    /// prepare justification for the provided \param estimate
    boost::optional<GrandpaJustification> finalizingPrecommits(
        const BlockInfo &estimate) const;

    /// Construct \return SignedPrevote with provided \param last_round_state
    outcome::result<SignedPrevote> constructPrevote(
        const RoundState &last_round_state) const;

    /// Construct \return SignedPrecommit with provided \param last_round_state
    outcome::result<SignedPrecommit> constructPrecommit(
        const RoundState &last_round_state) const;

    /// Check if received \param vote has valid \param justification. If so
    /// \return true, false otherwise
    bool validate(const BlockInfo &vote,
                  const GrandpaJustification &justification) const;

   private:
    std::shared_ptr<VoterSet> voter_set_;
    const RoundNumber round_number_;
    const Duration duration_;  // length of round (T in spec)
    TimePoint start_time_;
    RoundState cur_round_state_;

    boost::optional<RoundState> last_round_state_;
    const Id id_;  // id of current peer

    std::shared_ptr<Environment> env_;

    std::shared_ptr<VoteCryptoProvider> vote_crypto_provider_;
    State state_;
    size_t threshold_;  // supermajority threshold

    std::shared_ptr<VoteTracker<Prevote>> prevotes_;
    std::shared_ptr<VoteTracker<Precommit>> precommits_;
    std::shared_ptr<VoteGraph> graph_;

    std::shared_ptr<Clock> clock_;

    std::shared_ptr<boost::asio::io_context> io_context_;
    Timer prevote_timer_;
    Timer precommit_timer_;

    common::Logger logger_;
    // equivocators arrays. Index in vector corresponds to the index of voter in
    // voterset, value corresponds to the weight of the voter
    std::vector<bool> prevote_equivocators_;
    std::vector<bool> precommit_equivocators_;

    boost::optional<PrimaryPropose> primary_vote_;
    bool completable_{false};
  };
}  // namespace kagome::consensus::grandpa

#endif  // KAGOME_CORE_CONSENSUS_GRANDPA_IMPL_VOTING_ROUND_IMPL_HPP
