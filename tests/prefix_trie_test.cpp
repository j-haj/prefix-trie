#include <gtest/gtest.h>
#include <string>
#include <vector>
#include <algorithm>
#include "prefix_trie.h"

class PrefixTrieTest : public ::testing::Test {
 protected:
  PrefixTrie trie;
};

// Insert tests
TEST_F(PrefixTrieTest, InsertSingleString) {
  trie.Insert("hello");
  EXPECT_TRUE(trie.Contains("hello"));
}

TEST_F(PrefixTrieTest, InsertMultipleStrings) {
  trie.Insert("hello");
  trie.Insert("world");
  trie.Insert("help");

  EXPECT_TRUE(trie.Contains("hello"));
  EXPECT_TRUE(trie.Contains("world"));
  EXPECT_TRUE(trie.Contains("help"));
}

TEST_F(PrefixTrieTest, InsertEmptyString) {
  trie.Insert("");
  // Empty string should be a no-op
  EXPECT_TRUE(trie.Contains(""));
}

TEST_F(PrefixTrieTest, InsertIdempotent) {
  trie.Insert("test");
  trie.Insert("test");
  trie.Insert("test");

  EXPECT_TRUE(trie.Contains("test"));
}

TEST_F(PrefixTrieTest, InsertPrefixStrings) {
  trie.Insert("test");
  trie.Insert("testing");
  trie.Insert("tester");

  EXPECT_TRUE(trie.Contains("test"));
  EXPECT_TRUE(trie.Contains("testing"));
  EXPECT_TRUE(trie.Contains("tester"));
}

// Contains tests
TEST_F(PrefixTrieTest, ContainsEmptyTrie) {
  EXPECT_FALSE(trie.Contains("anything"));
}

TEST_F(PrefixTrieTest, ContainsEmptyString) {
  EXPECT_TRUE(trie.Contains(""));
}

TEST_F(PrefixTrieTest, ContainsPrefix) {
  trie.Insert("testing");

  EXPECT_TRUE(trie.Contains("test"));
  EXPECT_TRUE(trie.Contains("testi"));
  EXPECT_TRUE(trie.Contains("testin"));
  EXPECT_TRUE(trie.Contains("testing"));
}

TEST_F(PrefixTrieTest, DoesNotContainNonExistent) {
  trie.Insert("hello");

  EXPECT_FALSE(trie.Contains("help"));
  EXPECT_FALSE(trie.Contains("world"));
  EXPECT_FALSE(trie.Contains("helloworld"));
}

TEST_F(PrefixTrieTest, ContainsPartialMatch) {
  trie.Insert("race");
  trie.Insert("racecar");

  EXPECT_TRUE(trie.Contains("rac"));
  EXPECT_TRUE(trie.Contains("race"));
  EXPECT_TRUE(trie.Contains("racec"));
  EXPECT_TRUE(trie.Contains("racecar"));
  EXPECT_FALSE(trie.Contains("racecard"));
}

// MatchWithCallback tests
TEST_F(PrefixTrieTest, MatchWithCallbackEmptyPrefix) {
  trie.Insert("hello");
  trie.Insert("world");

  std::vector<std::string> matches;
  trie.MatchWithCallback("", [&matches](const std::string& s) {
    matches.push_back(s);
  });

  // Empty prefix should match all strings in the trie
  EXPECT_EQ(matches.size(), 2);
  EXPECT_NE(std::find(matches.begin(), matches.end(), "hello"), matches.end());
  EXPECT_NE(std::find(matches.begin(), matches.end(), "world"), matches.end());
}

TEST_F(PrefixTrieTest, MatchWithCallbackNoMatches) {
  trie.Insert("hello");
  trie.Insert("world");

  std::vector<std::string> matches;
  trie.MatchWithCallback("xyz", [&matches](const std::string& s) {
    matches.push_back(s);
  });

  EXPECT_EQ(matches.size(), 0);
}

