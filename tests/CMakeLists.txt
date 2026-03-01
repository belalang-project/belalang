find_package(GTest REQUIRED)

add_executable(unit_tests TestLexer.cpp)
target_link_libraries(unit_tests belalang_lib GTest::gtest_main)

include(GoogleTest)
gtest_discover_tests(unit_tests)
