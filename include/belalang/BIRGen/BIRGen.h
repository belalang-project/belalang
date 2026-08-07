#ifndef BELALANG_BIRGEN_BIRGEN_H_
#define BELALANG_BIRGEN_BIRGEN_H_

#include "belalang/AST/ASTContext.h"
#include "belalang/AST/ASTVisitor.h"
#include "belalang/BIRGen/TypeChecker.h"
#include "belalang/Diag/Diag.h"
#include "mlir/IR/Builders.h"
#include "mlir/IR/BuiltinOps.h"
#include "mlir/IR/Location.h"
#include "mlir/IR/MLIRContext.h"
#include <string>

namespace belalang {
namespace birgen {

class BIRGen : public ast::ASTVisitor<BIRGen, mlir::Value> {
public:
  BIRGen(ast::ASTContext &ctx, diag::DiagnosticEngine &diagEngine);
  ~BIRGen() = default;

  uintptr_t getModulePtr() const {
    return reinterpret_cast<uintptr_t>(&module);
  }

  mlir::ModuleOp generateProgram(ast::Program *prog);
  bool runLoweringPipeline();
  std::string dumpToString() const;

#define STMT(name) mlir::Value visit##name##Stmt(ast::name##Stmt *);
#define EXPR(name) mlir::Value visit##name##Expr(ast::name##Expr *);
#define DECL(name) mlir::Value visit##name##Decl(ast::name##Decl *);
#include "belalang/AST/ASTNodes.def"

private:
  diag::DiagnosticEngine &diagEngine;
  TypeChecker typeChecker;
  llvm::StringMap<mlir::Value> symbolTable;

  void buildMainReturn();
  mlir::Type mapTypeName(llvm::StringRef name);

  mlir::MLIRContext context;
  mlir::ModuleOp module;
  mlir::OpBuilder builder;
  mlir::Location loc;
};

} // namespace birgen
} // namespace belalang

#endif // BELALANG_BIRGEN_BIRGEN_H_
