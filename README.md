# String Prefix Trie

A modern C++14 implementation of a prefix trie (also known as a trie or prefix tree) for `std::string`s with comprehensive functionality for string storage, search, and analysis.

## Features

### Core Operations
* **Insert** - Add strings to the trie (idempotent)
* **Remove** - Delete strings from the trie with automatic branch cleanup
* **Contains** - Check if the trie contains a given prefix
* **Clear** - Remove all strings from the trie
* **Size** - Get the total number of strings stored
* **Count** - Count strings matching a given prefix

### Search and Iteration
* **MatchWithCallback** - Call a callback function for all strings matching a prefix
* **MatchBackInserter** - Insert matching strings into a container
* **Matches** - Modern C++ iterator interface for range-based for loops

### Diagnostics and Debugging
* **GetStats** - Retrieve trie statistics (node count, depth, branching factor, memory usage)
* **Visualize** - Generate ASCII tree visualization for debugging

## Building

This project uses CMake with C++14 standard and includes Google Test for testing.

```bash
# Create build directory and configure
mkdir build && cd build
cmake ..

# Build the project
cmake --build .

# Run the example
./main

# Run tests
./prefix_trie_tests
```

## Usage Examples

### Basic Operations

```cpp
#include "prefix_trie.h"

PrefixTrie trie;

// Insert strings
trie.Insert("hello");
trie.Insert("world");
trie.Insert("help");

// Check if prefix exists
if (trie.Contains("hel")) {
    // "hel" is a valid prefix
}

// Get statistics
std::cout << "Total strings: " << trie.Size() << std::endl;
std::cout << "Strings starting with 'hel': " << trie.Count("hel") << std::endl;

// Remove a string
trie.Remove("hello");
```

### Using Iterators (Modern C++)

```cpp
// Range-based for loop
for (const auto& str : trie.Matches("hel")) {
    std::cout << str << std::endl;
}

// Works with algorithms
auto matches = trie.Matches("wor");
auto it = std::find(matches.begin(), matches.end(), "world");
```

### Using Callbacks

```cpp
// Custom callback
trie.MatchWithCallback("hel", [](const std::string& s) {
    std::cout << "Found: " << s << std::endl;
});

// Back inserter
std::vector<std::string> results;
trie.MatchBackInserter(results, "hel");
```

### Diagnostics

```cpp
// Get detailed statistics
auto stats = trie.GetStats();
std::cout << "Nodes: " << stats.num_nodes << std::endl;
std::cout << "Max depth: " << stats.max_depth << std::endl;
std::cout << "Avg branching factor: " << stats.avg_branching_factor << std::endl;
std::cout << "Memory usage: " << stats.memory_bytes << " bytes" << std::endl;

// Visualize structure
std::cout << trie.Visualize() << std::endl;
```

## Implementation Details

* **Memory efficient**: Uses `std::unique_ptr` for automatic memory management
* **Iterative traversal**: DFS implementation minimizes stack usage
* **Branch cleanup**: Remove operation automatically cleans up empty branches
* **Well tested**: 43+ unit tests covering all functionality

## Testing

The project uses Google Test framework. Run tests with:

```bash
cd build
./prefix_trie_tests
```
