target_sources(MLIRGCDialect PRIVATE
  GCDialect.cpp
  GCOps.cpp
  GCTypes.cpp
)

if(BUILD_TESTING AND BELALANG_BUILD_TESTS)
  add_executable(MLIRGCIRTest GCTypesTest.cpp)
  target_link_libraries(MLIRGCIRTest PRIVATE MLIRGCDialect GTest::gtest_main)
  add_test(NAME MLIRGCIRTest COMMAND MLIRGCIRTest)
endif()
