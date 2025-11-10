#ifndef PREFIX_TRIE_TEMPLATED_H__
#define PREFIX_TRIE_TEMPLATED_H__

#include <algorithm>
#include <cstddef>
#include <functional>
#include <iomanip>
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
   * Serializes the trie to JSON format.
   * Returns a JSON string containing all strings in the trie.
   */
  std::string ToJSON() const {
    std::ostringstream oss;
    oss << "[";

    std::vector<string_type> all_strings;
    MatchBackInserter(all_strings, string_type());

    for (std::size_t i = 0; i < all_strings.size(); ++i) {
      if (i > 0) oss << ",";
      oss << "\"";
      // Escape special characters
      for (std::size_t j = 0; j < all_strings[i].size(); ++j) {
        CharT ch = all_strings[i][j];
        if (ch == CharT('"')) {
          oss << "\\\"";
        } else if (ch == CharT('\\')) {
          oss << "\\\\";
        } else if (ch == CharT('\n')) {
          oss << "\\n";
        } else if (ch == CharT('\r')) {
          oss << "\\r";
        } else if (ch == CharT('\t')) {
          oss << "\\t";
        } else if (sizeof(CharT) == 1) {
          // For char, output directly
          oss << static_cast<char>(ch);
        } else {
          // For wchar_t, use Unicode escape sequence
          oss << "\\u" << std::hex << std::setw(4) << std::setfill('0')
              << static_cast<int>(ch) << std::dec;
        }
      }
      oss << "\"";
    }

    oss << "]";
    return oss.str();
  }

  /**
   * Deserializes a trie from JSON format.
   * Expects a JSON array of strings: ["string1", "string2", ...]
   * Returns true on success, false on parse error.
   */
  bool FromJSON(const std::string& json) {
    Clear();

    // Simple JSON parser for array of strings
    std::size_t pos = 0;

    // Skip whitespace
    auto skip_whitespace = [&]() {
      while (pos < json.size() &&
             (json[pos] == ' ' || json[pos] == '\n' ||
              json[pos] == '\r' || json[pos] == '\t')) {
        ++pos;
      }
    };

    skip_whitespace();
    if (pos >= json.size() || json[pos] != '[') return false;
    ++pos;

    bool found_closing = false;
    while (pos < json.size()) {
      skip_whitespace();

      if (pos >= json.size()) return false;

      if (json[pos] == ']') {
        ++pos;
        found_closing = true;
        break;
      }

      if (json[pos] != '"') return false;
      ++pos;

      // Parse string
      string_type str;
      while (pos < json.size() && json[pos] != '"') {
        if (json[pos] == '\\') {
          ++pos;
          if (pos >= json.size()) return false;

          if (json[pos] == '"') {
            str += CharT('"');
          } else if (json[pos] == '\\') {
            str += CharT('\\');
          } else if (json[pos] == 'n') {
            str += CharT('\n');
          } else if (json[pos] == 'r') {
            str += CharT('\r');
          } else if (json[pos] == 't') {
            str += CharT('\t');
          } else if (json[pos] == 'u') {
            // Unicode escape: \uXXXX
            if (pos + 4 >= json.size()) return false;
            ++pos;
            int code = 0;
            for (int i = 0; i < 4; ++i) {
              char hex = json[pos + i];
              if (hex >= '0' && hex <= '9') {
                code = code * 16 + (hex - '0');
              } else if (hex >= 'a' && hex <= 'f') {
                code = code * 16 + (hex - 'a' + 10);
              } else if (hex >= 'A' && hex <= 'F') {
                code = code * 16 + (hex - 'A' + 10);
              } else {
                return false;
              }
            }
            str += static_cast<CharT>(code);
            pos += 3;  // Will be incremented by 1 at end of loop
          } else {
            return false;
          }
          ++pos;
        } else {
          str += static_cast<CharT>(json[pos]);
          ++pos;
        }
      }

      if (pos >= json.size()) return false;
      ++pos;  // Skip closing "

      Insert(str);

      skip_whitespace();
      if (pos < json.size() && json[pos] == ',') {
        ++pos;
      }
    }

    return found_closing;
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
   * Finds all strings in the trie within the specified edit distance (Levenshtein distance)
   * from the given string. Returns a vector of pairs: (matched_string, edit_distance).
   *
   * @param query The query string to match against
   * @param max_distance Maximum allowed edit distance (insertions, deletions, substitutions)
   * @return Vector of (string, distance) pairs for all matches within max_distance
   */
  std::vector<std::pair<string_type, int>> MatchFuzzy(const string_type& query,
                                                       int max_distance) const {
    std::vector<std::pair<string_type, int>> results;

    if (max_distance < 0) return results;

    // Initialize the first row of the DP table (distance from empty string)
    std::vector<int> current_row(query.size() + 1);
    for (std::size_t i = 0; i <= query.size(); ++i) {
      current_row[i] = static_cast<int>(i);
    }

    // Traverse the trie using DFS
    for (const auto& pair : root_->Children()) {
      FuzzySearchRecursive(pair.second.get(), pair.first, query, string_type(),
                          current_row, max_distance, results);
    }

    return results;
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

  /**
   * Recursive helper for fuzzy matching using edit distance.
   * Uses dynamic programming to compute Levenshtein distance while traversing.
   */
  void FuzzySearchRecursive(TrieNode* node, CharT ch, const string_type& query,
                           const string_type& current_str,
                           const std::vector<int>& previous_row,
                           int max_distance,
                           std::vector<std::pair<string_type, int>>& results) const {
    // Check if this node is a string terminator FIRST
    if (node->Key() == CharT()) {
      // This is a string terminator - current_str is the complete string
      // Use previous_row since we haven't actually added a character
      int final_distance = previous_row[query.size()];
      if (final_distance <= max_distance) {
        results.push_back(std::make_pair(current_str, final_distance));
      }
      return;  // Don't recurse from terminators
    }

    std::size_t query_len = query.size();
    std::vector<int> current_row(query_len + 1);

    // First column: edit distance from empty query to current prefix
    current_row[0] = previous_row[0] + 1;

    // Compute edit distance for each query position
    for (std::size_t i = 1; i <= query_len; ++i) {
      // Cost of substitution (or match if characters are equal)
      int substitute_cost = previous_row[i - 1];
      if (query[i - 1] != ch) {
        substitute_cost += 1;
      }

      // Minimum of: delete from trie, delete from query, substitute
      current_row[i] = std::min({
          current_row[i - 1] + 1,      // Insert into trie (delete from query)
          previous_row[i] + 1,          // Delete from trie (insert into query)
          substitute_cost               // Substitute or match
      });
    }

    // Pruning: if minimum distance in current row exceeds max_distance, stop
    int min_distance = current_row[0];
    for (std::size_t i = 1; i <= query_len; ++i) {
      min_distance = std::min(min_distance, current_row[i]);
    }
    if (min_distance > max_distance) {
      return;  // This branch cannot yield results within max_distance
    }

    // Build the current string by adding this character
    string_type new_str = current_str + ch;

    // Recurse to children
    for (const auto& pair : node->Children()) {
      FuzzySearchRecursive(pair.second.get(), pair.first, query, new_str,
                          current_row, max_distance, results);
    }
  }

  std::unique_ptr<TrieNode> root_;

};  // class PrefixTrieBase

// Type aliases for common character types
using PrefixTrie = PrefixTrieBase<char>;
using WPrefixTrie = PrefixTrieBase<wchar_t>;

#endif  // PREFIX_TRIE_TEMPLATED_H__
