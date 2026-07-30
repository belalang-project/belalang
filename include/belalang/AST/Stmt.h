#ifndef BELALANG_AST_STMT_H_
#define BELALANG_AST_STMT_H_

#include "belalang/AST/ASTContext.h"

namespace belalang {
namespace ast {

class Expr;
class BlockExpr;
class StringLitExpr;

// -----------------------------------------------------------------------------
// Base Statement
// -----------------------------------------------------------------------------

class Stmt {
public:
  Stmt() = delete;
  Stmt(const Stmt &) = delete;
  Stmt(Stmt &&) = delete;
  Stmt &operator=(const Stmt &) = delete;
  Stmt &operator=(Stmt &&) = delete;

  void *operator new(size_t bytes, const ASTContext &ctx,
                     unsigned alignment = 8) {
    return ::operator new(bytes, ctx, alignment);
  }
  void *operator new(size_t bytes, const ASTContext *ctx,
                     unsigned alignment = 8) {
    return ::operator new(bytes, *ctx, alignment);
  }

  void *operator new(size_t bytes) = delete;
  void operator delete(void *ptr) = delete;

  enum class StmtKind {
    ReturnStmt,
    WhileStmt,
    BreakStmt,
    ContinueStmt,
    ImportStmt,
    ExprStmt,
    DeclStmt,
  };

  StmtKind getKind() const { return kind; }
  size_t getSpanStart() const { return spanStart; }
  size_t getSpanEnd() const { return spanEnd; }

protected:
  Stmt(StmtKind kind, size_t spanStart, size_t spanEnd)
      : kind(kind), spanStart(spanStart), spanEnd(spanEnd) {};

private:
  StmtKind kind;
  size_t spanStart;
  size_t spanEnd;
};

// -----------------------------------------------------------------------------
// Derived Statements
// -----------------------------------------------------------------------------

/// Represents return statement, e.g. `return ...`
class ReturnStmt : public Stmt {
  Expr *returnValue;

public:
  ReturnStmt(size_t spanStart, size_t spanEnd, Expr *returnValue)
      : Stmt(StmtKind::ReturnStmt, spanStart, spanEnd),
        returnValue(returnValue) {}

  static bool classof(const Stmt *t) {
    return t->getKind() == StmtKind::ReturnStmt;
  }

  Expr *getReturnValue() { return returnValue; }
  const Expr *getReturnValue() const { return returnValue; }
};

/// Represents while statement, e.g. `while (...) {...}`
class WhileStmt : public Stmt {
  Expr *cond;
  BlockExpr *body;

public:
  WhileStmt(size_t spanStart, size_t spanEnd, Expr *cond, BlockExpr *body)
      : Stmt(StmtKind::WhileStmt, spanStart, spanEnd), cond(cond), body(body) {}

  static bool classof(const Stmt *t) {
    return t->getKind() == StmtKind::WhileStmt;
  }

  Expr *getCond() const { return cond; }
  BlockExpr *getBody() const { return body; }
};

/// Represents break statement, e.g. `break`
class BreakStmt : public Stmt {
public:
  BreakStmt(size_t spanStart, size_t spanEnd)
      : Stmt(StmtKind::BreakStmt, spanStart, spanEnd) {}

  static bool classof(const Stmt *t) {
    return t->getKind() == StmtKind::BreakStmt;
  }
};

/// Represents continue statement, e.g. `continue`
class ContinueStmt : public Stmt {
public:
  ContinueStmt(size_t spanStart, size_t spanEnd)
      : Stmt(StmtKind::ContinueStmt, spanStart, spanEnd) {}

  static bool classof(const Stmt *t) {
    return t->getKind() == StmtKind::ContinueStmt;
  }
};

/// Represents import statement, e.g. `import "..."`
class ImportStmt : public Stmt {
  StringLitExpr *value;

public:
  ImportStmt(size_t spanStart, size_t spanEnd, StringLitExpr *value)
      : Stmt(StmtKind::ImportStmt, spanStart, spanEnd), value(value) {}

  static bool classof(const Stmt *t) {
    return t->getKind() == StmtKind::ImportStmt;
  }

  StringLitExpr *getValue() const { return value; }
};

class ExprStmt : public Stmt {
  Expr *expr;

public:
  ExprStmt(size_t spanStart, size_t spanEnd, Expr *expr)
      : Stmt(StmtKind::ExprStmt, spanStart, spanEnd), expr(expr) {}

  static bool classof(const Stmt *t) {
    return t->getKind() == StmtKind::ExprStmt;
  }

  Expr *getExpr() { return expr; }
  const Expr *getExpr() const { return expr; }
};

class DeclStmt : public Stmt {
  Decl *decl;

public:
  DeclStmt(size_t spanStart, size_t spanEnd, Decl *decl)
      : Stmt(StmtKind::DeclStmt, spanStart, spanEnd), decl(decl) {}

  static bool classof(const Stmt *t) {
    return t->getKind() == StmtKind::DeclStmt;
  }

  Decl *getDecl() { return decl; }
  const Decl *getDecl() const { return decl; }
};

} // namespace ast
} // namespace belalang

#endif // BELALANG_AST_STMT_H_
