target_sources(BelalangBIR PRIVATE BIRAttrs.cpp BIRDialect.cpp BIROps.cpp BIRTypes.cpp)

if(BUILD_TESTING AND BELALANG_BUILD_TESTS)
  add_executable(BelalangBIRTypesTest BIRTypesTest.cpp)
  target_link_libraries(BelalangBIRTypesTest PRIVATE BelalangBIR GTest::gtest_main)
  add_test(NAME BelalangBIRTypesTest COMMAND BelalangBIRTypesTest)
endif()

