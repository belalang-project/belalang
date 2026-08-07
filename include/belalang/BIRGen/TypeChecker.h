#ifndef BELALANG_BIRGEN_TYPECHECKER_H_
#define BELALANG_BIRGEN_TYPECHECKER_H_

#include "belalang/AST/ASTVisitor.h"
#include "belalang/AST/ASTContext.h"
#include "belalang/Diag/Diag.h"
#include "llvm/ADT/StringMap.h"
#include "llvm/ADT/StringRef.h"
#include <string>

namespace belalang {
namespace birgen {

enum class Type {
  String,
  Integer,
  Float,
  Boolean,
  None,
};

class TypeChecker : public ast::ASTVisitor<TypeChecker, Type> {
public:
  TypeChecker(ast::ASTContext ctx, diag::DiagnosticEngine &diagEngine);

  void infer(ast::Program *prog);
  void checkExpr(ast::Expr *expr, Type expected);

  Type visitIntLitExpr(ast::IntLitExpr *expr);
  Type visitFloatLitExpr(ast::FloatLitExpr *expr);
  Type visitStringLitExpr(ast::StringLitExpr *expr);
  Type visitBoolExpr(ast::BoolExpr *expr);
  Type visitIdentifierExpr(ast::IdentifierExpr *expr);
  Type visitInfixExpr(ast::InfixExpr *expr);
  Type visitBlockExpr(ast::BlockExpr *expr);
  Type visitIfExpr(ast::IfExpr *expr);
  Type visitVarDecl(ast::VarDecl *decl);
  Type visitDeclStmt(ast::DeclStmt *stmt);
  Type visitExprStmt(ast::ExprStmt *stmt);

private:
  ast::ASTContext ctx;
  diag::DiagnosticEngine &diagEngine;
  llvm::StringMap<Type> env;

  std::string tyToStr(Type ty) const;
  Type strToTy(llvm::StringRef str) const;
};

} // namespace birgen
} // namespace belalang

#endif // BELALANG_BIRGEN_TYPECHECKER_H_
