add_library(BelalangGCIR)
add_dependencies(BelalangGCIR BelalangGCIRTableGen)
target_link_libraries(BelalangGCIR
  PUBLIC
    BelalangHeaders
    MLIRIR
)

add_subdirectory(IR)
