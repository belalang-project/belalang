add_library(BelalangGCIR)
add_dependencies(BelalangGCIR BelalangGCIRIncGen BelalangGCIRConversionPassIncGen)
target_link_libraries(BelalangGCIR
  PUBLIC
    MLIRIR
    MLIRFuncToLLVM
    MLIRLLVMDialect
    MLIRPass
)

add_subdirectory(Conversions)

add_subdirectory(IR)
