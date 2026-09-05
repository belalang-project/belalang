add_library(BelalangGCIR)
add_dependencies(BelalangGCIR BelalangGCIRIncGen)
target_link_libraries(BelalangGCIR
  PUBLIC
    MLIRIR
)

add_subdirectory(IR)
