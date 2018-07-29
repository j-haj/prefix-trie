#ifndef PREFIX_TRIE_H__
#define PREFIX_TRIE_H__
#include <iterator>
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
   * Takes a prefix an iterator to a container in which the strings matching the
   * given prefix will be copied.
   */
  template <typename Container>
  void MatchBackInserter(Container& c, const std::string& s) const noexcept {
    auto bi = std::back_insert_iterator<Container>(c);
    MatchWithCallback(s, [&bi](const std::string& s) {
      *bi = s;
      ++bi;
    });
  }

  /**
   * Passes strings who match the given prefix into the given function callback.
   *
   * The strings are found via an iterative depth-first traversal to save
   * memory. Note the empty string prefix is always matched.
   */
  void MatchWithCallback(
      const std::string& s,
      const std::function<void(const std::string&)>& callback) const;

 private:
  std::unique_ptr<TrieNode> root_;

};  // class PrefixTrie

#endif  // PREFIX_TRIE_H__
