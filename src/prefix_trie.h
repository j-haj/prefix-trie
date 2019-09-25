#ifndef PREFIX_TRIE_H__
#define PREFIX_TRIE_H__
#include <iterator>
#include <memory>
#include <set>
#include <sstream>
#include <stack>
#include <string>
#include <unordered_map>

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
  template <typename Callable>
  void MatchWithCallback(const std::string& s, const Callable& callback) const {
    // Check early exit conditions
    if (s.empty())
      callback("");
    else if (root_->Children().find(s[0]) == root_->Children().end())
      return;

    TrieNode* runner = root_->Children()[s[0]].get();
    std::size_t cur_index = 1;

    // Traverse trie to end of prefix
    std::stringstream base;
    while (cur_index < s.size()) {
      base << runner->Key();
      // If we can't move forward the prefix must not exist, exit early
      if (runner->Children().find(s[cur_index]) == runner->Children().end())
        return;
      runner = runner->Children()[s[cur_index]].get();
      ++cur_index;
    }
    base << runner->Key();

    // Begin depth-first traversal over all strings who match the given prefix
    std::stack<std::pair<std::size_t, TrieNode*>> nodes;
    for (const auto& n : runner->Children()) {
      nodes.push(std::make_pair(cur_index, n.second.get()));
    }
    std::vector<char> postfix;
    std::string prefix = base.str();
    while (!nodes.empty()) {
      auto tmp = nodes.top();
      nodes.pop();

      // Discard all postfix characters from most recent DFS that are beyond
      // our current depth within the tree
      while (prefix.size() + postfix.size() > tmp.first) postfix.pop_back();

      postfix.push_back(tmp.second->Key());

      if (tmp.second->IsLeaf()) {
        // Construct full string since we are at a leaf node
        std::stringstream p;
        for (const auto c : postfix) {
          p << c;
        }
        std::stringstream ss;
        ss << prefix << p.str();

        // String constructed, pass to callback
        callback(ss.str());
      } else {
        // Add all children nodes to stack
        for (const auto& c : tmp.second->Children()) {
          nodes.push(std::make_pair(tmp.first + 1, c.second.get()));
        }
      }
    }
  }

 private:
  class TrieNode {
   public:
    /**
     * Default constructor.
     */
    TrieNode() : TrieNode('\0') {}

    /**
     * Constructs a TrieNode with the given key.
     */
    TrieNode(char k) : key_(k) {}

    // No copy-constructor since each TrieNode owns its children data
    TrieNode(const TrieNode& o) = delete;
    TrieNode(TrieNode&& o) : key_(o.key_) {
      for (auto& p : o.children_) {
        children_[p.first] = std::move(p.second);
      }
    }

    char Key() const noexcept { return key_; }
    bool IsLeaf() const noexcept { return children_.empty(); }

    std::unordered_map<char, std::unique_ptr<TrieNode>>& Children() noexcept {
      return children_;
    }

   private:
    char key_;
    std::unordered_map<char, std::unique_ptr<TrieNode>> children_;
  };  // class TrieNode
  std::unique_ptr<TrieNode> root_;

};  // class PrefixTrie

#endif  // PREFIX_TRIE_H__
