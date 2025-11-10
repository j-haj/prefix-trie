#include <algorithm>
#include <cstddef>
#include <functional>
#include <iostream>
#include <memory>
#include <set>
#include <sstream>
#include <stack>
#include <string>
#include <vector>

#include "prefix_trie.h"

// MatchResult iterator implementations
PrefixTrie::Iterator PrefixTrie::MatchResult::begin() const {
  return Iterator(&matches_, 0);
}

PrefixTrie::Iterator PrefixTrie::MatchResult::end() const {
  return Iterator(&matches_, matches_.size());
}

void PrefixTrie::Insert(const std::string& s) noexcept {
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
    runner->Children()[s[cur_index]] = std::make_unique<TrieNode>(s[cur_index]);
    runner = runner->Children()[s[cur_index]].get();
    ++cur_index;
  }
  runner->Children()[s[cur_index - 1]] = std::make_unique<TrieNode>('\0');
}

void PrefixTrie::Remove(const std::string& s) noexcept {
  if (s.empty()) return;

  // Build path to the string while traversing
  std::vector<std::pair<TrieNode*, char>> path;
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
  // Using the last character as key for the '\0' marker child
  if (runner->Children().find(s[cur_index - 1]) == runner->Children().end())
    return;  // Not a complete string

  // Remove the '\0' marker
  runner->Children().erase(s[cur_index - 1]);

  // Walk back up the path, removing nodes that have no children
  for (auto it = path.rbegin(); it != path.rend(); ++it) {
    TrieNode* parent = it->first;
    char key = it->second;
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

void PrefixTrie::Clear() noexcept {
  root_ = std::make_unique<TrieNode>();
}

std::size_t PrefixTrie::Size() const noexcept {
  return Count("");
}

std::size_t PrefixTrie::Count(const std::string& prefix) const noexcept {
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
      if (runner->Children().find(prefix[cur_index]) == runner->Children().end())
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
    if (node->Key() == '\0') {
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

PrefixTrie::MatchResult PrefixTrie::Matches(const std::string& prefix) const noexcept {
  std::vector<std::string> matches;
  MatchBackInserter(matches, prefix);
  return MatchResult(matches);
}

PrefixTrie::Stats PrefixTrie::GetStats() const noexcept {
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
    if (node->Key() == '\0') {
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
    stats.avg_branching_factor = static_cast<double>(total_children) / non_leaf_nodes;
  }

  // Estimate memory usage
  // Each node has: key (char), unique_ptr overhead, unordered_map
  const std::size_t node_overhead = sizeof(TrieNode);
  const std::size_t map_entry_overhead = sizeof(char) + sizeof(std::unique_ptr<TrieNode>);
  stats.memory_bytes = stats.num_nodes * node_overhead +
                       total_children * map_entry_overhead;

  return stats;
}

std::string PrefixTrie::Visualize() const {
  std::ostringstream oss;
  oss << "Root\n";

  // Helper lambda for recursive visualization
  std::function<void(TrieNode*, const std::string&, std::size_t)> visualize_node;
  visualize_node = [&](TrieNode* node, const std::string& prefix, std::size_t index) {
    const auto& children = node->Children();
    std::size_t i = 0;
    for (const auto& pair : children) {
      char key = pair.first;
      const std::unique_ptr<TrieNode>& child = pair.second;

      bool is_last = (i == children.size() - 1);
      std::string connector = is_last ? "└── " : "├── ";
      std::string child_prefix = prefix + (is_last ? "    " : "│   ");

      if (child->Key() == '\0') {
        oss << prefix << connector << "[END]\n";
      } else {
        oss << prefix << connector << child->Key();

        // Check if this node represents a complete string
        bool has_terminator = child->Children().find(child->Key()) !=
                              child->Children().end() &&
                              child->Children()[child->Key()]->Key() == '\0';

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

  visualize_node(root_.get(), "", 0);
  return oss.str();
}

bool PrefixTrie::Contains(const std::string& s) const noexcept {
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
