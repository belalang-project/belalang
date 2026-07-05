#include "mlir/IR/MLIRContext.h"
#include "mlir/IR/Builders.h"
#include "mlir/IR/BuiltinOps.h"

#include "belalang/BIR/IR/BIR.h"
#include "belalang/BIRGen/BIRGen.h"

#include "BIRGen2.h"

namespace belalang {
namespace birgen2 {

std::unique_ptr<BIRGen2> create_birgen2(uintptr_t gen_ptr) {
  auto *gen = reinterpret_cast<birgen::BIRGen *>(gen_ptr);
  return std::make_unique<BIRGen2>(*gen);
}

BIRGen2::BIRGen2(::belalang::birgen::BIRGen &gen) : gen(gen) {}

#include "Bindings.cpp.inc"

} // birgen2
} // belalang
