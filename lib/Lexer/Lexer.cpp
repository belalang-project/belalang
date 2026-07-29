#include "belalang/Lexer/Lexer.h"
#include "llvm/Support/ConvertUTF.h"

namespace belalang {
namespace lexer {

static char32_t decodeUTF8(const char *&ptr, const char *end) {
  if (ptr >= end)
    return 0;

  const auto *sourcePtr = reinterpret_cast<const llvm::UTF8 *>(ptr);
  const auto *sourceEnd = reinterpret_cast<const llvm::UTF8 *>(end);
  llvm::UTF32 codePoint = 0;

  llvm::ConversionResult res = llvm::convertUTF8Sequence(
      &sourcePtr, sourceEnd, &codePoint, llvm::strictConversion);

  if (res == llvm::conversionOK) {
    ptr = reinterpret_cast<const char *>(sourcePtr);
    return static_cast<char32_t>(codePoint);
  }

  auto fallback = static_cast<unsigned char>(*ptr);
  ++ptr;
  return fallback;
}

bool Lexer::lex(Token &result) {
  result.startToken();

  while (ptr < end) {
    if (*ptr == ' ' || *ptr == '\t' || *ptr == '\r' || *ptr == '\n') {
      ++ptr;
    } else if (*ptr == '#') {
      while (ptr < end && *ptr != '\n') {
        ++ptr;
      }
    } else {
      break;
    }
  }

  if (ptr >= end) {
    result.setKind(TokenKind::EndOfFile);
    return false;
  }

  offsetStart = ptr - start;
  const char *prevPtr = ptr;
  char32_t cp = decodeUTF8(ptr, end);

  switch (cp) {
  case ':':
    result.setKind(TokenKind::Colon);
    break;

  case '=':
    if (ptr < end) {
      if (*ptr == '=') {
        ++ptr;
        result.setKind(TokenKind::Eq);
        break;
      }
    }
    result.setKind(TokenKind::Assign);
    break;

  case '!':
    if (ptr < end) {
      if (*ptr == '=') {
        ++ptr;
        result.setKind(TokenKind::Ne);
        break;
      }
    }
    result.setKind(TokenKind::Not);
    break;

  case '&':
    if (ptr < end) {
      if (*ptr == '&') {
        ++ptr;
        result.setKind(TokenKind::And);
        break;
      }
      if (*ptr == '=') {
        ++ptr;
        result.setKind(TokenKind::BitAndAssign);
        break;
      }
    }
    result.setKind(TokenKind::BitAnd);
    break;

  case '|':
    if (ptr < end) {
      if (*ptr == '|') {
        ++ptr;
        result.setKind(TokenKind::Or);
        break;
      }
      if (*ptr == '=') {
        ++ptr;
        result.setKind(TokenKind::BitOrAssign);
        break;
      }
    }
    result.setKind(TokenKind::BitOr);
    break;

  case '^':
    if (ptr < end) {
      if (*ptr == '=') {
        ++ptr;
        result.setKind(TokenKind::BitXorAssign);
        break;
      }
    }
    result.setKind(TokenKind::BitXor);
    break;

  case '<':
    if (ptr < end) {
      if (*ptr == '=') {
        ++ptr;
        result.setKind(TokenKind::Le);
        break;
      }
      if (*ptr == '<') {
        ++ptr;
        if (ptr < end) {
          if (*ptr == '=') {
            ++ptr;
            result.setKind(TokenKind::ShiftLeftAssign);
            break;
          }
        }
        result.setKind(TokenKind::ShiftLeft);
        break;
      }
    }
    result.setKind(TokenKind::Lt);
    break;

  case '>':
    if (ptr < end) {
      if (*ptr == '=') {
        ++ptr;
        result.setKind(TokenKind::Ge);
        break;
      }
      if (*ptr == '>') {
        ++ptr;
        if (ptr < end) {
          if (*ptr == '=') {
            ++ptr;
            result.setKind(TokenKind::ShiftRightAssign);
            break;
          }
        }
        result.setKind(TokenKind::ShiftRight);
        break;
      }
    }
    result.setKind(TokenKind::Gt);
    break;

  case '+':
    if (ptr < end) {
      if (*ptr == '=') {
        ++ptr;
        result.setKind(TokenKind::AddAssign);
        break;
      }
    }
    result.setKind(TokenKind::Add);
    break;

  case '-':
    if (ptr < end) {
      if (*ptr == '=') {
        ++ptr;
        result.setKind(TokenKind::SubAssign);
        break;
      }
    }
    result.setKind(TokenKind::Sub);
    break;

  case '*':
    if (ptr < end) {
      if (*ptr == '=') {
        ++ptr;
        result.setKind(TokenKind::MulAssign);
        break;
      }
    }
    result.setKind(TokenKind::Mul);
    break;

  case '/':
    if (ptr < end) {
      if (*ptr == '=') {
        ++ptr;
        result.setKind(TokenKind::DivAssign);
        break;
      }
    }
    result.setKind(TokenKind::Div);
    break;

  case '%':
    if (ptr < end) {
      if (*ptr == '=') {
        ++ptr;
        result.setKind(TokenKind::ModAssign);
        break;
      }
    }
    result.setKind(TokenKind::Mod);
    break;

  case '(':
    result.setKind(TokenKind::LeftParen);
    break;

  case ')':
    result.setKind(TokenKind::RightParen);
    break;

  case '{':
    result.setKind(TokenKind::LeftBrace);
    break;

  case '}':
    result.setKind(TokenKind::RightBrace);
    break;

  case '[':
    result.setKind(TokenKind::LeftBracket);
    break;

  case ']':
    result.setKind(TokenKind::RightBracket);
    break;

  case ';':
    result.setKind(TokenKind::Semicolon);
    break;

  case ',':
    result.setKind(TokenKind::Comma);
    break;

  case '.':
    result.setKind(TokenKind::Dot);
    break;

  case '\\':
    result.setKind(TokenKind::Backslash);
    break;

  case '"':
    if (!lexString(result))
      return false;
    break;

  default:
    if (isIdentStart(cp)) {
      if (!lexIdentifier(result, prevPtr))
        return false;
      break;
    }
    if (cp >= '0' && cp <= '9') {
      if (!lexNumber(result, static_cast<char>(cp)))
        return false;
      break;
    }
    diag.print(diag::Diagnostic::error("Unknown token")
                   .withLabel(diag::Label::primary(
                       offsetStart, (size_t)(ptr - start), "Unknown token")));
    return false;
  }

  result.setSpan(offsetStart, (size_t)(ptr - start));
  return true;
}

bool Lexer::lexString(Token &result) {
  std::string str;

  while (ptr < end) {
    char c = *ptr;
    if (c == '\\') {
      size_t escapeStart = (size_t)(ptr - start);
      ++ptr;
      if (ptr >= end) {
        diag.print(diag::Diagnostic::error("Unclosed string")
                       .withLabel(diag::Label::primary(offsetStart,
                                                       (size_t)(ptr - start),
                                                       "Unclosed string")));
        return false;
      }

      switch (*ptr) {
      case 'n':
        str += '\n';
        ++ptr;
        break;
      case 'r':
        str += '\r';
        ++ptr;
        break;
      case 't':
        str += '\t';
        ++ptr;
        break;
      case '"':
        str += '"';
        ++ptr;
        break;
      case '\\':
        str += '\\';
        ++ptr;
        break;
      case 'x': {
        ++ptr; // consume the `x`

        if (ptr >= end || !isxdigit(static_cast<unsigned char>(*ptr))) {
          diag.print(diag::Diagnostic::error("Unknown escape string")
                         .withLabel(diag::Label::primary(
                             escapeStart, (size_t)(ptr - start) + 1,
                             "Unknown escape string")));
          return false;
        }
        unsigned char hi = static_cast<unsigned char>(*ptr++);

        if (ptr >= end || !isxdigit(static_cast<unsigned char>(*ptr))) {
          diag.print(diag::Diagnostic::error("Unknown escape string")
                         .withLabel(diag::Label::primary(
                             escapeStart, (size_t)(ptr - start) + 1,
                             "Unknown escape string")));
          return false;
        }
        unsigned char lo = static_cast<unsigned char>(*ptr++);

        auto hexVal = [](unsigned char c) -> unsigned char {
          if (c >= '0' && c <= '9')
            return c - '0';
          if (c >= 'a' && c <= 'f')
            return c - 'a' + 10;
          return c - 'A' + 10;
        };

        str += static_cast<char>((hexVal(hi) << 4) | hexVal(lo));
        break;
      }
      default:
        diag.print(diag::Diagnostic::error("Unknown escape string")
                       .withLabel(diag::Label::primary(
                           escapeStart, (size_t)(ptr - start) + 1,
                           "Unknown escape string")));
        return false;
      }
    } else if (c == '"') {
      ++ptr;
      result.setKind(TokenKind::LiteralString);
      result.setStr(str);
      return true;
    } else {
      str += c;
      ++ptr;
    }
  }

  diag.print(diag::Diagnostic::error("Unclosed string")
                 .withLabel(diag::Label::primary(
                     offsetStart, (size_t)(ptr - start), "Unclosed string")));
  return false;
}

bool Lexer::lexIdentifier(Token &result, const char *identStart) {
  const char *cur = ptr;

  while (cur < end) {
    const char *next = cur;
    char32_t cp = decodeUTF8(next, end);
    if (!isIdentContinue(cp))
      break;
    cur = next;
  }

  std::string ident(identStart, cur - identStart);
  ptr = cur;

#define KW(name, str)                                                          \
  if (ident == str)                                                            \
    result.setKind(TokenKind::name);                                           \
  else
#include "belalang/Lexer/TokenKinds.def"
  {
    result.setKind(TokenKind::Ident);
  }

  result.setStr(ident);
  return true;
}

bool Lexer::lexNumber(Token &result, char first) {
  std::string num;
  num += first;
  bool hasDecimal = false;

  while (ptr < end) {
    if (isdigit(static_cast<unsigned char>(*ptr))) {
      num += *ptr;
      ++ptr;
    } else if (*ptr == '.' && !hasDecimal) {
      hasDecimal = true;
      num += '.';
      ++ptr;
    } else {
      break;
    }
  }

  result.setKind(hasDecimal ? TokenKind::LiteralFloat
                            : TokenKind::LiteralInteger);
  result.setStr(num);
  return true;
}

} // namespace lexer
} // namespace belalang
