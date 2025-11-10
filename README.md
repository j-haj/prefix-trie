# String Prefix Trie

A comprehensive, modern C++14 implementation of a prefix trie (also known as a trie or prefix tree) with support for multiple character types, fuzzy matching, serialization, and advanced search capabilities.

## Features

### Core Operations
* **Insert** - Add strings to the trie (idempotent)
* **Remove** - Delete strings with automatic branch cleanup
* **Contains** - Check if the trie contains a given prefix
* **Clear** - Remove all strings from the trie
* **Size** - Get the total number of strings stored
* **Count** - Count strings matching a given prefix

### Search and Iteration
* **MatchWithCallback** - Execute callback for all strings matching a prefix
* **MatchBackInserter** - Insert matching strings into any container
* **Matches** - Modern C++ iterator interface for range-based for loops
* **MatchFuzzy** - Find strings within edit distance (Levenshtein distance)

### Serialization
* **ToJSON** - Serialize trie to JSON format
* **FromJSON** - Deserialize trie from JSON with validation

### Diagnostics and Debugging
* **GetStats** - Retrieve trie statistics (nodes, depth, branching factor, memory)
* **Visualize** - Generate ASCII tree visualization

### Advanced Features
* **Template support** - Works with `char`, `wchar_t`, or custom character types
* **Unicode support** - Full support for wide strings and international characters
* **Fuzzy matching** - Levenshtein distance-based approximate string matching
* **Smart pruning** - Efficient search with automatic branch pruning

## Building

This project uses CMake with C++14 standard and includes Google Test for comprehensive testing.

```bash
# Create build directory and configure
mkdir build && cd build
cmake ..

# Build the project
cmake --build .

# Run the example
./main

# Run tests (68+ tests)
./prefix_trie_tests
```

## Quick Start

```cpp
#include "prefix_trie.h"

PrefixTrie trie;  // For std::string
// or
WPrefixTrie wtrie;  // For std::wstring

// Basic usage
trie.Insert("hello");
trie.Insert("world");

// Modern iteration
for (const auto& word : trie.Matches("hel")) {
    std::cout << word << std::endl;  // Prints: hello
}
```

## Usage Examples

### 1. Basic Operations

```cpp
#include "prefix_trie.h"

PrefixTrie trie;

// Insert strings
trie.Insert("apple");
trie.Insert("application");
trie.Insert("apply");

// Check containment
if (trie.Contains("app")) {
    std::cout << "Prefix 'app' exists\n";
}

// Get counts
std::cout << "Total strings: " << trie.Size() << std::endl;
std::cout << "Strings with 'app': " << trie.Count("app") << std::endl;

// Remove strings
trie.Remove("apple");
```

### 2. Modern C++ Iterators

```cpp
// Range-based for loop
for (const auto& str : trie.Matches("app")) {
    std::cout << str << std::endl;
}

// Works with STL algorithms
auto matches = trie.Matches("app");
auto it = std::find(matches.begin(), matches.end(), "application");
if (it != matches.end()) {
    std::cout << "Found: " << *it << std::endl;
}
```

### 3. Fuzzy Matching (Edit Distance)

```cpp
trie.Insert("hello");
trie.Insert("help");
trie.Insert("world");

// Find strings within edit distance 1 of "hallo"
auto results = trie.MatchFuzzy("hallo", 1);

for (const auto& [word, distance] : results) {
    std::cout << word << " (distance: " << distance << ")\n";
    // Output: hello (distance: 1)
}

// Spell checking example
auto suggestions = trie.MatchFuzzy("wrold", 2);
// Returns: world (distance: 2)
```

### 4. JSON Serialization

```cpp
// Serialize to JSON
PrefixTrie trie;
trie.Insert("cat");
trie.Insert("dog");
trie.Insert("bird");

std::string json = trie.ToJSON();
// Output: ["cat","dog","bird"]

// Save to file
std::ofstream file("trie.json");
file << json;
file.close();

// Deserialize from JSON
PrefixTrie trie2;
std::ifstream infile("trie.json");
std::string json_data((std::istreambuf_iterator<char>(infile)),
                       std::istreambuf_iterator<char>());
if (trie2.FromJSON(json_data)) {
    std::cout << "Loaded " << trie2.Size() << " strings\n";
}
```

### 5. Unicode and Wide Strings

```cpp
WPrefixTrie wtrie;  // Wide string trie

// Unicode support
wtrie.Insert(L"hello");
wtrie.Insert(L"世界");     // Chinese
wtrie.Insert(L"こんにちは");  // Japanese
wtrie.Insert(L"Москва");   // Russian

// All operations work with wide strings
std::wcout << L"Size: " << wtrie.Size() << std::endl;

for (const auto& word : wtrie.Matches(L"")) {
    std::wcout << word << std::endl;
}
```

