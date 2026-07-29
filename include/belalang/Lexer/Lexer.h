#ifndef BELALANG_LEXER_LEXER_H_
#define BELALANG_LEXER_LEXER_H_

#include "belalang/Diag/Diag.h"
#include "belalang/Lexer/Token.h"
#include "llvm/ADT/StringRef.h"

namespace belalang {
namespace lexer {

// -----------------------------------------------------------------------------
// Lexer
// -----------------------------------------------------------------------------

class Lexer {
  const char *ptr;
  const char *start;
  const char *end;
  const diag::DiagnosticEngine &diag;

  size_t offsetStart;

public:
  Lexer(llvm::StringRef source, const diag::DiagnosticEngine &diag)
      : ptr(source.data()), start(source.data()),
        end(source.data() + source.size()), diag(diag) {}

  bool lex(Token &result);

private:
  bool lexIdentifier(Token &result, const char *identStart);
  bool lexString(Token &result);
  bool lexNumber(Token &result, char first);
};

} // namespace lexer
} // namespace belalang

#endif // BELALANG_LEXER_LEXER_H_
