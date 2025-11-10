project(prefix_trie)
cmake_minimum_required(VERSION 3.14)

set(CMAKE_CXX_STANDARD 14)
include_directories(${PROJECT_SOURCE_DIR}/src)

set(HEADERS
  ${PROJECT_SOURCE_DIR}/src/prefix_trie.h
)

add_executable(main ${PROJECT_SOURCE_DIR}/examples/main.cpp)

# Google Test setup
include(FetchContent)
FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG v1.14.0
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

enable_testing()

add_executable(
  prefix_trie_tests
  tests/prefix_trie_test.cpp
)

target_link_libraries(
  prefix_trie_tests
  GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(prefix_trie_tests)
