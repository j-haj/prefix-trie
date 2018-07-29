# String Prefix Trie
Very simple implementation of a prefix trie for `std::string`s. Operations on
the trie include:

* **insert** - add strings to the trie
* **contains** - check if the trie contains the given prefix
* **match** - call a given callback function on all strings who match the given
  prefix.
