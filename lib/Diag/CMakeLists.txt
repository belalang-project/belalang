add_library(BelalangDiag Diag.cpp)
target_link_libraries(BelalangDiag PUBLIC BelalangHeaders LLVMSupport)

if(BUILD_TESTING AND BELALANG_BUILD_TESTS)
  add_executable(BelalangDiagTest DiagTest.cpp)
  target_link_libraries(BelalangDiagTest PRIVATE BelalangDiag GTest::gtest_main)
  add_test(NAME BelalangDiagTest COMMAND BelalangDiagTest)
endif()
