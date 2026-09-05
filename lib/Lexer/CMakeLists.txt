add_library(BelalangLexer Lexer.cpp)
target_link_libraries(BelalangLexer
  PUBLIC
    BelalangHeaders
    BelalangDiag
    LLVMSupport
)
