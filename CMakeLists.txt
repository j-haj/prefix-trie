project(prefix_trie)
cmake_minimum_required(VERSION 3.10)

set(CMAKE_CXX_STANDARD 14)
include_directories(${PROJECT_SOURCE_DIR}/src)
set(SOURCES
  ${PROJECT_SOURCE_DIR}/src/prefix_trie.cpp
)

set(HEADERS
  ${PROJECT_SOURCE_DIR}/src/prefix_trie.h
  ${PROJECT_SOURCE_DIR}/src/trie_node.h
)

add_executable(main ${PROJECT_SOURCE_DIR}/examples/main.cpp ${SOURCES})
