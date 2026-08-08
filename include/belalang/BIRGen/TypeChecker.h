#ifndef BELALANG_BIRGEN_TYPECHECKER_H_
#define BELALANG_BIRGEN_TYPECHECKER_H_

#include "belalang/AST/ASTContext.h"
#include "belalang/AST/ASTVisitor.h"
#include "belalang/Diag/Diag.h"
#include "llvm/ADT/StringMap.h"

namespace belalang {
namespace birgen {

class TypeChecker : public ast::ASTVisitor<TypeChecker, ast::Type *> {
public:
  TypeChecker(ast::ASTContext &ctx, diag::DiagnosticEngine &diagEngine);

  void infer(ast::Program *prog);
  void checkExpr(ast::Expr *expr, ast::Type *expected);

  ast::Type *visitIntLitExpr(ast::IntLitExpr *expr);
  ast::Type *visitFloatLitExpr(ast::FloatLitExpr *expr);
  ast::Type *visitStringLitExpr(ast::StringLitExpr *expr);
  ast::Type *visitBoolExpr(ast::BoolExpr *expr);
  ast::Type *visitIdentifierExpr(ast::IdentifierExpr *expr);
  ast::Type *visitInfixExpr(ast::InfixExpr *expr);
  ast::Type *visitBlockExpr(ast::BlockExpr *expr);
  ast::Type *visitIfExpr(ast::IfExpr *expr);
  ast::Type *visitVarDecl(ast::VarDecl *decl);
  ast::Type *visitDeclStmt(ast::DeclStmt *stmt);
  ast::Type *visitExprStmt(ast::ExprStmt *stmt);

private:
  ast::ASTContext &ctx;
  diag::DiagnosticEngine &diagEngine;
  llvm::StringMap<ast::Type *> env;
};

} // namespace birgen
} // namespace belalang

#endif // BELALANG_BIRGEN_TYPECHECKER_H_
