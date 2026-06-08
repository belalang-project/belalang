#ifndef BELALANG_BIR_BUILDER_H_
#define BELALANG_BIR_BUILDER_H_

#include "mlir/IR/Builders.h"
#include "mlir/IR/BuiltinOps.h"
#include "mlir/IR/MLIRContext.h"
#include "rust/cxx.h"
#include <memory>

namespace belalang {
namespace bir {

class BIRValue {
public:
  BIRValue(mlir::Value value) : value(value) {}
  mlir::Value getValue() const { return value; }

private:
  mlir::Value value;
};

class BIRBuilder {
public:
  BIRBuilder();
  ~BIRBuilder() = default;

  std::unique_ptr<BIRValue> build_constant_int(int64_t val);
  std::unique_ptr<BIRValue> build_constant_float(double val);
  std::unique_ptr<BIRValue> build_add(const BIRValue &lhs, const BIRValue &rhs);
  std::unique_ptr<BIRValue> build_sub(const BIRValue &lhs, const BIRValue &rhs);
  std::unique_ptr<BIRValue> build_mul(const BIRValue &lhs, const BIRValue &rhs);
  std::unique_ptr<BIRValue> build_div(const BIRValue &lhs, const BIRValue &rhs);
  std::unique_ptr<BIRValue> build_mod(const BIRValue &lhs, const BIRValue &rhs);
  void build_print(const BIRValue &val);

  void dump() const;
  rust::String dump_to_string() const;

private:
  mlir::MLIRContext context;
  mlir::ModuleOp module;
  mlir::OpBuilder builder;
  mlir::Location loc;
};

std::unique_ptr<BIRBuilder> create_builder();

} // namespace bir
} // namespace belalang

#endif // BELALANG_BIR_BUILDER_H_
