#ifndef PREFIX_TRIE_H__
#define PREFIX_TRIE_H__
#include <string>
#include <memory>

class TrieNode;

class PrefixTrie {
  public:

    /**
     * Inserts the string into the prefix trie. This method is idempotent.
     */
    void Insert(const std::string& s) noexcept;

    /**
     * Check if prefix trie contains string.
     */
    bool Contains(const std::string& s) const noexcept;

  private:
    std::unique_ptr<TrieNode> root_;

}; // class PrefixTrie

#endif // PREFIX_TRIE_H__
