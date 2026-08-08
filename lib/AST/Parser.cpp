#include "belalang/AST/Parser.h"
#include "belalang/AST/ASTContext.h"
#include "belalang/AST/Type.h"
#include "llvm/ADT/SmallVector.h"

namespace belalang {
namespace ast {

// -----------------------------------------------------------------------------
// Token handling
// -----------------------------------------------------------------------------

void Parser::consumeToken() {
  tok = peekTok;
  lexer.lex(peekTok);
}

bool Parser::tryConsumeToken(lexer::TokenKind expectedTok) {
  if (tok.getKind() != expectedTok)
    return false;

  consumeToken();
  return true;
}

// -----------------------------------------------------------------------------
// Diagnostics
// -----------------------------------------------------------------------------

void Parser::errorAt(size_t spanStart, size_t spanEnd, std::string message,
                     std::string label) {
  diagEngine.print(diag::Diagnostic::error(std::move(message))
                       .withLabel(diag::Label::primary(spanStart, spanEnd,
                                                       std::move(label))));
  hasError = true;
}

void Parser::errorAt(size_t spanStart, size_t spanEnd, std::string message) {
  errorAt(spanStart, spanEnd, message, std::move(message));
}

// -----------------------------------------------------------------------------
// Parsing program
// -----------------------------------------------------------------------------

Program *Parser::parseProgram() {
  llvm::SmallVector<Stmt *, 0> stmts;
  while (tok.getKind() != lexer::TokenKind::EndOfFile && !hasError) {
    Stmt *stmt = parseStmt();
    if (!stmt)
      break;
    stmts.push_back(stmt);
  }

  if (hasError)
    return nullptr;

  Stmt **stmtArr = c.alloc<Stmt *>(stmts.size());
  std::copy(stmts.begin(), stmts.end(), stmtArr);
  return new (c) Program(llvm::ArrayRef<Stmt *>(stmtArr, stmts.size()));
}

// -----------------------------------------------------------------------------
// Parsing statements
// -----------------------------------------------------------------------------

Stmt *Parser::parseStmt() {
  switch (tok.getKind()) {
  case lexer::TokenKind::Return:
    return parseReturnStmt();
  case lexer::TokenKind::While:
    return parseWhileStmt();
  case lexer::TokenKind::Import:
    return parseImportStmt();
  case lexer::TokenKind::KwBreak:
    return parseBreakStmt();
  case lexer::TokenKind::KwContinue:
    return parseContinueStmt();
  case lexer::TokenKind::Struct:
    return parseDeclStmt();
  case lexer::TokenKind::Ident:
    if (peekTok.getKind() == lexer::TokenKind::Colon)
      return parseDeclStmt();
    return parseExprStmt();
  default:
    return parseExprStmt();
  };
}

Stmt *Parser::parseReturnStmt() {
  size_t start = tok.getSpanStart();
  size_t end = tok.getSpanEnd();
  consumeToken(); // consume `return`

  Expr *returnValue = nullptr;
  if (tok.getKind() != lexer::TokenKind::Semicolon &&
      tok.getKind() != lexer::TokenKind::RightBrace &&
      tok.getKind() != lexer::TokenKind::EndOfFile) {
    returnValue = parseExpr(Precedence::Lowest);
    if (returnValue) {
      end = returnValue->getSpanEnd();
    }
  }

  if (tok.getKind() == lexer::TokenKind::Semicolon) {
    hasSemicolon = true;
    end = tok.getSpanEnd();
    consumeToken(); // consume `;`
  } else {
    hasSemicolon = false;
  }

  return new (c) ReturnStmt(start, end, returnValue);
}

Stmt *Parser::parseWhileStmt() {
  size_t start = tok.getSpanStart();
  consumeToken(); // consume `while`

  Expr *cond;
  {
    auto guard = restrict(Restr::NoStructLiteral);
    cond = parseExpr(Precedence::Lowest);
  }
  if (!cond)
    return nullptr;

  BlockExpr *body = static_cast<BlockExpr *>(parseBlockExpr());
  if (!body)
    return nullptr;
  size_t end = body->getSpanEnd();

  return new (c) WhileStmt(start, end, cond, body);
}

Stmt *Parser::parseImportStmt() {
  size_t start = tok.getSpanStart();
  consumeToken(); // consume `import`
  StringLitExpr *value = static_cast<StringLitExpr *>(parseStringLitExpr());
  size_t end = value->getSpanEnd();

  if (tok.getKind() == lexer::TokenKind::Semicolon) {
    hasSemicolon = true;
    end = tok.getSpanEnd();
    consumeToken(); // consume `;`
  } else {
    hasSemicolon = false;
  }

  return new (c) ImportStmt(start, end, value);
}

Stmt *Parser::parseBreakStmt() {
  size_t start = tok.getSpanStart();
  size_t end = tok.getSpanEnd();
  consumeToken(); // consume `break`

  if (tok.getKind() == lexer::TokenKind::Semicolon) {
    hasSemicolon = true;
    end = tok.getSpanEnd();
    consumeToken(); // consume `;`
  } else {
    hasSemicolon = false;
  }

  return new (c) BreakStmt(start, end);
}

Stmt *Parser::parseContinueStmt() {
  size_t start = tok.getSpanStart();
  size_t end = tok.getSpanEnd();
  consumeToken(); // consume `continue`

  if (tok.getKind() == lexer::TokenKind::Semicolon) {
    hasSemicolon = true;
    end = tok.getSpanEnd();
    consumeToken(); // consume `;`
  } else {
    hasSemicolon = false;
  }

  return new (c) ContinueStmt(start, end);
}

Stmt *Parser::parseExprStmt() {
  size_t start = tok.getSpanStart();
  Expr *e = parseExpr(Precedence::Lowest);
  if (!e)
    return nullptr;

  size_t end = e->getSpanEnd();
  if (tok.getKind() == lexer::TokenKind::Semicolon) {
    hasSemicolon = true;
    end = tok.getSpanEnd();
    consumeToken(); // consume `;`
  } else {
    hasSemicolon = false;
  }

  return new (c) ExprStmt(start, end, e);
}

Stmt *Parser::parseDeclStmt() {
  Decl *d = parseDecl();
  if (!d)
    return nullptr;
  return new (c) DeclStmt(d->getSpanStart(), d->getSpanEnd(), d);
}

// -----------------------------------------------------------------------------
// Parsing expressions
// -----------------------------------------------------------------------------

Parser::Precedence Parser::precedenceOf(lexer::TokenKind kind) {
  switch (kind) {
  case lexer::TokenKind::Assign:
  case lexer::TokenKind::AddAssign:
  case lexer::TokenKind::SubAssign:
  case lexer::TokenKind::MulAssign:
  case lexer::TokenKind::DivAssign:
  case lexer::TokenKind::ModAssign:
  case lexer::TokenKind::BitAndAssign:
  case lexer::TokenKind::BitOrAssign:
  case lexer::TokenKind::BitXorAssign:
  case lexer::TokenKind::ShiftLeftAssign:
  case lexer::TokenKind::ShiftRightAssign:
  case lexer::TokenKind::Colon:
    return Precedence::AssignmentOps;
  case lexer::TokenKind::Or:
    return Precedence::LogicalOr;
  case lexer::TokenKind::And:
    return Precedence::LogicalAnd;
  case lexer::TokenKind::BitOr:
    return Precedence::BitOr;
  case lexer::TokenKind::BitXor:
    return Precedence::BitXor;
  case lexer::TokenKind::BitAnd:
    return Precedence::BitAnd;
  case lexer::TokenKind::Eq:
  case lexer::TokenKind::Ne:
    return Precedence::Equality;
  case lexer::TokenKind::Lt:
  case lexer::TokenKind::Le:
  case lexer::TokenKind::Gt:
  case lexer::TokenKind::Ge:
    return Precedence::Relational;
  case lexer::TokenKind::ShiftLeft:
  case lexer::TokenKind::ShiftRight:
    return Precedence::Shift;
  case lexer::TokenKind::Add:
  case lexer::TokenKind::Sub:
    return Precedence::Additive;
  case lexer::TokenKind::Div:
  case lexer::TokenKind::Mul:
  case lexer::TokenKind::Mod:
    return Precedence::Multiplicative;
  case lexer::TokenKind::LeftParen:
  case lexer::TokenKind::LeftBrace:
    return Precedence::Call;
  case lexer::TokenKind::LeftBracket:
  case lexer::TokenKind::Dot:
    return Precedence::Index;
  default:
    return Precedence::Lowest;
  }
}

Expr *Parser::parseExpr(Precedence prec) {
  Expr *left = parsePrefix();
  if (!left)
    return nullptr;

  while (prec < precedenceOf(tok.getKind())) {
    if (tok.getKind() == lexer::TokenKind::LeftBrace &&
        hasRestr(Restr::NoStructLiteral))
      break;
    left = parseInfix(left);
    if (!left)
      break;
  }

  return left;
}

Expr *Parser::parsePrefix() {
  switch (tok.getKind()) {
  case lexer::TokenKind::LiteralInteger:
    return parseIntLitExpr();
  case lexer::TokenKind::LiteralFloat:
    return parseFloatLitExpr();
  case lexer::TokenKind::LiteralString:
    return parseStringLitExpr();
  case lexer::TokenKind::KwTrue:
  case lexer::TokenKind::KwFalse:
    return parseBoolExpr();
  case lexer::TokenKind::LeftBrace:
    return parseBlockExpr();
  case lexer::TokenKind::Not:
  case lexer::TokenKind::Sub:
    return parsePrefixExpr();
  case lexer::TokenKind::Ident:
    return parseIdentifierExpr();
  case lexer::TokenKind::LeftBracket:
    return parseArrayLitExpr();
  case lexer::TokenKind::LeftParen:
    return parseGroupedExpr();
  case lexer::TokenKind::Function:
    return parseFunctionLitExpr();
  case lexer::TokenKind::If:
    return parseIfExpr();
  default:
    errorAt(tok.getSpanStart(), tok.getSpanEnd(), "unknown prefix");
    return nullptr;
  }
}

Expr *Parser::parseInfix(Expr *left) {
  switch (tok.getKind()) {
  case lexer::TokenKind::Add:
  case lexer::TokenKind::Sub:
  case lexer::TokenKind::Mul:
  case lexer::TokenKind::Div:
  case lexer::TokenKind::Mod:
  case lexer::TokenKind::Eq:
  case lexer::TokenKind::Ne:
  case lexer::TokenKind::Gt:
  case lexer::TokenKind::Ge:
  case lexer::TokenKind::Lt:
  case lexer::TokenKind::Le:
  case lexer::TokenKind::BitAnd:
  case lexer::TokenKind::BitOr:
  case lexer::TokenKind::BitXor:
  case lexer::TokenKind::ShiftLeft:
  case lexer::TokenKind::ShiftRight:
  case lexer::TokenKind::Or:
  case lexer::TokenKind::And:
    return parseInfixExpr(left);
  case lexer::TokenKind::LeftParen:
    return parseCallExpr(left);
  case lexer::TokenKind::LeftBracket:
    return parseIndexExpr(left);
  case lexer::TokenKind::Dot:
    return parseMemberAccessExpr(left);
  case lexer::TokenKind::LeftBrace:
    return parseStructLiteralExpr(left);
  case lexer::TokenKind::Assign:
  case lexer::TokenKind::AddAssign:
  case lexer::TokenKind::SubAssign:
  case lexer::TokenKind::MulAssign:
  case lexer::TokenKind::DivAssign:
  case lexer::TokenKind::ModAssign:
  case lexer::TokenKind::BitAndAssign:
  case lexer::TokenKind::BitOrAssign:
  case lexer::TokenKind::BitXorAssign:
  case lexer::TokenKind::ShiftLeftAssign:
  case lexer::TokenKind::ShiftRightAssign:
    return parseVarExpr(left);
  default:
    errorAt(tok.getSpanStart(), tok.getSpanEnd(), "unknown prefix");
    return nullptr;
  }
}

Expr *Parser::parseIntLitExpr() {
  size_t start = tok.getSpanStart();
  size_t end = tok.getSpanEnd();
  uint64_t value;

  if (tok.getStr().getAsInteger(10, value)) {
    errorAt(start, end, "error parsing integer");
    return nullptr;
  }

  consumeToken(); // consume the integer itself
  return new (c) IntLitExpr(start, end, value);
}

Expr *Parser::parseFloatLitExpr() {
  size_t start = tok.getSpanStart();
  size_t end = tok.getSpanEnd();
  double value;

  if (tok.getStr().getAsDouble(value)) {
    errorAt(start, end, "error parsing float");
    return nullptr;
  }

  consumeToken(); // consume the float itself
  return new (c) FloatLitExpr(start, end, value);
}

Expr *Parser::parseStringLitExpr() {
  size_t start = tok.getSpanStart();
  size_t end = tok.getSpanEnd();
  llvm::StringRef value = tok.getStr();

  char *strData = c.alloc<char>(value.size());
  std::copy(value.begin(), value.end(), strData);
  value = llvm::StringRef(strData, value.size());

  consumeToken(); // consume the string itself
  return new (c) StringLitExpr(start, end, value);
}

Expr *Parser::parseBoolExpr() {
  size_t start = tok.getSpanStart();
  size_t end = tok.getSpanEnd();
  bool value = tok.getKind() == lexer::TokenKind::KwTrue;

  consumeToken(); // consume the boolean itself
  return new (c) BoolExpr(start, end, value);
}

Expr *Parser::parseArrayLitExpr() {
  size_t start = tok.getSpanStart();
  consumeToken(); // consume opening `[`
  llvm::SmallVector<Expr *, 0> elements;

  while (tok.getKind() != lexer::TokenKind::RightBracket) {
    Expr *expr = parseExpr(Precedence::Lowest);
    if (!expr)
      return nullptr;
    elements.push_back(expr);

    if (tok.getKind() != lexer::TokenKind::Comma)
      break;
    consumeToken(); // consume `,`
  }

  size_t end = tok.getSpanEnd();
  if (!tryConsumeToken(lexer::TokenKind::RightBracket)) {
    errorAt(tok.getSpanStart(), tok.getSpanEnd(), "unexpected token");
    return nullptr;
  }

  Expr **elementsData = c.alloc<Expr *>(elements.size());
  std::copy(elements.begin(), elements.end(), elementsData);

  return new (c) ArrayLitExpr(
      start, end, llvm::ArrayRef<Expr *>(elementsData, elements.size()));
}

Expr *Parser::parseVarExpr(Expr *left) {
  if (left->getKind() != Expr::ExprKind::IdentifierExpr) {
    errorAt(left->getSpanStart(), left->getSpanEnd(), "invalid lhs");
    return nullptr;
  }

  size_t start = left->getSpanStart();
  lexer::TokenKind assignOp = tok.getKind();
  consumeToken(); // consume the assignment operator

  llvm::StringRef name = static_cast<IdentifierExpr *>(left)->getName();

  Expr *value = parseExpr(Precedence::Lowest);
  if (!value)
    return nullptr;
  size_t end = value->getSpanEnd();

  return new (c) VarExpr(start, end, name, value, assignOp);
}

Expr *Parser::parseFunctionLitExpr() {
  size_t start = tok.getSpanStart();
  consumeToken(); // consume `fn`

  // consume `(`
  if (!tryConsumeToken(lexer::TokenKind::LeftParen)) {
    errorAt(tok.getSpanStart(), tok.getSpanEnd(), "unexpected token");
    return nullptr;
  }

  llvm::SmallVector<VarDecl *, 0> params;
  while (tok.getKind() != lexer::TokenKind::RightParen) {
    size_t start = tok.getSpanStart();
    llvm::StringRef name = tok.getStr();
    char *strData = c.alloc<char>(name.size());
    std::copy(name.begin(), name.end(), strData);
    name = llvm::StringRef(strData, name.size());
    consumeToken(); // consume param name

    // consume `:`
    if (!tryConsumeToken(lexer::TokenKind::Colon)) {
      errorAt(tok.getSpanStart(), tok.getSpanEnd(), "unexpected token");
      return nullptr;
    }

    Type *ty = c.strToTy(tok.getStr());

    size_t end = tok.getSpanEnd();
    params.push_back(new (c) VarDecl(start, end, name, nullptr, ty));
    consumeToken(); // consume type

    if (tryConsumeToken(lexer::TokenKind::RightParen))
      break;

    if (!tryConsumeToken(lexer::TokenKind::Comma)) {
      errorAt(tok.getSpanStart(), tok.getSpanEnd(), "unexpected token");
      return nullptr;
    }
  }

  if (tok.getKind() == lexer::TokenKind::RightParen)
    consumeToken();

  ast::Type *type = nullptr;
  if (tryConsumeToken(lexer::TokenKind::Colon)) {
    type = c.strToTy(tok.getStr());
    consumeToken(); // consume type name
  }

  BlockExpr *body = static_cast<BlockExpr *>(parseBlockExpr());
  if (!body)
    return nullptr;
  size_t end = body->getSpanEnd();

  VarDecl **paramsData = c.alloc<VarDecl *>(params.size());
  std::copy(params.begin(), params.end(), paramsData);
  auto paramsC = llvm::ArrayRef(paramsData, params.size());

  return new (c) FunctionLitExpr(start, end, paramsC, body, type);
}

Expr *Parser::parseCallExpr(Expr *left) {
  size_t start = left->getSpanStart();
  consumeToken(); // consume `(`

  llvm::SmallVector<Expr *, 0> args;
  while (tok.getKind() != lexer::TokenKind::RightParen) {
    Expr *arg = parseExpr(Precedence::Lowest);
    if (!arg)
      return nullptr;
    args.push_back(arg);

    if (tok.getKind() != lexer::TokenKind::Comma)
      break;
    consumeToken(); // consume `,`
  }

  size_t end = tok.getSpanEnd();
  if (!tryConsumeToken(lexer::TokenKind::RightParen)) {
    errorAt(tok.getSpanStart(), tok.getSpanEnd(), "unexpected token");
    return nullptr;
  }

  Expr **argsData = c.alloc<Expr *>(args.size());
  std::copy(args.begin(), args.end(), argsData);

  return new (c)
      CallExpr(start, end, left, llvm::ArrayRef<Expr *>(argsData, args.size()));
}

Expr *Parser::parseIndexExpr(Expr *left) {
  size_t start = left->getSpanStart();
  consumeToken(); // consume `[`

  Expr *index = parseExpr(Precedence::Lowest);
  if (!index)
    return nullptr;

  size_t end = tok.getSpanEnd();
  if (!tryConsumeToken(lexer::TokenKind::RightBracket)) {
    errorAt(tok.getSpanStart(), tok.getSpanEnd(), "unexpected token");
    return nullptr;
  }

  return new (c) IndexExpr(start, end, left, index);
}

Expr *Parser::parseIdentifierExpr() {
  size_t start = tok.getSpanStart();
  size_t end = tok.getSpanEnd();
  llvm::StringRef name = tok.getStr();

  char *strData = c.alloc<char>(name.size());
  std::copy(name.begin(), name.end(), strData);
  name = llvm::StringRef(strData, name.size());

  consumeToken(); // consume the identifier
  return new (c) IdentifierExpr(start, end, name);
}

Expr *Parser::parseIfExpr() {
  size_t start = tok.getSpanStart();
  consumeToken(); // consume `if`

  Expr *cond;
  {
    auto guard = restrict(Restr::NoStructLiteral);
    cond = parseExpr(Precedence::Lowest);
  }
  if (!cond)
    return nullptr;

  BlockExpr *then = static_cast<BlockExpr *>(parseBlockExpr());
  if (!then)
    return nullptr;
  size_t end = then->getSpanEnd();

  Expr *alt = nullptr;
  if (tryConsumeToken(lexer::TokenKind::Else)) {
    if (tok.getKind() == lexer::TokenKind::If)
      alt = parseIfExpr();
    else if (tok.getKind() == lexer::TokenKind::LeftBrace)
      alt = parseBlockExpr();
    else {
      errorAt(tok.getSpanStart(), tok.getSpanEnd(), "unexpected token");
      return nullptr;
    }
  }

  if (alt)
    end = alt->getSpanEnd();

  return new (c) IfExpr(start, end, cond, then, alt);
}

Expr *Parser::parseInfixExpr(Expr *left) {
  size_t start = left->getSpanStart();
  lexer::TokenKind infixOp = tok.getKind();
  Precedence prec = precedenceOf(infixOp);
  consumeToken(); // consume the operator

  Expr *right = parseExpr(prec);
  if (!right)
    return nullptr;
  size_t end = right->getSpanEnd();

  return new (c) InfixExpr(start, end, left, right, infixOp);
}

Expr *Parser::parsePrefixExpr() {
  size_t start = tok.getSpanStart();
  lexer::TokenKind prefixTok = tok.getKind();
  consumeToken(); // consume the prefix

  Expr *right = parseExpr(Precedence::Prefix);
  if (!right)
    return nullptr;
  size_t end = right->getSpanEnd();

  return new (c) PrefixExpr(start, end, prefixTok, right);
}

Expr *Parser::parseBlockExpr() {
  llvm::SmallVector<Stmt *, 0> stmts;

  size_t start = tok.getSpanStart();
  if (!tryConsumeToken(lexer::TokenKind::LeftBrace)) {
    errorAt(tok.getSpanStart(), tok.getSpanEnd(), "unexpected token");
    return nullptr;
  }

  while (tok.getKind() != lexer::TokenKind::EndOfFile &&
         tok.getKind() != lexer::TokenKind::RightBrace && !hasError) {
    Stmt *stmt = parseStmt();
    if (!stmt)
      break;
    stmts.push_back(stmt);
  }

  size_t end = tok.getSpanEnd();
  if (!tryConsumeToken(lexer::TokenKind::RightBrace)) {
    errorAt(tok.getSpanStart(), tok.getSpanEnd(), "unexpected token");
    return nullptr;
  }

  // Copy the statemnts data to the context's bump allocator.
  Stmt **stmtsData = c.alloc<Stmt *>(stmts.size());
  std::copy(stmts.begin(), stmts.end(), stmtsData);

  return new (c)
      BlockExpr(start, end, llvm::ArrayRef<Stmt *>(stmtsData, stmts.size()));
}

Expr *Parser::parseMemberAccessExpr(Expr *left) {
  size_t start = left->getSpanStart();
  consumeToken(); // consume `.`

  llvm::StringRef member = tok.getStr();
  char *strData = c.alloc<char>(member.size());
  std::copy(member.begin(), member.end(), strData);
  member = llvm::StringRef(strData, member.size());

  size_t end = tok.getSpanEnd();
  consumeToken(); // consume the member identifier

  return new (c) MemberAccessExpr(start, end, left, member);
}

Expr *Parser::parseStructLiteralExpr(Expr *left) {
  size_t start = left->getSpanStart();
  consumeToken(); // consume `{`

  llvm::SmallVector<VarExpr *, 0> fields;
  while (tok.getKind() != lexer::TokenKind::RightBrace) {
    size_t fieldStart = tok.getSpanStart();
    llvm::StringRef fieldName = tok.getStr();
    char *strData = c.alloc<char>(fieldName.size());
    std::copy(fieldName.begin(), fieldName.end(), strData);
    fieldName = llvm::StringRef(strData, fieldName.size());
    size_t fieldEnd = tok.getSpanEnd();
    consumeToken(); // consume the field identifier

    if (!tryConsumeToken(lexer::TokenKind::Assign)) {
      errorAt(fieldStart, fieldEnd, "error parsing struct",
              "expected field assignment");
      return nullptr;
    }

    Expr *fieldValue = parseExpr(Precedence::Lowest);
    if (!fieldValue)
      return nullptr;
    fields.push_back(new (c) VarExpr(fieldStart, fieldValue->getSpanEnd(),
                                     fieldName, fieldValue,
                                     lexer::TokenKind::Assign));

    if (tok.getKind() != lexer::TokenKind::Comma)
      break;
    consumeToken(); // consume `,`
  }

  size_t end = tok.getSpanEnd();
  if (!tryConsumeToken(lexer::TokenKind::RightBrace)) {
    errorAt(tok.getSpanStart(), tok.getSpanEnd(), "error parsing struct",
            "unexpected token");
    return nullptr;
  }

  if (left->getKind() != Expr::ExprKind::IdentifierExpr) {
    errorAt(left->getSpanStart(), left->getSpanEnd(), "error parsing struct",
            "invalid struct name");
    return nullptr;
  }

  llvm::StringRef structName = static_cast<IdentifierExpr *>(left)->getName();

  VarExpr **fieldsData = c.alloc<VarExpr *>(fields.size());
  std::copy(fields.begin(), fields.end(), fieldsData);

  return new (c)
      StructLiteralExpr(start, end, structName,
                        llvm::ArrayRef<VarExpr *>(fieldsData, fields.size()));
}

Expr *Parser::parseGroupedExpr() {
  size_t start = tok.getSpanStart();
  consumeToken(); // consume opening `(`

  Expr *expr = nullptr;
  {
    auto guard = restrict(Restr::None);
    expr = parseExpr(Precedence::Lowest);
  }
  if (!expr)
    return nullptr;

  if (!tryConsumeToken(lexer::TokenKind::RightParen)) {
    errorAt(tok.getSpanStart(), tok.getSpanEnd(), "unexpected token");
    return nullptr;
  }

  return expr;
}

// -----------------------------------------------------------------------------
// Parsing declarations
// -----------------------------------------------------------------------------

Decl *Parser::parseDecl() {
  switch (tok.getKind()) {
  case lexer::TokenKind::Struct:
    return parseStructDecl();
  case lexer::TokenKind::Ident:
    return parseVarDecl();
  default:
    errorAt(tok.getSpanStart(), tok.getSpanEnd(), "unexpected token");
    return nullptr;
  }
}

Decl *Parser::parseVarDecl() {
  size_t start = tok.getSpanStart();

  llvm::StringRef name = tok.getStr();
  char *nameData = c.alloc<char>(name.size());
  std::copy(name.begin(), name.end(), nameData);
  name = llvm::StringRef(nameData, name.size());

  consumeToken(); // consume the identifier
  consumeToken(); // consume `:`

  ast::Type *explicitType = nullptr;
  if (tok.getKind() == lexer::TokenKind::Assign) {
    // `name := value`; no explicit type, `tok` is the `=`.
  } else if (tok.getKind() == lexer::TokenKind::Ident) {
    explicitType = c.strToTy(tok.getStr());
    consumeToken(); // consume the type name
  } else {
    errorAt(tok.getSpanStart(), tok.getSpanEnd(), "unexpected token");
    return nullptr;
  }

  Expr *value = nullptr;
  if (tok.getKind() == lexer::TokenKind::Assign) {
    consumeToken(); // consume `=`; `tok` is now the start of the value
    value = parseExpr(Precedence::Lowest);
  }
  if (hasError)
    return nullptr;

  size_t end = value ? value->getSpanEnd() : tok.getSpanEnd();
  if (tok.getKind() == lexer::TokenKind::Semicolon) {
    hasSemicolon = true;
    end = tok.getSpanEnd();
    consumeToken(); // consume `;`
  } else {
    hasSemicolon = false;
  }

  return new (c) VarDecl(start, end, name, value, explicitType);
}

Decl *Parser::parseStructDecl() {
  size_t start = tok.getSpanStart();
  size_t structEnd;
  consumeToken(); // consume `struct`

  llvm::StringRef name = tok.getStr();
  char *strData = c.alloc<char>(name.size());
  std::copy(name.begin(), name.end(), strData);
  name = llvm::StringRef(strData, name.size());
  consumeToken(); // consume name

  if (!tryConsumeToken(lexer::TokenKind::LeftBrace)) {
    errorAt(tok.getSpanStart(), tok.getSpanEnd(), "error parsing struct",
            "unexpected token");
    return nullptr;
  }

  llvm::SmallVector<VarDecl *, 0> members;
  while (tok.getKind() != lexer::TokenKind::RightBrace && !hasError) {
    size_t start = tok.getSpanStart();
    llvm::StringRef name = tok.getStr();

    char *strData = c.alloc<char>(name.size());
    std::copy(name.begin(), name.end(), strData);
    name = llvm::StringRef(strData, name.size());
    consumeToken(); // consume member name

    // consume `:`
    if (!tryConsumeToken(lexer::TokenKind::Colon)) {
      errorAt(start, tok.getSpanEnd(), "error parsing struct",
              "invalid statement");
      return nullptr;
    }

    ast::Type *ty = c.strToTy(tok.getStr());

    size_t end = tok.getSpanEnd();
    members.push_back(new (c) VarDecl(start, end, name, nullptr, ty));
    consumeToken(); // consume type

    structEnd = tok.getSpanEnd();
    if (tryConsumeToken(lexer::TokenKind::RightBrace))
      break;

    tryConsumeToken(lexer::TokenKind::Semicolon);
  }

  if (tok.getKind() == lexer::TokenKind::RightBrace)
    consumeToken();

  VarDecl **membersData = c.alloc<VarDecl *>(members.size());
  std::copy(members.begin(), members.end(), membersData);

  return new (c)
      StructDecl(start, structEnd, name,
                 llvm::ArrayRef<VarDecl *>(membersData, members.size()));
}

} // namespace ast
} // namespace belalang