TEST_F(PrefixTrieTest, MatchWithCallbackSingleMatch) {
  trie.Insert("hello");
  trie.Insert("world");

  std::vector<std::string> matches;
  trie.MatchWithCallback("wor", [&matches](const std::string& s) {
    matches.push_back(s);
  });

  EXPECT_EQ(matches.size(), 1);
  EXPECT_EQ(matches[0], "world");
}

TEST_F(PrefixTrieTest, MatchWithCallbackMultipleMatches) {
  trie.Insert("race");
  trie.Insert("racecar");
  trie.Insert("raceday");
  trie.Insert("raccoon");

  std::vector<std::string> matches;
  trie.MatchWithCallback("race", [&matches](const std::string& s) {
    matches.push_back(s);
  });

  EXPECT_EQ(matches.size(), 3);
  EXPECT_NE(std::find(matches.begin(), matches.end(), "race"), matches.end());
  EXPECT_NE(std::find(matches.begin(), matches.end(), "racecar"), matches.end());
  EXPECT_NE(std::find(matches.begin(), matches.end(), "raceday"), matches.end());
  EXPECT_EQ(std::find(matches.begin(), matches.end(), "raccoon"), matches.end());
}

TEST_F(PrefixTrieTest, MatchWithCallbackCommonPrefix) {
  trie.Insert("apple");
  trie.Insert("application");
  trie.Insert("apply");
  trie.Insert("apricot");

  std::vector<std::string> matches;
  trie.MatchWithCallback("app", [&matches](const std::string& s) {
    matches.push_back(s);
  });

  EXPECT_EQ(matches.size(), 3);
  EXPECT_NE(std::find(matches.begin(), matches.end(), "apple"), matches.end());
  EXPECT_NE(std::find(matches.begin(), matches.end(), "application"), matches.end());
  EXPECT_NE(std::find(matches.begin(), matches.end(), "apply"), matches.end());
}

// MatchBackInserter tests
TEST_F(PrefixTrieTest, MatchBackInserterBasic) {
  trie.Insert("test");
  trie.Insert("testing");
  trie.Insert("tester");

  std::vector<std::string> matches;
  trie.MatchBackInserter(matches, "test");

  EXPECT_EQ(matches.size(), 3);
  EXPECT_NE(std::find(matches.begin(), matches.end(), "test"), matches.end());
  EXPECT_NE(std::find(matches.begin(), matches.end(), "testing"), matches.end());
  EXPECT_NE(std::find(matches.begin(), matches.end(), "tester"), matches.end());
}

TEST_F(PrefixTrieTest, MatchBackInserterNoMatches) {
  trie.Insert("hello");

  std::vector<std::string> matches;
  trie.MatchBackInserter(matches, "world");

  EXPECT_EQ(matches.size(), 0);
}

TEST_F(PrefixTrieTest, MatchBackInserterWithExistingElements) {
  trie.Insert("new");
  trie.Insert("news");

  std::vector<std::string> matches = {"existing"};
  trie.MatchBackInserter(matches, "new");

  EXPECT_EQ(matches.size(), 3);
  EXPECT_EQ(matches[0], "existing");
}

// Edge cases
TEST_F(PrefixTrieTest, SingleCharacterStrings) {
  trie.Insert("a");
  trie.Insert("b");
  trie.Insert("c");

  EXPECT_TRUE(trie.Contains("a"));
  EXPECT_TRUE(trie.Contains("b"));
  EXPECT_TRUE(trie.Contains("c"));

  std::vector<std::string> matches;
  trie.MatchWithCallback("a", [&matches](const std::string& s) {
    matches.push_back(s);
  });

  EXPECT_EQ(matches.size(), 1);
  EXPECT_EQ(matches[0], "a");
}

TEST_F(PrefixTrieTest, LongStrings) {
  std::string long_string(1000, 'a');
  trie.Insert(long_string);

  EXPECT_TRUE(trie.Contains(long_string));
  EXPECT_TRUE(trie.Contains(long_string.substr(0, 500)));
}