### 6. Callbacks and Custom Processing

```cpp
// Custom callback
trie.MatchWithCallback("app", [](const std::string& s) {
    std::cout << "Found: " << s << std::endl;
});

// Back inserter into any container
std::vector<std::string> results;
trie.MatchBackInserter(results, "app");

std::set<std::string> unique_results;
trie.MatchBackInserter(unique_results, "app");
```

### 7. Statistics and Debugging

```cpp
// Get detailed statistics
auto stats = trie.GetStats();
std::cout << "Number of strings: " << stats.num_strings << "\n";
std::cout << "Number of nodes: " << stats.num_nodes << "\n";
std::cout << "Maximum depth: " << stats.max_depth << "\n";
std::cout << "Average depth: " << stats.avg_depth << "\n";
std::cout << "Avg branching factor: " << stats.avg_branching_factor << "\n";
std::cout << "Memory usage: " << stats.memory_bytes << " bytes\n";

// Visualize tree structure
std::cout << trie.Visualize() << std::endl;
/* Output:
Root
|-- a
|   |-- p
|   |   |-- p
|   |   |   |-- l
|   |   |   |   |-- e *
|   |   |   |   |   +-- [END]
*/
```

## Implementation Details

### Architecture
* **Header-only template library** - All code in `prefix_trie.h`
* **Template on character type** - `PrefixTrieBase<CharT>` supports any character type
* **Smart memory management** - Uses `std::unique_ptr` for automatic cleanup
* **Iterative DFS** - Minimizes stack usage for large tries
* **Branch cleanup** - Remove operation automatically prunes empty branches

### Performance
* **Space-efficient** - Shared prefix compression
* **Smart pruning** - Fuzzy matching prunes impossible branches early
* **Cache-friendly** - Iterative algorithms avoid deep recursion

### Testing
* **Comprehensive test suite** - 68+ unit tests with Google Test
* **100% feature coverage** - All operations thoroughly tested
* **Edge case handling** - Empty strings, Unicode, special characters

## API Reference

### Type Aliases
```cpp
using PrefixTrie = PrefixTrieBase<char>;       // For std::string
using WPrefixTrie = PrefixTrieBase<wchar_t>;   // For std::wstring
```

### Core Methods
```cpp
void Insert(const string& s);                   // Insert string
void Remove(const string& s);                   // Remove string
void Clear();                                   // Clear all strings
bool Contains(const string& s) const;           // Check prefix exists
size_t Size() const;                            // Total string count
size_t Count(const string& prefix) const;       // Prefix match count
```

### Search Methods
```cpp
MatchResult Matches(const string& prefix) const;
void MatchWithCallback(const string& prefix, Callable callback) const;
void MatchBackInserter(Container& c, const string& prefix) const;
vector<pair<string, int>> MatchFuzzy(const string& query, int max_distance) const;
```

### Serialization
```cpp
string ToJSON() const;                          // Serialize to JSON
bool FromJSON(const string& json);              // Deserialize from JSON
```

### Diagnostics
```cpp
Stats GetStats() const;                         // Get statistics
string Visualize() const;                       // ASCII visualization
```

## Use Cases

### Autocomplete Systems
```cpp
PrefixTrie dictionary;
// ... load dictionary ...

// User types "pro"
for (const auto& suggestion : dictionary.Matches("pro")) {
    std::cout << suggestion << std::endl;
}
```

### Spell Checking
```cpp
// Find similar words
auto suggestions = dictionary.MatchFuzzy("recieve", 2);
// Returns: receive (distance: 2)
```

### IP Address Routing
```cpp
// Store IP prefixes for routing tables
PrefixTrie routes;
routes.Insert("192.168.1");
routes.Insert("10.0.0");
```

### Search Suggestions
```cpp
// E-commerce product search
PrefixTrie products;
products.Insert("laptop");
products.Insert("laptop bag");
products.Insert("laptop stand");

// User searches for "lapt"
for (const auto& product : products.Matches("lapt")) {
    // Show suggestions...
}
```

## Testing

Run the comprehensive test suite:

```bash
cd build
./prefix_trie_tests
```

Test coverage includes:
- Basic operations (insert, remove, contains)
- Edge cases (empty strings, duplicates, special characters)
- Iterator functionality
- Fuzzy matching with various edit distances
- JSON serialization/deserialization
- Unicode and wide string support
- Statistics and visualization

## License

See LICENSE file for details.

## Contributing

Contributions are welcome! Please ensure all tests pass and add new tests for new features.
