add_executable(bir-tblgen bir-tblgen.cpp)
target_link_libraries(bir-tblgen
  PRIVATE
    BelalangBIR
    LLVMTableGen
    MLIRTableGen
)