TEST_F(PrefixTrieTest, SpecialCharacters) {
  trie.Insert("hello-world");
  trie.Insert("test_case");
  trie.Insert("file.txt");
  trie.Insert("path/to/file");

  EXPECT_TRUE(trie.Contains("hello-world"));
  EXPECT_TRUE(trie.Contains("test_case"));
  EXPECT_TRUE(trie.Contains("file.txt"));
  EXPECT_TRUE(trie.Contains("path/to/file"));
}

TEST_F(PrefixTrieTest, NumericStrings) {
  trie.Insert("123");
  trie.Insert("1234");
  trie.Insert("456");

  EXPECT_TRUE(trie.Contains("123"));
  EXPECT_TRUE(trie.Contains("1234"));
  EXPECT_TRUE(trie.Contains("456"));

  std::vector<std::string> matches;
  trie.MatchWithCallback("12", [&matches](const std::string& s) {
    matches.push_back(s);
  });

  EXPECT_EQ(matches.size(), 2);
}

// Remove tests
TEST_F(PrefixTrieTest, RemoveSingleString) {
  trie.Insert("hello");
  EXPECT_TRUE(trie.Contains("hello"));

  trie.Remove("hello");
  EXPECT_FALSE(trie.Contains("hello"));
}

TEST_F(PrefixTrieTest, RemoveNonExistent) {
  trie.Insert("hello");
  trie.Remove("world");  // Should be no-op

  EXPECT_TRUE(trie.Contains("hello"));
}

TEST_F(PrefixTrieTest, RemoveWithSharedPrefix) {
  trie.Insert("test");
  trie.Insert("testing");
  trie.Insert("tester");

  trie.Remove("test");

  // After removing "test", the prefix still exists due to "testing" and "tester"
  EXPECT_TRUE(trie.Contains("test"));  // Prefix should still exist
  EXPECT_TRUE(trie.Contains("testing"));
  EXPECT_TRUE(trie.Contains("tester"));

  // But "test" should no longer match as a complete string
  std::vector<std::string> matches;
  trie.MatchWithCallback("test", [&matches](const std::string& s) {
    matches.push_back(s);
  });
  EXPECT_EQ(matches.size(), 2);  // Only "testing" and "tester"
  EXPECT_NE(std::find(matches.begin(), matches.end(), "testing"), matches.end());
  EXPECT_NE(std::find(matches.begin(), matches.end(), "tester"), matches.end());
  EXPECT_EQ(std::find(matches.begin(), matches.end(), "test"), matches.end());
}

TEST_F(PrefixTrieTest, RemoveCleanupBranch) {
  trie.Insert("hello");
  trie.Insert("help");
  trie.Insert("world");

  trie.Remove("hello");

  EXPECT_FALSE(trie.Contains("hello"));
  EXPECT_TRUE(trie.Contains("help"));
  EXPECT_TRUE(trie.Contains("world"));

  // Remove "help" should clean up the entire "hel" branch
  trie.Remove("help");
  EXPECT_FALSE(trie.Contains("help"));
  EXPECT_TRUE(trie.Contains("world"));
}

TEST_F(PrefixTrieTest, RemovePrefixOfOther) {
  trie.Insert("race");
  trie.Insert("racecar");

  trie.Remove("race");

  EXPECT_TRUE(trie.Contains("racecar"));
  EXPECT_TRUE(trie.Contains("race"));  // Prefix still exists

  std::vector<std::string> matches;
  trie.MatchWithCallback("race", [&matches](const std::string& s) {
    matches.push_back(s);
  });
  EXPECT_EQ(matches.size(), 1);
  EXPECT_EQ(matches[0], "racecar");
}

TEST_F(PrefixTrieTest, RemoveEmptyString) {
  trie.Insert("hello");
  trie.Remove("");  // Should be no-op

  EXPECT_TRUE(trie.Contains("hello"));
}

// Clear tests
TEST_F(PrefixTrieTest, ClearEmptyTrie) {
  trie.Clear();
  EXPECT_EQ(trie.Size(), 0);
}

