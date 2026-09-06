add_library(BelalangGCIR)
add_dependencies(BelalangGCIR BelalangGCIRIncGen BelalangGCIRPassIncGen)
target_link_libraries(BelalangGCIR
  PUBLIC
    MLIRIR
    MLIRFuncToLLVM
    MLIRLLVMDialect
    MLIRPass
)

target_sources(BelalangGCIR PRIVATE GCIRToLLVM.cpp)

add_subdirectory(IR)
