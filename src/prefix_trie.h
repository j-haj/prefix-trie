#ifndef PREFIX_TRIE_TEMPLATED_H__
#define PREFIX_TRIE_TEMPLATED_H__

#include <algorithm>
#include <cstddef>
#include <functional>
#include <iterator>
#include <memory>
#include <set>
#include <sstream>
#include <stack>
#include <string>
#include <unordered_map>
#include <vector>

template <typename CharT>
class PrefixTrieBase {
 public:
  using string_type = std::basic_string<CharT>;
  using stringstream_type = std::basic_stringstream<CharT>;
  using ostringstream_type = std::basic_ostringstream<CharT>;

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
    MatchResult(const std::vector<string_type>& matches) : matches_(matches) {}

    Iterator begin() const { return Iterator(&matches_, 0); }
    Iterator end() const { return Iterator(&matches_, matches_.size()); }

   private:
    std::vector<string_type> matches_;
    friend class Iterator;
  };

  // Forward iterator for strings in the trie
  class Iterator {
   public:
    using iterator_category = std::forward_iterator_tag;
    using value_type = string_type;
    using difference_type = std::ptrdiff_t;
    using pointer = const string_type*;
    using reference = const string_type&;

    Iterator(const std::vector<string_type>* matches, std::size_t index)
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

    bool operator!=(const Iterator& other) const { return !(*this == other); }

   private:
    const std::vector<string_type>* matches_;
    std::size_t index_;
  };

  PrefixTrieBase() : root_(std::make_unique<TrieNode>()) {}

  /**
   * Inserts the string into the prefix trie. This method is idempotent.
   */
  void Insert(const string_type& s) noexcept {
    if (s.empty()) return;
    TrieNode* runner = root_.get();
    std::size_t cur_index = 0;
    while (!runner->Children().empty() && cur_index < s.size()) {
      if (runner->Children().find(s[cur_index]) == runner->Children().end())
        break;
      runner = runner->Children()[s[cur_index]].get();
      ++cur_index;
    }
    while (cur_index < s.size()) {
      runner->Children()[s[cur_index]] =
          std::make_unique<TrieNode>(s[cur_index]);
      runner = runner->Children()[s[cur_index]].get();
      ++cur_index;
    }
    runner->Children()[s[cur_index - 1]] = std::make_unique<TrieNode>(CharT());
  }

  /**
   * Removes the string from the prefix trie. If the string doesn't exist,
   * this is a no-op. Cleans up empty branches after removal.
   */
  void Remove(const string_type& s) noexcept {
    if (s.empty()) return;

    // Build path to the string while traversing
    std::vector<std::pair<TrieNode*, CharT>> path;
    TrieNode* runner = root_.get();
    std::size_t cur_index = 0;

    // Traverse to the end of the string
    while (cur_index < s.size()) {
      if (runner->Children().find(s[cur_index]) == runner->Children().end())
        return;  // String doesn't exist
      path.push_back(std::make_pair(runner, s[cur_index]));
      runner = runner->Children()[s[cur_index]].get();
      ++cur_index;
    }

    // Check if this is actually a complete string (has '\0' marker)
    if (runner->Children().find(s[cur_index - 1]) == runner->Children().end())
      return;  // Not a complete string

    // Remove the termination marker
    runner->Children().erase(s[cur_index - 1]);

    // Walk back up the path, removing nodes that have no children
    for (auto it = path.rbegin(); it != path.rend(); ++it) {
      TrieNode* parent = it->first;
      CharT key = it->second;
      TrieNode* child = parent->Children()[key].get();

      // If the child has no children, remove it
      if (child->Children().empty()) {
        parent->Children().erase(key);
      } else {
        // Stop cleanup if we reach a node with children
        break;
      }
    }
  }

  /**
   * Removes all strings from the trie.
   */
  void Clear() noexcept { root_ = std::make_unique<TrieNode>(); }

  /**
   * Returns the total number of strings stored in the trie.
   */
  std::size_t Size() const noexcept { return Count(string_type()); }

  /**
   * Returns the number of strings that match the given prefix.
   */
  std::size_t Count(const string_type& prefix) const noexcept {
    std::size_t count = 0;

    // Handle empty prefix - count all strings in trie
    TrieNode* start_node = root_.get();
    std::size_t cur_index = 0;

    // Navigate to the prefix node
    if (!prefix.empty()) {
      if (root_->Children().find(prefix[0]) == root_->Children().end())
        return 0;  // Prefix doesn't exist

      TrieNode* runner = root_->Children()[prefix[0]].get();
      cur_index = 1;

      while (cur_index < prefix.size()) {
        if (runner->Children().find(prefix[cur_index]) ==
            runner->Children().end())
          return 0;  // Prefix doesn't exist
        runner = runner->Children()[prefix[cur_index]].get();
        ++cur_index;
      }
      start_node = runner;
    }

    // DFS to count all '\0' markers from start_node
    std::stack<TrieNode*> nodes;
    for (const auto& n : start_node->Children()) {
      nodes.push(n.second.get());
    }

    while (!nodes.empty()) {
      TrieNode* node = nodes.top();
      nodes.pop();

      // Check if this is a string termination marker
      if (node->Key() == CharT()) {
        ++count;
      } else {
        // Add all children to stack
        for (const auto& c : node->Children()) {
          nodes.push(c.second.get());
        }
      }
    }

    return count;
  }

  /**
   * Check if prefix trie contains string.
   */
  bool Contains(const string_type& s) const noexcept {
    if (s.empty()) return true;

    TrieNode* runner = root_.get();
    std::size_t cur_index = 0;
    while (!runner->Children().empty() && cur_index < s.size()) {
      if (runner->Children().find(s[cur_index]) == runner->Children().end())
        return false;
      runner = runner->Children()[s[cur_index]].get();
      ++cur_index;
    }
    return cur_index == s.size();
  }

  /**
   * Returns an iterable result of all strings matching the given prefix.
   * Enables range-based for loops: for (const auto& s : trie.Matches("prefix"))
   */
  MatchResult Matches(const string_type& prefix) const noexcept {
    std::vector<string_type> matches;
    MatchBackInserter(matches, prefix);
    return MatchResult(matches);
  }

  /**
   * Returns statistics about the trie structure for debugging and analysis.
   */
  Stats GetStats() const noexcept {
    Stats stats = {0, 0, 0, 0.0, 0.0, 0};

    // DFS traversal to collect statistics
    std::stack<std::pair<TrieNode*, std::size_t>> nodes;  // node, depth
    std::size_t total_depth = 0;
    std::size_t non_leaf_nodes = 0;
    std::size_t total_children = 0;

    // Start with root's children
    stats.num_nodes = 1;  // Count root
    for (const auto& n : root_->Children()) {
      nodes.push(std::make_pair(n.second.get(), 1));
    }

    while (!nodes.empty()) {
      auto pair = nodes.top();
      TrieNode* node = pair.first;
      std::size_t depth = pair.second;
      nodes.pop();

      ++stats.num_nodes;

      // Check if this is a string termination marker
      if (node->Key() == CharT()) {
        ++stats.num_strings;
        total_depth += depth;
        stats.max_depth = std::max(stats.max_depth, depth);
      } else {
        // Count children for branching factor
        std::size_t num_children = node->Children().size();
        if (num_children > 0) {
          ++non_leaf_nodes;
          total_children += num_children;
        }

        // Add children to stack
        for (const auto& c : node->Children()) {
          nodes.push(std::make_pair(c.second.get(), depth + 1));
        }
      }
    }

    // Calculate averages
    if (stats.num_strings > 0) {
      stats.avg_depth = static_cast<double>(total_depth) / stats.num_strings;
    }

    if (non_leaf_nodes > 0) {
      stats.avg_branching_factor =
          static_cast<double>(total_children) / non_leaf_nodes;
    }

    // Estimate memory usage
    const std::size_t node_overhead = sizeof(TrieNode);
    const std::size_t map_entry_overhead =
        sizeof(CharT) + sizeof(std::unique_ptr<TrieNode>);
    stats.memory_bytes =
        stats.num_nodes * node_overhead + total_children * map_entry_overhead;

    return stats;
  }

  /**
   * Returns a string visualization of the trie structure for debugging.
   */
  string_type Visualize() const {
    ostringstream_type oss;
    oss << "Root\n";

    // Helper lambda for recursive visualization
    std::function<void(TrieNode*, const string_type&, std::size_t)>
        visualize_node;
    visualize_node = [&](TrieNode* node, const string_type& prefix,
                        std::size_t index) {
      const auto& children = node->Children();
      std::size_t i = 0;
      for (const auto& pair : children) {
        CharT key = pair.first;
        const std::unique_ptr<TrieNode>& child = pair.second;

        bool is_last = (i == children.size() - 1);
        // Use ASCII characters for compatibility
        string_type connector;
        string_type child_prefix_str;

        if (is_last) {
          connector += CharT('+');
          connector += CharT('-');
          connector += CharT('-');
          connector += CharT(' ');
          for (int j = 0; j < 4; ++j) child_prefix_str += CharT(' ');
        } else {
          connector += CharT('|');
          connector += CharT('-');
          connector += CharT('-');
          connector += CharT(' ');
          child_prefix_str += CharT('|');
          for (int j = 0; j < 3; ++j) child_prefix_str += CharT(' ');
        }

        string_type child_prefix = prefix + child_prefix_str;

        if (child->Key() == CharT()) {
          oss << prefix << connector << "[END]\n";
        } else {
          oss << prefix << connector << child->Key();

          // Check if this node represents a complete string
          bool has_terminator =
              child->Children().find(child->Key()) != child->Children().end() &&
              child->Children()[child->Key()]->Key() == CharT();

          if (has_terminator) {
            oss << " *";  // Mark complete strings
          }
          oss << "\n";

          // Recursively visualize children
          visualize_node(child.get(), child_prefix, 0);
        }
        ++i;
      }
    };

    visualize_node(root_.get(), string_type(), 0);
    return oss.str();
  }

  /**
   * Takes a prefix and a container in which the strings matching the
   * given prefix will be copied.
   */
  template <typename Container>
  void MatchBackInserter(Container& c, const string_type& s) const noexcept {
    auto bi = std::back_insert_iterator<Container>(c);
    MatchWithCallback(s, [&bi](const string_type& s) {
      *bi = s;
      ++bi;
    });
  }

  /**
   * Passes strings who match the given prefix into the given function callback.
   *
   * The strings are found via an iterative depth-first traversal to save
   * memory.
   */
  template <typename Callable>
  void MatchWithCallback(const string_type& s, const Callable& callback) const {
    TrieNode* runner;
    std::size_t cur_index;
    stringstream_type base;

    // Handle empty prefix - match all strings in trie
    if (s.empty()) {
      runner = root_.get();
      cur_index = 0;
    } else {
      // Check if first character exists
      if (root_->Children().find(s[0]) == root_->Children().end()) return;

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
    std::vector<CharT> postfix;
    string_type prefix = base.str();
    while (!nodes.empty()) {
      auto tmp = nodes.top();
      nodes.pop();

      // Discard all postfix characters from most recent DFS that are beyond
      // our current depth within the tree
      while (prefix.size() + postfix.size() > tmp.first) postfix.pop_back();

      // Check if this is a string termination marker
      if (tmp.second->Key() == CharT()) {
        // Construct full string since we found a complete string
        stringstream_type p;
        for (const auto c : postfix) {
          p << c;
        }
        stringstream_type ss;
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
    TrieNode() : TrieNode(CharT()) {}

    /**
     * Constructs a TrieNode with the given key.
     */
    TrieNode(CharT k) : key_(k) {}

    // No copy-constructor since each TrieNode owns its children data
    TrieNode(const TrieNode& o) = delete;
    TrieNode(TrieNode&& o) : key_(o.key_) {
      for (auto& p : o.children_) {
        children_[p.first] = std::move(p.second);
      }
    }

    CharT Key() const noexcept { return key_; }
    bool IsLeaf() const noexcept { return children_.empty(); }

    std::unordered_map<CharT, std::unique_ptr<TrieNode>>& Children() noexcept {
      return children_;
    }

    const std::unordered_map<CharT, std::unique_ptr<TrieNode>>& Children()
        const noexcept {
      return children_;
    }

   private:
    CharT key_;
    std::unordered_map<CharT, std::unique_ptr<TrieNode>> children_;
  };  // class TrieNode
  std::unique_ptr<TrieNode> root_;

};  // class PrefixTrieBase

// Type aliases for common character types
using PrefixTrie = PrefixTrieBase<char>;
using WPrefixTrie = PrefixTrieBase<wchar_t>;

#endif  // PREFIX_TRIE_TEMPLATED_H__
