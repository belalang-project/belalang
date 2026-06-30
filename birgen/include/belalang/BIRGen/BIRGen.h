#ifndef BELALANG_BIRGEN_BIRGEN_H_
#define BELALANG_BIRGEN_BIRGEN_H_

#include "llvm/IR/Module.h"

#include "mlir/IR/Builders.h"
#include "mlir/IR/BuiltinOps.h"
#include "mlir/IR/MLIRContext.h"
#include "mlir/Target/LLVMIR/Dialect/LLVMIR/LLVMToLLVMIRTranslation.h"
#include "mlir/Target/LLVMIR/Export.h"
#include "rust/cxx.h"
#include <memory>
#include <stdint.h>

// Forward declarations for lib.rs.h
namespace belalang {
namespace birgen {
class LLVMGen;
class BIRValue;
class BIRGuard;
class BIRGen;
}
}

#include "birgen/src/lib.rs.h"

namespace belalang {
namespace birgen {

class LLVMGen {
public:
  LLVMGen(mlir::ModuleOp *module);
  ~LLVMGen() = default;

  rust::String dump_to_string() const;
  rust::String compile_object_file(rust::String out) const;

private:
  llvm::LLVMContext context;
  std::unique_ptr<llvm::Module> module;
};

class BIRValue {
public:
  BIRValue(mlir::Value value) : value(value) {}
  mlir::Value getValue() const { return value; }

private:
  mlir::Value value;
};

class BIRGuard {
public:
  BIRGuard(mlir::OpBuilder &builder, mlir::Value fnValue)
      : builder(builder), guard(builder), fnValue(fnValue) {}
  ~BIRGuard() = default;

  std::unique_ptr<BIRValue> get_value() const {
    return std::make_unique<BIRValue>(fnValue);
  }
  std::unique_ptr<BIRValue> get_arg(size_t index) const;

private:
  mlir::OpBuilder &builder;
  mlir::OpBuilder::InsertionGuard guard;
  mlir::Value fnValue;
};

class BIRGen {
public:
  BIRGen();
  ~BIRGen() = default;

  std::unique_ptr<BIRValue> build_constant_int(int64_t val);
  std::unique_ptr<BIRValue> build_constant_float(double val);
  std::unique_ptr<BIRValue> build_constant_string(rust::Str val);
  std::unique_ptr<BIRValue> build_constant_bool(bool val);

  std::unique_ptr<BIRValue> build_binop(BinOpKind kind, const BIRValue &lhs, const BIRValue &rhs);
  std::unique_ptr<BIRValue> build_var_declare(const BIRValue &v, rust::Str name);
  std::unique_ptr<BIRValue> build_var_declare_ty(TypeKind v, rust::Str name);
  std::unique_ptr<BIRValue> build_var_load(const BIRValue &refValue);
  std::unique_ptr<BIRGuard> build_fn_expr(TypeKind resultTy, rust::Slice<const TypeKind> paramTys);
  void build_var_store(const BIRValue &v, const BIRValue &ref);
  void build_print(const BIRValue &val);
  void build_return(const BIRValue &val);
  void build_empty_return();
  void build_main_return();

  void start_call(const BIRValue &callee);
  void add_call_arg(const BIRValue &arg);
  std::unique_ptr<BIRValue> finish_call();

  void dump() const;
  rust::String dump_to_string() const;

  bool optimize();

  std::unique_ptr<LLVMGen> llvmgen();

private:
  mlir::MLIRContext context;
  mlir::ModuleOp module;
  mlir::OpBuilder builder;
  mlir::Location loc;
  mlir::Value current_callee;
  std::vector<mlir::Value> current_args;

  mlir::Type mapType(TypeKind);
};

std::unique_ptr<BIRGen> create_birgen();

} // namespace birgen
} // namespace belalang

#endif // BELALANG_BIRGEN_BIRGEN_H_
