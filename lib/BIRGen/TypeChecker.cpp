#include "belalang/BIRGen/TypeChecker.h"

#include "belalang/AST/Decl.h"
#include "belalang/AST/Expr.h"
#include "belalang/AST/Stmt.h"
#include "belalang/Diag/Diag.h"
#include "llvm/Support/FormatVariadic.h"

namespace belalang {
namespace birgen {

TypeChecker::TypeChecker(ast::ASTContext &ctx,
                         diag::DiagnosticEngine &diagEngine)
    : ctx(ctx), diagEngine(diagEngine) {}

void TypeChecker::infer(ast::Program *prog) { visitProgram(prog); }

void TypeChecker::checkExpr(ast::Expr *expr, ast::Type *expected) {
  ast::Type *inferred = visitExpr(expr);
  if (inferred != expected) {
    auto label = diag::Label::primary(
        expr->getSpanStart(), expr->getSpanEnd(),
        llvm::formatv("expected type `{0}`, found type `{1}`",
                      ctx.tyToStr(expected), ctx.tyToStr(inferred)));
    diagEngine.print(
        diag::Diagnostic::error("mismatched type").withLabel(label));
  }
}

ast::Type *TypeChecker::visitIntLitExpr(ast::IntLitExpr *expr) {
  return ctx.intTy;
}

ast::Type *TypeChecker::visitFloatLitExpr(ast::FloatLitExpr *expr) {
  return ctx.floatTy;
}

ast::Type *TypeChecker::visitStringLitExpr(ast::StringLitExpr *expr) {
  return ctx.stringTy;
}

ast::Type *TypeChecker::visitBoolExpr(ast::BoolExpr *expr) {
  return ctx.boolTy;
}

ast::Type *TypeChecker::visitIdentifierExpr(ast::IdentifierExpr *expr) {
  auto it = env.find(expr->getName());
  if (it != env.end()) {
    return it->second;
  }
  return ctx.noneTy;
}

ast::Type *TypeChecker::visitInfixExpr(ast::InfixExpr *expr) {
  ast::Type *leftTy = visitExpr(expr->getLeft());
  ast::Type *rightTy = visitExpr(expr->getRight());

  if (leftTy != rightTy) {
    auto label = diag::Label::primary(
        expr->getRight()->getSpanStart(), expr->getRight()->getSpanEnd(),
        llvm::formatv("expected type `{0}`, found type `{1}`",
                      ctx.tyToStr(leftTy), ctx.tyToStr(rightTy)));
    diagEngine.print(
        diag::Diagnostic::error("mismatched type").withLabel(label));
  }

  switch (expr->getInfixOp()) {
  case lexer::TokenKind::Eq:
  case lexer::TokenKind::Ne:
  case lexer::TokenKind::Lt:
  case lexer::TokenKind::Le:
  case lexer::TokenKind::Gt:
  case lexer::TokenKind::Ge:
    return ctx.boolTy;

  case lexer::TokenKind::And:
  case lexer::TokenKind::Or: {
    if (leftTy != ctx.boolTy) {
      auto label = diag::Label::primary(
          expr->getLeft()->getSpanStart(), expr->getLeft()->getSpanEnd(),
          llvm::formatv("expected type `Boolean`, found type `{0}`",
                        ctx.tyToStr(leftTy)));
      diagEngine.print(
          diag::Diagnostic::error("mismatched type").withLabel(label));
    }
    return ctx.boolTy;
  }

  default:
    return leftTy;
  }
}

ast::Type *TypeChecker::visitBlockExpr(ast::BlockExpr *expr) {
  ast::Type *lastTy = ctx.noneTy;
  for (ast::Stmt *stmt : expr->getStmts()) {
    lastTy = visitStmt(stmt);
  }
  return lastTy;
}

ast::Type *TypeChecker::visitIfExpr(ast::IfExpr *expr) {
  checkExpr(expr->getCond(), ctx.boolTy);
  ast::Type *consequenceTy = visitBlockExpr(expr->getThen());

  if (ast::Expr *altExpr = expr->getAlt()) {
    ast::Type *alternativeTy = visitExpr(altExpr);
    if (consequenceTy != alternativeTy) {
      auto label = diag::Label::primary(
          altExpr->getSpanStart(), altExpr->getSpanEnd(),
          llvm::formatv("expected type `{0}`, found type `{1}`",
                        ctx.tyToStr(consequenceTy),
                        ctx.tyToStr(alternativeTy)));
      diagEngine.print(
          diag::Diagnostic::error("mismatched type").withLabel(label));
    }
    return consequenceTy;
  }
  return ctx.noneTy;
}

ast::Type *TypeChecker::visitVarDecl(ast::VarDecl *decl) {
  ast::Type *explicitTy = decl->getExplicitType();

  ast::Type *rhsTy = ctx.noneTy;
  if (explicitTy) {
    if (ast::Expr *value = decl->getValue()) {
      checkExpr(value, explicitTy);
    }
    rhsTy = explicitTy;
  } else {
    if (ast::Expr *value = decl->getValue()) {
      rhsTy = visitExpr(value);
    } else {
      rhsTy = ctx.noneTy;
    }
  }

  env[decl->getName()] = rhsTy;
  return rhsTy;
}

ast::Type *TypeChecker::visitDeclStmt(ast::DeclStmt *stmt) {
  return visitDecl(stmt->getDecl());
}

ast::Type *TypeChecker::visitExprStmt(ast::ExprStmt *stmt) {
  return visitExpr(stmt->getExpr());
}

} // namespace birgen
} // namespace belalang
