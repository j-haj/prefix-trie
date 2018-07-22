#ifndef PREFIX_TRIE_H__
#define PREFIX_TRIE_H__
#include <memory>
#include <set>
#include <string>

#include "trie_node.h"

class PrefixTrie {
 public:
  PrefixTrie() : root_(std::make_unique<TrieNode>()) {}

  /**
   * Inserts the string into the prefix trie. This method is idempotent.
   */
  void Insert(const std::string& s) noexcept;

  /**
   * Check if prefix trie contains string.
   */
  bool Contains(const std::string& s) const noexcept;

  /**
   * Returns a set of suffixes of strings whose prefix is `s`.
   */
  std::set<std::string> Suffixes(const std::string& s) const noexcept;

 private:
  std::unique_ptr<TrieNode> root_;

};  // class PrefixTrie

#endif  // PREFIX_TRIE_H__
