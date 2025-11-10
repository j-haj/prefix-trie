#ifndef PREFIX_TRIE_H__
#define PREFIX_TRIE_H__
#include <iterator>
#include <memory>
#include <set>
#include <sstream>
#include <stack>
#include <string>
#include <unordered_map>
#include <vector>

class PrefixTrie {
 public:
  // Statistics about the trie structure
  struct Stats {
    std::size_t num_strings;      // Total number of complete strings
    std::size_t num_nodes;         // Total number of nodes (including root)
    std::size_t max_depth;         // Maximum depth of any string
    double avg_depth;              // Average depth of strings
    double avg_branching_factor;   // Average children per non-leaf node
    std::size_t memory_bytes;      // Estimated memory usage in bytes
  };

  // Forward declaration for iterator
  class Iterator;

  // Iterator wrapper class for matches
  class MatchResult {
   public:
    MatchResult(const std::vector<std::string>& matches) : matches_(matches) {}

    Iterator begin() const;
    Iterator end() const;

   private:
    std::vector<std::string> matches_;
    friend class Iterator;
  };

  // Forward iterator for strings in the trie
  class Iterator {
   public:
    using iterator_category = std::forward_iterator_tag;
    using value_type = std::string;
    using difference_type = std::ptrdiff_t;
    using pointer = const std::string*;
    using reference = const std::string&;

    Iterator(const std::vector<std::string>* matches, std::size_t index)
        : matches_(matches), index_(index) {}

    reference operator*() const { return (*matches_)[index_]; }
    pointer operator->() const { return &(*matches_)[index_]; }

    Iterator& operator++() {
      ++index_;
      return *this;
    }

    Iterator operator++(int) {
      Iterator tmp = *this;
      ++index_;
      return tmp;
    }

    bool operator==(const Iterator& other) const {
      return index_ == other.index_ && matches_ == other.matches_;
    }

    bool operator!=(const Iterator& other) const {
      return !(*this == other);
    }

   private:
    const std::vector<std::string>* matches_;
    std::size_t index_;
  };

  PrefixTrie() : root_(std::make_unique<TrieNode>()) {}

  /**
   * Inserts the string into the prefix trie. This method is idempotent.
   */
  void Insert(const std::string& s) noexcept;

  /**
   * Removes the string from the prefix trie. If the string doesn't exist,
   * this is a no-op. Cleans up empty branches after removal.
   */
  void Remove(const std::string& s) noexcept;

  /**
   * Removes all strings from the trie.
   */
  void Clear() noexcept;

  /**
   * Returns the total number of strings stored in the trie.
   */
  std::size_t Size() const noexcept;

  /**
   * Returns the number of strings that match the given prefix.
   */
  std::size_t Count(const std::string& prefix) const noexcept;

  /**
   * Check if prefix trie contains string.
   */
  bool Contains(const std::string& s) const noexcept;

  /**
   * Returns an iterable result of all strings matching the given prefix.
   * Enables range-based for loops: for (const auto& s : trie.Matches("prefix"))
   */
  MatchResult Matches(const std::string& prefix) const noexcept;

  /**
   * Returns statistics about the trie structure for debugging and analysis.
   */
  Stats GetStats() const noexcept;

  /**
   * Returns a string visualization of the trie structure for debugging.
   * Shows the tree structure with indentation.
   */
  std::string Visualize() const;

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
    TrieNode* runner;
    std::size_t cur_index;
    std::stringstream base;

    // Handle empty prefix - match all strings in trie
    if (s.empty()) {
      runner = root_.get();
      cur_index = 0;
    } else {
      // Check if first character exists
      if (root_->Children().find(s[0]) == root_->Children().end())
        return;

      runner = root_->Children()[s[0]].get();
      cur_index = 1;

      // Traverse trie to end of prefix
      while (cur_index < s.size()) {
        base << runner->Key();
        // If we can't move forward the prefix must not exist, exit early
        if (runner->Children().find(s[cur_index]) == runner->Children().end())
          return;
        runner = runner->Children()[s[cur_index]].get();
        ++cur_index;
      }
      base << runner->Key();
    }

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

      // Check if this is a string termination marker
      if (tmp.second->Key() == '\0') {
        // Construct full string since we found a complete string
        std::stringstream p;
        for (const auto c : postfix) {
          p << c;
        }
        std::stringstream ss;
        ss << prefix << p.str();

        // String constructed, pass to callback
        callback(ss.str());
      } else {
        postfix.push_back(tmp.second->Key());

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
