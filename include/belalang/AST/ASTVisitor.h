#ifndef BELALANG_AST_ASTVISITOR_H_
#define BELALANG_AST_ASTVISITOR_H_

#include "belalang/AST/Decl.h"
#include "belalang/AST/Expr.h"
#include "belalang/AST/Program.h"
#include "belalang/AST/Stmt.h"

namespace belalang {
namespace ast {

template <typename Derived, typename RetT = void> class ASTVisitor {
public:
  void visitProgram(Program *prog) {
    for (Stmt *stmt : prog->getStmts())
      visitStmt(stmt);
  }

  RetT visitStmt(Stmt *stmt) {
    if (!stmt) {
      if constexpr (std::is_same_v<RetT, void>) {
        return;
      } else {
        return RetT{};
      }
    }

    switch (stmt->getKind()) {
      // clang-format off
#define STMT(name)                                                             \
  case Stmt::StmtKind:: name##Stmt:                                            \
    return static_cast<Derived *>(this)->visit##name##Stmt(                    \
        static_cast<name##Stmt *>(stmt));
#include "ASTNodes.def"
      // clang-format on
    }
  }

  RetT visitExpr(Expr *expr) {
    if (!expr) {
      if constexpr (std::is_same_v<RetT, void>) {
        return;
      } else {
        return RetT{};
      }
    }

    switch (expr->getKind()) {
      // clang-format off
#define EXPR(name)                                                             \
  case Expr::ExprKind:: name##Expr:                                            \
    return static_cast<Derived *>(this)->visit##name##Expr(                    \
        static_cast<name##Expr *>(expr));
#include "ASTNodes.def"
      // clang-format on
    }
  }

  RetT visitDecl(Decl *decl) {
    if (!decl) {
      if constexpr (std::is_same_v<RetT, void>) {
        return;
      } else {
        return RetT{};
      }
    }

    switch (decl->getKind()) {
      // clang-format off
#define DECL(name)                                                             \
  case Decl::DeclKind:: name##Decl:                                            \
    return static_cast<Derived *>(this)->visit##name##Decl(                    \
        static_cast<name##Decl *>(decl));
#include "ASTNodes.def"
      // clang-format on
    }
  }

#define STMT(name)                                                             \
  RetT visit##name##Stmt(name##Stmt *) { return; }
#define EXPR(name)                                                             \
  RetT visit##name##Expr(name##Expr *) { return; }
#define DECL(name)                                                             \
  RetT visit##name##Decl(name##Decl *) { return; }
#include "ASTNodes.def"
};

} // namespace ast
} // namespace belalang

#endif // BELALANG_AST_ASTVISITOR_H_
