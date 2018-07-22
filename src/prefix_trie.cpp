#include <cstddef>
#include <iostream>
#include <string>

#include "trie_node.h"
#include "prefix_trie.h"

void PrefixTrie::Insert(const std::string& s) noexcept {
  if (s.empty()) return;

  TrieNode* runner = root_.get();
  std::size_t cur_index = 0;
  while (!runner->Children().empty() && cur_index < s.size()) {
    std::cout << "At " << s[cur_index] << std::endl;
    runner = runner->Children()[s[cur_index]].get();
    ++cur_index;
  }
  while (cur_index < s.size()) {
    std::cout << "Creating child for key: " << s[cur_index] << std::endl;
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
    if (runner->Children().find(s[cur_index]) == runner->Children().end()) return false;
    runner = runner->Children()[s[cur_index]].get();
    ++cur_index;
  }
  return cur_index == s.size();
}
