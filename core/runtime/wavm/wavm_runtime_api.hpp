/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef KAGOME_CORE_RUNTIME_WAVM_WAVM_RUNTIME_API_HPP
#define KAGOME_CORE_RUNTIME_WAVM_WAVM_RUNTIME_API_HPP

#include "common/logger.hpp"
#include "runtime/wasm_result.hpp"
#include "scale/scale.hpp"
#include "runtime/wavm/module_provider.hpp"

namespace kagome::runtime::wavm {

  class WavmRuntimeApi {
   public:
    virtual ~WavmRuntimeApi() = default;

    WavmRuntimeApi(
        std::shared_ptr<WasmProvider> wasm_provider,
        std::shared_ptr<extensions::ExtensionFactory> extenstion_factory,
        std::shared_ptr<TrieStorageProvider> trie_storage_provider);

   protected:

    /**
     * @brief executes wasm export method returning non-void result
     * @tparam R result type including void
     * @tparam Args arguments types list
     * @param name - export method name
     * @param state_root - a hash of the state root to which the state will be
     * reset before executing the export method
     * @param persistency - PERSISTENT if changes made by the method should
     * persist in the state, EPHEMERAL if they can be discraded
     * @param args - export method arguments
     * @return a parsed result or an error
     */
    template <typename R, typename... Args>
    outcome::result<R> executeAt(std::string_view name,
                                 const common::Hash256 &state_root,
                                 CallPersistency persistency,
                                 Args &&... args) {
      return executeMaybeAt<R>(
          name, state_root, persistency, std::forward<Args>(args)...);
    }

    /**
     * @brief executes wasm export method returning non-void result
     * @tparam R result type including void
     * @tparam Args arguments types list
     * @param name - export method name
     * @param persistency - PERSISTENT if changes made by the method should
     * persist in the state, EPHEMERAL if they can be discraded
     * @param args - export method arguments
     * @return a parsed result or an error
     */
    template <typename R, typename... Args>
    outcome::result<R> execute(std::string_view name,
                               CallPersistency persistency,
                               Args &&... args) {
      return executeMaybeAt<R>(
          name, boost::none, persistency, std::forward<Args>(args)...);
    }

   private:

    /**
     * If \arg state_root contains a value, then the state will be reset to the
     * hash in this value, otherwise the export method will be executed on the
     * current state
     * @note for explanation of arguments \see execute or \see executeAt
     */
    template <typename R, typename... Args>
    outcome::result<R> executeMaybeAt(
        std::string_view name,
        const boost::optional<common::Hash256> &state_root,
        CallPersistency persistency,
        Args &&... args) {

      std::vector<uint8_t> scale_args{};

      if constexpr (sizeof...(args) > 0) {
        OUTCOME_TRY(buffer, scale::encode(std::forward<Args>(args)...));
        scale_args = std::move(buffer);
      }

      OUTCOME_TRY(res, module_provider_->execute(name, state_root, persistency, scale_args));

      if constexpr (!std::is_same_v<void, R>) {
        return scale::decode<R>(res);
      }
      return outcome::success();
    }

   private:
    std::shared_ptr<ModuleProvider> module_provider_;
  };

}  // namespace kagome::runtime::wavm

#endif  // KAGOME_CORE_RUNTIME_WAVM_WAVM_RUNTIME_API_HPP
