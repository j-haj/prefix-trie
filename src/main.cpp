#include <iostream>
#include <string>

#include "prefix_trie.h"

int main() {
  std::string test1 = "race";
  std::string test2 = "racecar";
  PrefixTrie pt;
  pt.Insert(test1);
  pt.Insert(test2);
  std::cout << "pt.Contains(race) (expected true): " << pt.Contains("race") << std::endl;
  std::cout << "pt.Contains(racet) (expected false): " << pt.Contains("racet") << std::endl;
  std::cout << "pt.Contains(racec) (expected true): " << pt.Contains("racec") << std::endl;
  std::cout << "pt.Cotnains(racecar) (expected true): " << pt.Contains("racecar") << std::endl;
  return 0;
}
