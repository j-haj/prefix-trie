#include <cstddef>
#include <set>
#include <stack>
#include <string>

#include "prefix_trie.h"
#include "trie_node.h"

void PrefixTrie::Insert(const std::string& s) noexcept {
  if (s.empty()) return;

  TrieNode* runner = root_.get();
  std::size_t cur_index = 0;
  while (!runner->Children().empty() && cur_index < s.size()) {
    runner = runner->Children()[s[cur_index]].get();
    ++cur_index;
  }
  while (cur_index < s.size()) {
    runner->Children()[s[cur_index]] = std::make_unique<TrieNode>(s[cur_index]);
    runner = runner->Children()[s[cur_index]].get();
    ++cur_index;
  }
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
    std::function<void(const std::string&)> callback) const {
  TrieNode* runner = root_.get();
  std::size_t cur_index = 0;
  // Traverse trie to end of prefix
  while (cur_index < s.size()) {
    if (runner->Children().find(s[cur_index]) == runner->Children().end()) return;
    runner = runner->Children()[s[cur_index]].get();
    ++cur_index;
  }

  // Begin iteration over all strings who match the given prefix
  std::stack<TrieNode*> stumps;
  stumps.push(runner);
  while (!stumps.empty()) {
    auto tmp = stumps.top();
    stumps.pop(); 
  }
}

