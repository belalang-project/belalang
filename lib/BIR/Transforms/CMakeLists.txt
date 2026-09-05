target_sources(BelalangBIR PRIVATE
  BIRToLLVM.cpp
  FlattenCFG.cpp
  LowerDeclToMemory.cpp
  LowerFuncExpr.cpp
  OptimizeStructLayout.cpp
  PrepareGCAllocations.cpp
  VerifyLoweredForm.cpp
  ../Pipelines.cpp
)

if(BUILD_TESTING AND BELALANG_BUILD_TESTS)
  add_executable(BelalangOptimizeStructLayoutTest OptimizeStructLayoutTest.cpp)
  target_link_libraries(BelalangOptimizeStructLayoutTest PRIVATE BelalangBIR GTest::gtest_main)
  add_test(NAME BelalangOptimizeStructLayoutTest COMMAND BelalangOptimizeStructLayoutTest)
endif()

