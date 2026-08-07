#include "belalang/BIRGen/TypeChecker.h"

#include "belalang/AST/Decl.h"
#include "belalang/AST/Expr.h"
#include "belalang/AST/Stmt.h"
#include "belalang/Diag/Diag.h"
#include "llvm/Support/FormatVariadic.h"
#include <optional>

namespace belalang {
namespace birgen {

TypeChecker::TypeChecker(ast::ASTContext ctx, diag::DiagnosticEngine &diagEngine)
    : ctx(ctx), diagEngine(diagEngine) {}

void TypeChecker::infer(ast::Program *prog) { visitProgram(prog); }

void TypeChecker::checkExpr(ast::Expr *expr, Type expected) {
  Type inferred = visitExpr(expr);
  if (inferred != expected) {
    auto label = diag::Label::primary(
        expr->getSpanStart(), expr->getSpanEnd(),
        llvm::formatv("expected type `{0}`, found type `{1}`",
                      tyToStr(expected), tyToStr(inferred)));
    diagEngine.print(
        diag::Diagnostic::error("mismatched type").withLabel(label));
  }
}

Type TypeChecker::visitIntLitExpr(ast::IntLitExpr *expr) {
  return Type::Integer;
}

Type TypeChecker::visitFloatLitExpr(ast::FloatLitExpr *expr) {
  return Type::Float;
}

Type TypeChecker::visitStringLitExpr(ast::StringLitExpr *expr) {
  return Type::String;
}

Type TypeChecker::visitBoolExpr(ast::BoolExpr *expr) { return Type::Boolean; }

Type TypeChecker::visitIdentifierExpr(ast::IdentifierExpr *expr) {
  auto it = env.find(expr->getName());
  if (it != env.end()) {
    return it->second;
  }
  return Type::None;
}

Type TypeChecker::visitInfixExpr(ast::InfixExpr *expr) {
  Type leftTy = visitExpr(expr->getLeft());
  Type rightTy = visitExpr(expr->getRight());

  if (leftTy != rightTy) {
    auto label = diag::Label::primary(
        expr->getRight()->getSpanStart(), expr->getRight()->getSpanEnd(),
        llvm::formatv("expected type `{0}`, found type `{1}`", tyToStr(leftTy),
                      tyToStr(rightTy)));
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
    return Type::Boolean;

  case lexer::TokenKind::And:
  case lexer::TokenKind::Or: {
    if (leftTy != Type::Boolean) {
      auto label = diag::Label::primary(
          expr->getLeft()->getSpanStart(), expr->getLeft()->getSpanEnd(),
          llvm::formatv("expected type `Boolean`, found type `{0}`",
                        tyToStr(leftTy)));
      diagEngine.print(
          diag::Diagnostic::error("mismatched type").withLabel(label));
    }
    return Type::Boolean;
  }

  default:
    return leftTy;
  }
}

Type TypeChecker::visitBlockExpr(ast::BlockExpr *expr) {
  Type lastTy = Type::None;
  for (ast::Stmt *stmt : expr->getStmts()) {
    lastTy = visitStmt(stmt);
  }
  return lastTy;
}

Type TypeChecker::visitIfExpr(ast::IfExpr *expr) {
  checkExpr(expr->getCond(), Type::Boolean);
  Type consequenceTy = visitBlockExpr(expr->getThen());

  if (ast::Expr *altExpr = expr->getAlt()) {
    Type alternativeTy = visitExpr(altExpr);
    if (consequenceTy != alternativeTy) {
      auto label = diag::Label::primary(
          altExpr->getSpanStart(), altExpr->getSpanEnd(),
          llvm::formatv("expected type `{0}`, found type `{1}`",
                        tyToStr(consequenceTy), tyToStr(alternativeTy)));
      diagEngine.print(
          diag::Diagnostic::error("mismatched type").withLabel(label));
    }
    return consequenceTy;
  }
  return Type::None;
}

Type TypeChecker::visitVarDecl(ast::VarDecl *decl) {
  std::optional<Type> explicitTy;
  if (!decl->getExplicitType().empty()) {
    explicitTy = strToTy(decl->getExplicitType());
  }

  Type rhsTy = Type::None;
  if (explicitTy.has_value()) {
    if (ast::Expr *value = decl->getValue()) {
      checkExpr(value, explicitTy.value());
    }
    rhsTy = explicitTy.value();
  } else {
    if (ast::Expr *value = decl->getValue()) {
      rhsTy = visitExpr(value);
    } else {
      rhsTy = Type::None;
    }
  }

  env[decl->getName()] = rhsTy;
  return rhsTy;
}

Type TypeChecker::visitDeclStmt(ast::DeclStmt *stmt) {
  return visitDecl(stmt->getDecl());
}

Type TypeChecker::visitExprStmt(ast::ExprStmt *stmt) {
  return visitExpr(stmt->getExpr());
}

std::string TypeChecker::tyToStr(Type ty) const {
  switch (ty) {
  case Type::String:
    return "String";
  case Type::Integer:
    return "Integer";
  case Type::Float:
    return "Float";
  case Type::Boolean:
    return "Boolean";
  case Type::None:
    return "None";
  }
  return "None";
}

Type TypeChecker::strToTy(llvm::StringRef str) const {
  if (str == "String")
    return Type::String;
  if (str == "Int")
    return Type::Integer;
  if (str == "Float")
    return Type::Float;
  if (str == "Bool")
    return Type::Boolean;
  return Type::None;
}

} // namespace birgen
} // namespace belalang
