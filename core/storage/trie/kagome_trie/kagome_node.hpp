#pragma once

#include <bitset>
#include "storage/trie/node.hpp"

namespace kagome::storage::trie {
  struct KagomeNode : public Node {
    KagomeNode(const KagomeNode&) = delete;
    KagomeNode(KagomeNode&&) noexcept = default;
    KagomeNode &operator=(const KagomeNode &) = delete;
    KagomeNode & operator=(KagomeNode&&) noexcept = default;
    std::optional<common::Buffer> value{std::nullopt};

    enum class Type : std::bitset<4>{Dummy = 0b00,
                                     Leaf = 0b01,
                                     BranchEmptyValue = 0b10,
                                     BranchWithValue = 0b11} type{Dummy};

    std::optional<std::array<KagomeNode, 16>> children{std::nullopt};
  };

  class KagomeTrie {
    KagomeNode root_;

    void insert(const KagomeNode& parent, com)
  };
}  // namespace kagome::storage::trie
