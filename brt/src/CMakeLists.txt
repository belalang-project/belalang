add_library(brt
  brt.c
  gc.c
  llvm_stackmap.c
)
target_include_directories(brt PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")

if(BUILD_TESTING AND BELALANG_BUILD_TESTS)
  file(GLOB TEST_SOURCES "*_test.cpp")

  foreach(TEST_SOURCE ${TEST_SOURCES})
    get_filename_component(TEST_NAME ${TEST_SOURCE} NAME_WE)
    add_executable(${TEST_NAME} ${TEST_SOURCE})
    target_link_libraries(${TEST_NAME} PRIVATE brt GTest::gtest_main)
    add_test(NAME ${TEST_NAME} COMMAND ${TEST_NAME})
  endforeach()

  # add_executable(brt_gc_test gc_test.cpp)
  # target_link_libraries(brt_gc_test PRIVATE brt GTest::gtest_main)
  # add_test(NAME brt_gc_test COMMAND brt_gc_test)
endif()