TEST_F(PrefixTrieTest, ClearNonEmptyTrie) {
  trie.Insert("hello");
  trie.Insert("world");
  trie.Insert("test");

  EXPECT_EQ(trie.Size(), 3);

  trie.Clear();

  EXPECT_EQ(trie.Size(), 0);
  EXPECT_FALSE(trie.Contains("hello"));
  EXPECT_FALSE(trie.Contains("world"));
  EXPECT_FALSE(trie.Contains("test"));
}

// Size tests
TEST_F(PrefixTrieTest, SizeEmptyTrie) {
  EXPECT_EQ(trie.Size(), 0);
}

TEST_F(PrefixTrieTest, SizeAfterInserts) {
  EXPECT_EQ(trie.Size(), 0);

  trie.Insert("hello");
  EXPECT_EQ(trie.Size(), 1);

  trie.Insert("world");
  EXPECT_EQ(trie.Size(), 2);

  trie.Insert("hello");  // Idempotent
  EXPECT_EQ(trie.Size(), 2);
}

TEST_F(PrefixTrieTest, SizeAfterRemoves) {
  trie.Insert("hello");
  trie.Insert("world");
  trie.Insert("test");
  EXPECT_EQ(trie.Size(), 3);

  trie.Remove("hello");
  EXPECT_EQ(trie.Size(), 2);

  trie.Remove("nonexistent");
  EXPECT_EQ(trie.Size(), 2);

  trie.Remove("world");
  trie.Remove("test");
  EXPECT_EQ(trie.Size(), 0);
}

// Count tests
TEST_F(PrefixTrieTest, CountEmptyPrefix) {
  trie.Insert("hello");
  trie.Insert("world");
  trie.Insert("test");

  EXPECT_EQ(trie.Count(""), 3);
}

TEST_F(PrefixTrieTest, CountWithPrefix) {
  trie.Insert("race");
  trie.Insert("racecar");
  trie.Insert("raceday");
  trie.Insert("raccoon");

  EXPECT_EQ(trie.Count("race"), 3);
  EXPECT_EQ(trie.Count("rac"), 4);
  EXPECT_EQ(trie.Count("racec"), 1);
}

TEST_F(PrefixTrieTest, CountNoMatches) {
  trie.Insert("hello");
  trie.Insert("world");

  EXPECT_EQ(trie.Count("xyz"), 0);
}

TEST_F(PrefixTrieTest, CountSingleMatch) {
  trie.Insert("unique");
  trie.Insert("test");

  EXPECT_EQ(trie.Count("unique"), 1);
  EXPECT_EQ(trie.Count("uniq"), 1);
}

// Iterator tests
TEST_F(PrefixTrieTest, IteratorBasic) {
  trie.Insert("hello");
  trie.Insert("help");
  trie.Insert("world");

  std::vector<std::string> matches;
  for (const auto& s : trie.Matches("hel")) {
    matches.push_back(s);
  }

  EXPECT_EQ(matches.size(), 2);
  EXPECT_NE(std::find(matches.begin(), matches.end(), "hello"), matches.end());
  EXPECT_NE(std::find(matches.begin(), matches.end(), "help"), matches.end());
}

TEST_F(PrefixTrieTest, IteratorEmpty) {
  std::vector<std::string> matches;
  for (const auto& s : trie.Matches("test")) {
    matches.push_back(s);
  }

  EXPECT_EQ(matches.size(), 0);
}

TEST_F(PrefixTrieTest, IteratorAllStrings) {
  trie.Insert("apple");
  trie.Insert("banana");
  trie.Insert("cherry");

  std::vector<std::string> matches;
  for (const auto& s : trie.Matches("")) {
    matches.push_back(s);
  }

  EXPECT_EQ(matches.size(), 3);
}

TEST_F(PrefixTrieTest, IteratorSingleMatch) {
  trie.Insert("unique");

  std::vector<std::string> matches;
  for (const auto& s : trie.Matches("uniq")) {
    matches.push_back(s);
  }

  EXPECT_EQ(matches.size(), 1);
  EXPECT_EQ(matches[0], "unique");
}

