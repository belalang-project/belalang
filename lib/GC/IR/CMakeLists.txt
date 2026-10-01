target_sources(MLIRGCDialect PRIVATE
  GCDialect.cpp
  GCOps.cpp
  GCTypes.cpp
)

if(BUILD_TESTING AND BELALANG_BUILD_TESTS)
  add_executable(MLIRGCTest GCTypesTest.cpp)
  target_link_libraries(MLIRGCTest PRIVATE MLIRGCDialect GTest::gtest_main)
  add_test(NAME MLIRGCTest COMMAND MLIRGCTest)
endif()
