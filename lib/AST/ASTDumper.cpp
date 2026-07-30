#include "belalang/AST/ASTDumper.h"
#include "belalang/Lexer/Token.h" // For TokenKind

// This is the ASTDumper implementation. For verbosity, we made every use of
// IndentGuard in its own block. This is not technically needed, but helps with
// visualizing the end result.

namespace belalang {
namespace ast {

void ASTDumper::visitProgram(Program *prog) {
  out.indent(depth) << "Program\n";
  {
    IndentGuard guard(depth);
    for (auto *stmt : prog->getStmts()) {
      visitStmt(stmt);
    }
  }
}

void ASTDumper::visitReturnStmt(ReturnStmt *stmt) {
  out.indent(depth) << "ReturnStmt\n";
  {
    IndentGuard guard(depth);
    visitExpr(stmt->getReturnValue());
  }
}

void ASTDumper::visitWhileStmt(WhileStmt *stmt) {
  out.indent(depth) << "WhileStmt\n";
  {
    IndentGuard guard(depth);
    visitExpr(stmt->getCond());
    visitExpr(stmt->getBody());
  }
}

void ASTDumper::visitBreakStmt(BreakStmt *stmt) {
  out.indent(depth) << "BreakStmt\n";
}

void ASTDumper::visitContinueStmt(ContinueStmt *stmt) {
  out.indent(depth) << "ContinueStmt\n";
}

void ASTDumper::visitImportStmt(ImportStmt *stmt) {
  out.indent(depth) << "ImportStmt\n";
  {
    IndentGuard guard(depth);
    visitExpr(stmt->getValue());
  }
}

void ASTDumper::visitExprStmt(ExprStmt *stmt) {
  out.indent(depth) << "ExprStmt\n";
  {
    IndentGuard guard(depth);
    visitExpr(stmt->getExpr());
  }
}

void ASTDumper::visitDeclStmt(DeclStmt *stmt) {
  out.indent(depth) << "DeclStmt\n";
  {
    IndentGuard guard(depth);
    visitDecl(stmt->getDecl());
  }
}

void ASTDumper::visitBoolExpr(BoolExpr *expr) {
  out.indent(depth) << "BoolExpr " << (expr->getValue() ? "true" : "false")
                    << "\n";
}

void ASTDumper::visitIntLitExpr(IntLitExpr *expr) {
  out.indent(depth) << "IntLitExpr " << expr->getValue() << "\n";
}

void ASTDumper::visitFloatLitExpr(FloatLitExpr *expr) {
  out.indent(depth) << "FloatLitExpr " << expr->getValue() << "\n";
}

void ASTDumper::visitStringLitExpr(StringLitExpr *expr) {
  out.indent(depth) << "StringLitExpr \"" << expr->getValue() << "\"\n";
}

void ASTDumper::visitArrayLitExpr(ArrayLitExpr *expr) {
  out.indent(depth) << "ArrayLitExpr\n";
  {
    IndentGuard guard(depth);
    for (auto *e : expr->getElements()) {
      visitExpr(e);
    }
  }
}

void ASTDumper::visitVarExpr(VarExpr *expr) {
  out.indent(depth) << "VarExpr " << expr->getName() << " "
                    << lexer::Token(expr->getAssignOp()).getString() << "\n";
  {
    IndentGuard guard(depth);
    visitExpr(expr->getValue());
  }
}

void ASTDumper::visitFunctionLitExpr(FunctionLitExpr *expr) {
  out.indent(depth) << "FunctionLitExpr";
  if (!expr->getExplicitType().empty()) {
    out << " -> " << expr->getExplicitType();
  }
  out << "\n";
  {
    IndentGuard guard(depth);
    for (auto *p : expr->getParams()) {
      visitDecl(p);
    }
    visitExpr(expr->getBody());
  }
}

void ASTDumper::visitCallExpr(CallExpr *expr) {
  out.indent(depth) << "CallExpr\n";
  {
    IndentGuard guard(depth);
    visitExpr(expr->getCallee());
    for (auto *a : expr->getArgs()) {
      visitExpr(a);
    }
  }
}

void ASTDumper::visitIndexExpr(IndexExpr *expr) {
  out.indent(depth) << "IndexExpr\n";
  {
    IndentGuard guard(depth);
    visitExpr(expr->getLeft());
    visitExpr(expr->getIndex());
  }
}

void ASTDumper::visitIdentifierExpr(IdentifierExpr *expr) {
  out.indent(depth) << "IdentifierExpr " << expr->getName() << "\n";
}

void ASTDumper::visitIfExpr(IfExpr *expr) {
  out.indent(depth) << "IfExpr\n";
  {
    IndentGuard guard(depth);
    visitExpr(expr->getCond());
    visitExpr(expr->getThen());
    if (expr->getAlt()) {
      visitExpr(expr->getAlt());
    }
  }
}

void ASTDumper::visitInfixExpr(InfixExpr *expr) {
  out.indent(depth) << "InfixExpr "
                    << lexer::Token(expr->getInfixOp()).getString() << "\n";
  {
    IndentGuard guard(depth);
    visitExpr(expr->getLeft());
    visitExpr(expr->getRight());
  }
}

void ASTDumper::visitPrefixExpr(PrefixExpr *expr) {
  out.indent(depth) << "PrefixExpr "
                    << lexer::Token(expr->getPrefixOp()).getString() << "\n";
  {
    IndentGuard guard(depth);
    visitExpr(expr->getRight());
  }
}

void ASTDumper::visitBlockExpr(BlockExpr *expr) {
  out.indent(depth) << "BlockExpr\n";
  {
    IndentGuard guard(depth);
    for (auto *stmt : expr->getStmts()) {
      visitStmt(stmt);
    }
  }
}

void ASTDumper::visitMemberAccessExpr(MemberAccessExpr *expr) {
  out.indent(depth) << "MemberAccessExpr ." << expr->getMember() << "\n";
  {
    IndentGuard guard(depth);
    visitExpr(expr->getSource());
  }
}

void ASTDumper::visitStructLiteralExpr(StructLiteralExpr *expr) {
  out.indent(depth) << "StructLiteralExpr " << expr->getName() << "\n";
  {
    IndentGuard guard(depth);
    for (auto *field : expr->getFields()) {
      visitExpr(field);
    }
  }
}

void ASTDumper::visitVarDecl(VarDecl *decl) {
  out.indent(depth) << "VarDecl " << decl->getName();
  if (!decl->getExplicitType().empty()) {
    out << " " << decl->getExplicitType();
  }
  out << "\n";
  if (decl->getValue()) {
    IndentGuard guard(depth);
    visitExpr(decl->getValue());
  }
}

void ASTDumper::visitStructDecl(StructDecl *decl) {
  out.indent(depth) << "StructDecl " << decl->getName() << "\n";
  {
    IndentGuard guard(depth);
    for (auto *field : decl->getFields()) {
      visitDecl(field);
    }
  }
}

} // namespace ast
} // namespace belalang
