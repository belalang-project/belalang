#ifndef BELALANG_LEXER_TOKEN_H_
#define BELALANG_LEXER_TOKEN_H_

#include <cctype>
#include <string>

#include "belalang/Lexer/XID.h"
#include "llvm/ADT/StringRef.h"

namespace belalang {
namespace lexer {

// -----------------------------------------------------------------------------
// TokenKind
// -----------------------------------------------------------------------------

enum class TokenKind {
#define TOK(name) name,
#include "TokenKinds.def"
};

// -----------------------------------------------------------------------------
// Token
// -----------------------------------------------------------------------------

class Token {
  TokenKind kind;
  size_t spanStart;
  size_t spanEnd;
  std::string tokenStr;
  llvm::StringRef str;

public:
  Token() : kind(TokenKind::Unknown) {}
  Token(TokenKind kind) : kind(kind) {}
  Token(const Token &other)
      : kind(other.kind), spanStart(other.spanStart), spanEnd(other.spanEnd),
        tokenStr(other.tokenStr), str(tokenStr) {}
  Token &operator=(const Token &other) {
    if (this != &other) {
      kind = other.kind;
      spanStart = other.spanStart;
      spanEnd = other.spanEnd;
      tokenStr = other.tokenStr;
      str = tokenStr;
    }
    return *this;
  }

  TokenKind getKind() { return kind; }
  llvm::StringRef getStr() { return str; }

  size_t getSpanStart() { return spanStart; }
  size_t getSpanEnd() { return spanEnd; }
  std::pair<size_t, size_t> getSpan() { return {spanStart, spanEnd}; }

  void setKind(TokenKind k) { kind = k; }
  void setSpan(size_t start, size_t end) {
    spanStart = start;
    spanEnd = end;
  }
  void setStr(llvm::StringRef s) {
    tokenStr = s.str();
    str = tokenStr;
  }

  void startToken() {
    kind = TokenKind::Unknown;
    tokenStr.clear();
    str = "";
  }

  const char *getString() {
    switch (kind) {
#define TOK(name)                                                              \
  case TokenKind::name:                                                        \
    return #name;
#include "TokenKinds.def"
    }
  };
};

inline bool isIdentStart(char32_t cp) {
  return isalpha(static_cast<unsigned char>(cp)) || cp == '_' ||
         is_xid_start(cp);
}

inline bool isIdentContinue(char32_t cp) {
  return isalnum(static_cast<unsigned char>(cp)) || cp == '_' ||
         is_xid_continue(cp);
}

} // namespace lexer
} // namespace belalang

#endif // BELALANG_LEXER_TOKEN_H_
