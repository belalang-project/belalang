#ifndef BELALANG_BIRGEN_BIRGEN_H_
#define BELALANG_BIRGEN_BIRGEN_H_

#include "mlir/IR/Builders.h"
#include "mlir/IR/BuiltinOps.h"
#include "mlir/IR/MLIRContext.h"
#include "rust/cxx.h"
#include <memory>

namespace belalang {
namespace birgen {

class BIRValue {
public:
  BIRValue(mlir::Value value) : value(value) {}
  mlir::Value getValue() const { return value; }

private:
  mlir::Value value;
};

class BIRGen {
public:
  BIRGen();
  ~BIRGen() = default;

  std::unique_ptr<BIRValue> build_constant_int(int64_t val);
  std::unique_ptr<BIRValue> build_constant_float(double val);
  std::unique_ptr<BIRValue> build_add(const BIRValue &lhs, const BIRValue &rhs);
  std::unique_ptr<BIRValue> build_sub(const BIRValue &lhs, const BIRValue &rhs);
  std::unique_ptr<BIRValue> build_mul(const BIRValue &lhs, const BIRValue &rhs);
  std::unique_ptr<BIRValue> build_div(const BIRValue &lhs, const BIRValue &rhs);
  std::unique_ptr<BIRValue> build_mod(const BIRValue &lhs, const BIRValue &rhs);
  void build_print(const BIRValue &val);
  void build_return(const BIRValue &val);
  void build_empty_return();

  void dump() const;
  rust::String dump_to_string() const;

  bool optimize();

private:
  mlir::MLIRContext context;
  mlir::ModuleOp module;
  mlir::OpBuilder builder;
  mlir::Location loc;
};

std::unique_ptr<BIRGen> create_birgen();

} // namespace birgen
} // namespace belalang

#endif // BELALANG_BIRGEN_BIRGEN_H_
