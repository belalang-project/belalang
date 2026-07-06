#include "mlir/IR/MLIRContext.h"
#include "mlir/IR/Builders.h"
#include "mlir/IR/BuiltinOps.h"

#include "belalang/BIR/IR/BIR.h"
#include "belalang/BIRGen/BIRGen.h"

#include "belalang/BIR/CodeGen/BIRGen.h"

namespace belalang {
namespace bir {
namespace codegen {

std::unique_ptr<BIRGen> create_birgen(uintptr_t gen_ptr) {
  auto *gen = reinterpret_cast<birgen::BIRGen *>(gen_ptr);
  return std::make_unique<BIRGen>(*gen);
}

BIRGen::BIRGen(::belalang::birgen::BIRGen &gen) : gen(gen) {}

#include "belalang/BIR/CodeGen/Bindings.cpp.inc"

} // codegen
} // bir
} // belalang
