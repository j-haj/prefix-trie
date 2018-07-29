#include <cstddef>
#include <iostream>
#include <memory>
#include <set>
#include <sstream>
#include <stack>
#include <string>
#include <vector>

#include "prefix_trie.h"
#include "trie_node.h"

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
  runner->Children()[s[cur_index-1]] = std::make_unique<TrieNode>('\0');
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

void PrefixTrie::MatchWithCallback(
    const std::string& s,
    const std::function<void(const std::string&)>& callback) const {
  // Check early exit conditions
  if (s.empty()) callback("");
  else if(root_->Children().find(s[0]) == root_->Children().end()) return;

  TrieNode* runner = root_->Children()[s[0]].get();
  std::size_t cur_index = 1;

  // Traverse trie to end of prefix
  std::stringstream base;
  while (cur_index < s.size()) {
    base << runner->Key();
    // If we can't move forward the prefix must not exist, exit early
    if (runner->Children().find(s[cur_index]) == runner->Children().end()) return;
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
        nodes.push(std::make_pair(tmp.first+1, c.second.get()));
      }
    }
  }
}