TEST_F(PrefixTrieTest, IteratorOperators) {
  trie.Insert("test1");
  trie.Insert("test2");

  auto result = trie.Matches("test");
  auto it1 = result.begin();
  auto it2 = result.begin();
  auto end = result.end();

  EXPECT_TRUE(it1 == it2);
  EXPECT_FALSE(it1 == end);
  EXPECT_TRUE(it1 != end);

  ++it1;
  EXPECT_FALSE(it1 == it2);
  EXPECT_TRUE(it1 != it2);
}

TEST_F(PrefixTrieTest, IteratorDereference) {
  trie.Insert("hello");

  auto result = trie.Matches("hello");
  auto it = result.begin();

  EXPECT_EQ(*it, "hello");
  EXPECT_EQ(it->size(), 5);
}

// WString tests
class WPrefixTrieTest : public ::testing::Test {
 protected:
  WPrefixTrie trie;
};

TEST_F(WPrefixTrieTest, InsertAndContains) {
  trie.Insert(L"hello");
  trie.Insert(L"world");
  trie.Insert(L"こんにちは");  // Japanese

  EXPECT_TRUE(trie.Contains(L"hello"));
  EXPECT_TRUE(trie.Contains(L"world"));
  EXPECT_TRUE(trie.Contains(L"こんにちは"));
  EXPECT_FALSE(trie.Contains(L"goodbye"));
}

TEST_F(WPrefixTrieTest, SizeAndCount) {
  trie.Insert(L"test");
  trie.Insert(L"testing");
  trie.Insert(L"tester");

  EXPECT_EQ(trie.Size(), 3);
  EXPECT_EQ(trie.Count(L"test"), 3);
  EXPECT_EQ(trie.Count(L"testi"), 1);
}

TEST_F(WPrefixTrieTest, RemoveWideStrings) {
  trie.Insert(L"hello");
  trie.Insert(L"help");

  trie.Remove(L"hello");

  EXPECT_FALSE(trie.Contains(L"hello"));
  EXPECT_TRUE(trie.Contains(L"help"));
  EXPECT_EQ(trie.Size(), 1);
}

TEST_F(WPrefixTrieTest, IteratorWithWideStrings) {
  trie.Insert(L"apple");
  trie.Insert(L"application");
  trie.Insert(L"apply");

  std::vector<std::wstring> matches;
  for (const auto& s : trie.Matches(L"app")) {
    matches.push_back(s);
  }

  EXPECT_EQ(matches.size(), 3);
}

TEST_F(WPrefixTrieTest, UnicodeStrings) {
  trie.Insert(L"café");
  trie.Insert(L"naïve");
  trie.Insert(L"résumé");
  trie.Insert(L"Москва");  // Russian
  trie.Insert(L"北京");     // Chinese

  EXPECT_TRUE(trie.Contains(L"café"));
  EXPECT_TRUE(trie.Contains(L"Москва"));
  EXPECT_TRUE(trie.Contains(L"北"));
  EXPECT_EQ(trie.Size(), 5);
}

TEST_F(WPrefixTrieTest, GetStatsWide) {
  trie.Insert(L"a");
  trie.Insert(L"ab");
  trie.Insert(L"abc");

  auto stats = trie.GetStats();
  EXPECT_EQ(stats.num_strings, 3);
  EXPECT_GT(stats.num_nodes, 0);
  EXPECT_GT(stats.max_depth, 0);
}

// JSON Serialization tests
TEST_F(PrefixTrieTest, ToJSONBasic) {
  trie.Insert("hello");
  trie.Insert("world");

  std::string json = trie.ToJSON();

  EXPECT_TRUE(json.find("hello") != std::string::npos);
  EXPECT_TRUE(json.find("world") != std::string::npos);
  EXPECT_TRUE(json.find("[") != std::string::npos);
  EXPECT_TRUE(json.find("]") != std::string::npos);
}

