#include <iostream>
#include <string>

#include "prefix_trie.h"

int main() {
  std::string test1 = "race";
  std::string test2 = "racecar";
  PrefixTrie pt;
  pt.Insert(test1);
  pt.Insert(test2);
  pt.Insert("raceday");
  pt.Insert("raccoon");
  std::cout << "Added:\n"
            << "\trace\n"
            << "\tracecar\n"
            << "\traceday\n"
            << "\traccoon\n";
  std::cout << "pt.Contains(race) (expected true): " << pt.Contains("race")
            << std::endl;
  std::cout << "pt.Contains(racet) (expected false): " << pt.Contains("racet")
            << std::endl;
  std::cout << "pt.Contains(racec) (expected true): " << pt.Contains("racec")
            << std::endl;
  std::cout << "pt.Cotnains(racecar) (expected true): "
            << pt.Contains("racecar") << std::endl;
  std::cout << "Attemtpting to match on 'ra'\n";
  pt.MatchWithCallback("ra", [](const std::string& s) {
    std::cout << "Matched: " << s << std::endl;
  });
  return 0;
}
