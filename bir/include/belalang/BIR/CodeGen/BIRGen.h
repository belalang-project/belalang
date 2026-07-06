#ifndef BELALANG_BIR_CODEGEN_BIRGEN_H_
#define BELALANG_BIR_CODEGEN_BIRGEN_H_

#include <cstdint>
#include <memory>
#include "mlir/IR/MLIRContext.h"
#include "mlir/IR/BuiltinOps.h"
#include "mlir/IR/Builders.h"
#include "mlir/IR/Location.h"

namespace belalang {

// forward declaration
namespace birgen {
class BIRGen;
} // namespace birgen

namespace bir {
namespace codegen {

// -----------------------------------------------------------------------------
// BIRGuard
// -----------------------------------------------------------------------------

class BIRGuard {
public:
  BIRGuard(mlir::OpBuilder &builder) : guard(builder), builder(builder) {}
  virtual ~BIRGuard() = default;

protected:
  mlir::OpBuilder::InsertionGuard guard;
  mlir::OpBuilder &builder;
};

#define GET_BIRGUARD_CLASS_DECLS
#include "belalang/BIR/CodeGen/Bindings.h.inc"

// -----------------------------------------------------------------------------
// BIRGen2
// -----------------------------------------------------------------------------

class BIRGen2 {
public:
  BIRGen2(birgen::BIRGen &gen);

#define GET_BUILDER_FUNCTION_DECLS
#include "belalang/BIR/CodeGen/Bindings.h.inc"

private:
  birgen::BIRGen &gen;
};

std::unique_ptr<BIRGen2> create_birgen2(uintptr_t gen);

} // namespace codegen
} // namespace bir
} // namespace belalang

#endif // BELALANG_BIR_CODEGEN_BIRGEN_H_