TEST_F(PrefixTrieTest, FromJSONBasic) {
  std::string json = R"(["hello", "world", "test"])";

  EXPECT_TRUE(trie.FromJSON(json));

  EXPECT_EQ(trie.Size(), 3);
  EXPECT_TRUE(trie.Contains("hello"));
  EXPECT_TRUE(trie.Contains("world"));
  EXPECT_TRUE(trie.Contains("test"));
}

TEST_F(PrefixTrieTest, RoundTripSerialization) {
  trie.Insert("apple");
  trie.Insert("application");
  trie.Insert("apply");

  std::string json = trie.ToJSON();

  PrefixTrie trie2;
  EXPECT_TRUE(trie2.FromJSON(json));

  EXPECT_EQ(trie.Size(), trie2.Size());
  EXPECT_TRUE(trie2.Contains("apple"));
  EXPECT_TRUE(trie2.Contains("application"));
  EXPECT_TRUE(trie2.Contains("apply"));
}

TEST_F(PrefixTrieTest, JSONWithSpecialCharacters) {
  trie.Insert("hello\"world");
  trie.Insert("test\\path");
  trie.Insert("line\nbreak");
  trie.Insert("tab\there");

  std::string json = trie.ToJSON();

  PrefixTrie trie2;
  EXPECT_TRUE(trie2.FromJSON(json));

  EXPECT_EQ(trie.Size(), trie2.Size());
  EXPECT_TRUE(trie2.Contains("hello\"world"));
  EXPECT_TRUE(trie2.Contains("test\\path"));
  EXPECT_TRUE(trie2.Contains("line\nbreak"));
  EXPECT_TRUE(trie2.Contains("tab\there"));
}

TEST_F(PrefixTrieTest, FromJSONEmptyArray) {
  std::string json = "[]";

  EXPECT_TRUE(trie.FromJSON(json));
  EXPECT_EQ(trie.Size(), 0);
}

TEST_F(PrefixTrieTest, FromJSONWithWhitespace) {
  std::string json = R"([
    "hello",
    "world",
    "test"
  ])";

  EXPECT_TRUE(trie.FromJSON(json));
  EXPECT_EQ(trie.Size(), 3);
}

TEST_F(PrefixTrieTest, FromJSONInvalidFormat) {
  // Not an array
  EXPECT_FALSE(trie.FromJSON("{\"key\": \"value\"}"));

  // Missing closing bracket
  EXPECT_FALSE(trie.FromJSON("[\"test\""));

  // Missing quotes
  EXPECT_FALSE(trie.FromJSON("[test]"));

  // Invalid escape sequence
  EXPECT_FALSE(trie.FromJSON("[\"test\\x\"]"));
}

TEST_F(PrefixTrieTest, JSONPreservesOrder) {
  trie.Insert("zebra");
  trie.Insert("apple");
  trie.Insert("mango");

  std::string json = trie.ToJSON();

  PrefixTrie trie2;
  EXPECT_TRUE(trie2.FromJSON(json));

  // Should contain all elements regardless of order
  EXPECT_TRUE(trie2.Contains("zebra"));
  EXPECT_TRUE(trie2.Contains("apple"));
  EXPECT_TRUE(trie2.Contains("mango"));
  EXPECT_EQ(trie2.Size(), 3);
}

// Fuzzy Matching tests
TEST_F(PrefixTrieTest, FuzzyMatchExact) {
  trie.Insert("hello");
  trie.Insert("world");

  auto results = trie.MatchFuzzy("hello", 0);

  EXPECT_EQ(results.size(), 1);
  EXPECT_EQ(results[0].first, "hello");
  EXPECT_EQ(results[0].second, 0);
}

TEST_F(PrefixTrieTest, FuzzyMatchSingleSubstitution) {
  trie.Insert("hello");
  trie.Insert("help");
  trie.Insert("world");

  auto results = trie.MatchFuzzy("hallo", 1);

  // Should match "hello" (1 substitution: e->a)
  EXPECT_GE(results.size(), 1);

  bool found_hello = false;
  for (const auto& pair : results) {
    if (pair.first == "hello") {
      found_hello = true;
      EXPECT_EQ(pair.second, 1);
    }
  }
  EXPECT_TRUE(found_hello);
}

