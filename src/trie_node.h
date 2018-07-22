#ifndef TRIE_NODE_H__
#define TRIE_NODE_H__
#include <string>
#include <string_view>
#include <unordered_map>

class TrieNode {
  public:
    /**
     * Default constructor.
     */
    TrieNode() : TrieNode('\0')  {}

    /**
     * Constructs a TrieNode with the given key.
     */
    TrieNode(char k) : key_(k) {}

    // No copy-constructor since each TrieNode owns its children data
    TrieNode(const TrieNode& o) = delete;
    TrieNode(TrieNode&& o) : key_(o.key_), is_leaf_(o.is_leaf_) {
      for (auto& p : o.children_) {
        children_[p.first] = std::move(p.second);
      }
    }

    char Key() const noexcept { return key_; }
    bool IsLeaf() const noexcept { return is_leaf_; }

    std::unordered_map<char, std::unique_ptr<TrieNode>>& Children() noexcept { return children_; }
    /**
     * Add a child node to the vector for children TrieNodes.
     * @param s value of child node key
     */
    void AddChild(const char key, const std::string& s) noexcept {
      children_[key] = std::make_unique<TrieNode>(s);
    }
  private:
    char key_;
    bool is_leaf_;
    std::unordered_map<char, std::unique_ptr<TrieNode>> children_;
}; // class TrieNode
#endif // TRIE_NODE_H__
