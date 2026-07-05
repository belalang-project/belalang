#ifndef BELALANG_BIR_BRTUTILS_H_
#define BELALANG_BIR_BRTUTILS_H_

#include "mlir/Support/LLVM.h"

constexpr llvm::StringRef kPrintInt = "brt_print_int";
constexpr llvm::StringRef kPrintFloat = "brt_print_float";
constexpr llvm::StringRef kPrintString = "brt_print_string";
constexpr llvm::StringRef kPrintBool = "brt_print_bool";
constexpr llvm::StringRef kGCAlloc = "brt_gc_alloc";
constexpr llvm::StringRef kInit = "brt_init";

#endif // BELALANG_BIR_BRTUTILS_H_