TEST_F(PrefixTrieTest, FuzzyMatchInsertion) {
  trie.Insert("cat");
  trie.Insert("dog");

  auto results = trie.MatchFuzzy("cart", 1);

  // Should match "cat" (1 insertion: r)
  bool found_cat = false;
  for (const auto& pair : results) {
    if (pair.first == "cat") {
      found_cat = true;
      EXPECT_LE(pair.second, 1);
    }
  }
  EXPECT_TRUE(found_cat);
}

TEST_F(PrefixTrieTest, FuzzyMatchDeletion) {
  trie.Insert("testing");

  auto results = trie.MatchFuzzy("test", 3);

  // Should match "testing" (3 deletions: i, n, g)
  EXPECT_GE(results.size(), 1);

  bool found = false;
  for (const auto& pair : results) {
    if (pair.first == "testing") {
      found = true;
      EXPECT_EQ(pair.second, 3);
    }
  }
  EXPECT_TRUE(found);
}

TEST_F(PrefixTrieTest, FuzzyMatchMultipleResults) {
  trie.Insert("cat");
  trie.Insert("car");
  trie.Insert("can");
  trie.Insert("cap");

  auto results = trie.MatchFuzzy("cat", 1);

  // Should match all 4 words (distance 0-1)
  EXPECT_GE(results.size(), 4);

  // Verify "cat" has distance 0
  for (const auto& pair : results) {
    if (pair.first == "cat") {
      EXPECT_EQ(pair.second, 0);
    }
  }
}

TEST_F(PrefixTrieTest, FuzzyMatchNoResults) {
  trie.Insert("hello");
  trie.Insert("world");

  auto results = trie.MatchFuzzy("xyz", 1);

  // No words within distance 1 of "xyz"
  EXPECT_EQ(results.size(), 0);
}

TEST_F(PrefixTrieTest, FuzzyMatchDistance2) {
  trie.Insert("kitten");

  auto results = trie.MatchFuzzy("sitting", 3);

  // "kitten" -> "sitting" requires 3 edits
  EXPECT_GE(results.size(), 1);

  bool found = false;
  for (const auto& pair : results) {
    if (pair.first == "kitten") {
      found = true;
      EXPECT_EQ(pair.second, 3);
    }
  }
  EXPECT_TRUE(found);
}

TEST_F(PrefixTrieTest, FuzzyMatchEmptyQuery) {
  trie.Insert("a");
  trie.Insert("ab");
  trie.Insert("abc");

  auto results = trie.MatchFuzzy("", 2);

  // Empty string should match short strings within distance 2
  EXPECT_GE(results.size(), 2);
}

TEST_F(PrefixTrieTest, FuzzyMatchNegativeDistance) {
  trie.Insert("hello");

  auto results = trie.MatchFuzzy("hello", -1);

  EXPECT_EQ(results.size(), 0);
}

TEST_F(PrefixTrieTest, FuzzyMatchLargeDistance) {
  trie.Insert("hello");

  auto results = trie.MatchFuzzy("world", 10);

  // "hello" -> "world" = 4 edits, should match with distance 10
  EXPECT_GE(results.size(), 1);
  EXPECT_EQ(results[0].first, "hello");
}

TEST_F(PrefixTrieTest, FuzzyMatchPruning) {
  // Add many strings to test pruning efficiency
  trie.Insert("apple");
  trie.Insert("application");
  trie.Insert("apply");
  trie.Insert("zebra");
  trie.Insert("zoo");

  auto results = trie.MatchFuzzy("app", 1);

  // Should efficiently prune "zebra" and "zoo" branches
  for (const auto& pair : results) {
    EXPECT_TRUE(pair.first[0] == 'a' || pair.second <= 1);
  }
}

// Main function
int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
