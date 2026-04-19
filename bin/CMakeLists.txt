find_package(CLI11 REQUIRED)
add_executable(belalang belalang.cpp)
target_link_libraries(belalang PRIVATE CLI11::CLI11)
