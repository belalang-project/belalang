#ifndef BELALANG_BIRGEN_BIRGEN_H_
#define BELALANG_BIRGEN_BIRGEN_H_

#include "mlir/IR/Builders.h"
#include "mlir/IR/BuiltinOps.h"
#include "mlir/IR/Location.h"
#include "mlir/IR/MLIRContext.h"
#include "rust/cxx.h"
#include <cstdint>
#include <memory>

namespace belalang {
namespace birgen {
class BIRValue;
class BIRGuard;
class BIRFunctionGuard;
class BIRIfGuard;
class BIRWhileGuard;
class BIRScopeGuard;
class BIRGen;

// NOTE: manual forward decl
class BIRWhileOpGuard;
} // namespace birgen
} // namespace belalang

#include "birgen/lib.rs.h"

namespace belalang {
namespace birgen {

// -----------------------------------------------------------------------------
// BIRValue
// -----------------------------------------------------------------------------

class BIRValue {
public:
  BIRValue(mlir::Value value) : value(value) {}
  mlir::Value getValue() const { return value; }

private:
  mlir::Value value;
};

// -----------------------------------------------------------------------------
// BIRGuards
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
#include "belalang/BIRGen/Bindings.h.inc"

class BIRFunctionGuard : public BIRGuard {
public:
  BIRFunctionGuard(mlir::OpBuilder &builder, mlir::Value fnValue,
                   mlir::Region *bodyRegion)
      : BIRGuard(builder), fnValue(fnValue), bodyRegion(bodyRegion) {}
  ~BIRFunctionGuard() = default;

  std::unique_ptr<BIRValue> get_value() const {
    return std::make_unique<BIRValue>(fnValue);
  }
  std::unique_ptr<BIRValue> get_arg(size_t index) const;

private:
  mlir::Value fnValue;
  mlir::Region *bodyRegion;
};

class BIRIfGuard : public BIRGuard {
public:
  BIRIfGuard(mlir::OpBuilder &builder, mlir::Region *thenRegion,
             mlir::Region *elseRegion, mlir::Value resultValue)
      : BIRGuard(builder), thenRegion(thenRegion), elseRegion(elseRegion),
        resultValue(resultValue) {}
  ~BIRIfGuard();

  void start_then();
  void start_else();
  std::unique_ptr<BIRValue> get_value() const;

private:
  mlir::Region *thenRegion;
  mlir::Region *elseRegion;
  mlir::Value resultValue;
};

class BIRWhileGuard : public BIRGuard {
public:
  BIRWhileGuard(mlir::OpBuilder &builder, mlir::Region *condRegion,
                mlir::Region *bodyRegion)
      : BIRGuard(builder), condRegion(condRegion), bodyRegion(bodyRegion) {}
  ~BIRWhileGuard() = default;

  void start_cond();
  void start_body();

private:
  mlir::Region *condRegion;
  mlir::Region *bodyRegion;
};

class BIRScopeGuard : public BIRGuard {
public:
  BIRScopeGuard(mlir::OpBuilder &builder, mlir::Region *scopeRegion, mlir::Value resultValue)
      : BIRGuard(builder), scopeRegion(scopeRegion), resultValue(resultValue) {}
  ~BIRScopeGuard() = default;

  void start_body();
  std::unique_ptr<BIRValue> get_value() const;

private:
  mlir::Region *scopeRegion;
  mlir::Value resultValue;
};

// -----------------------------------------------------------------------------
// BIRGen
// -----------------------------------------------------------------------------

std::unique_ptr<BIRGen> create_birgen();

class BIRGen {
public:
  BIRGen();
  ~BIRGen() = default;

  std::unique_ptr<BIRValue> build_constant_int(int64_t val);
  std::unique_ptr<BIRValue> build_constant_float(double val);
  std::unique_ptr<BIRValue> build_constant_string(rust::Str val);
  std::unique_ptr<BIRValue> build_constant_bool(bool val);

  std::unique_ptr<BIRValue> build_binop(BinOpKind kind, const BIRValue &lhs,
                                        const BIRValue &rhs);
  std::unique_ptr<BIRValue> build_var_declare(const BIRValue &v,
                                              rust::Str name);
  std::unique_ptr<BIRValue> build_var_declare_ty(TypeKind v, rust::Str name);
  std::unique_ptr<BIRValue> build_var_load(const BIRValue &refValue);
  std::unique_ptr<BIRFunctionGuard>
  build_fn_expr(TypeKind resultTy, rust::Slice<const TypeKind> paramTys);
  std::unique_ptr<BIRIfGuard> build_if_expr(const BIRValue &cond);
  std::unique_ptr<BIRIfGuard> build_if_expr_ty(const BIRValue &cond, TypeKind resultTy);
  std::unique_ptr<BIRWhileGuard> build_while_stmt();
  std::unique_ptr<BIRScopeGuard> build_block_expr();
  std::unique_ptr<BIRScopeGuard> build_block_expr_ty(TypeKind resultTy);
  void build_condition(const BIRValue &cond);
  void build_continue();
  void build_break();
  void build_var_store(const BIRValue &v, const BIRValue &ref);
  void build_print(const BIRValue &val);
  void build_return(const BIRValue &val);
  void build_empty_return();
  void build_main_return();
  void build_yield(const BIRValue &val);
  void build_empty_yield();

  void start_call(const BIRValue &callee);
  void add_call_arg(const BIRValue &arg);
  std::unique_ptr<BIRValue> finish_call();

  void dump() const;
  rust::String dump_to_string() const;

  bool run_lowering_pipeline();

  uintptr_t get_module_ptr() const {
    return reinterpret_cast<uintptr_t>(&module);
  }

#define GET_BUILDER_FUNCTION_DECLS
#include "belalang/BIRGen/Bindings.h.inc"

private:
  mlir::MLIRContext context;
  mlir::ModuleOp module;
  mlir::OpBuilder builder;
  mlir::Location loc;
  mlir::Value current_callee;
  std::vector<mlir::Value> current_args;

  mlir::Type mapType(TypeKind);
};

} // namespace birgen 
} // namespace belalang

#endif // BELALANG_BIRGEN_BIRGEN_H_
