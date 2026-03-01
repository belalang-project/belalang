#ifndef BELALANG_LEXER_LEXER_
#define BELALANG_LEXER_LEXER_

#include <cstdint>
#include <optional>
#include <string>
#include <variant>

namespace belalang {

/// Literal types supported by the lexer
enum class LiteralKind : uint8_t {
  Integer,
  Float,
  String,
  Boolean,
};

/// Assignment types supported by the lexer
enum class AssignmentKind : uint8_t {
  /// Assignment operator `=`
  Assign,
  /// Colon assignment operator `:=`
  ColonAssign,
  /// Addition assignment operator `+=`
  AddAssign,
  /// Subtraction assignment operator `-=`
  SubAssign,
  /// Multiplication assignment operator `*=`
  MulAssign,
  /// Division assignment operator `/=`
  DivAssign,
  /// Modulo assignment operator `%=`
  ModAssign,
  /// Bitwise AND assignment operator `&=`
  BitAndAssign,
  /// Bitwise OR assignment operator `|=`
  BitOrAssign,
  /// Bitwise XOR assignment operator `^=`
  BitXorAssign,
  /// Shift left assignment operator `<<=`
  ShiftLeftAssign,
  /// Shift right assignment operator `>>=`
  ShiftRightAssign,
};

enum class PrefixKind : uint8_t {
  Not,
  Sub,
};

enum class InfixKind : uint8_t {
  Add,
  Sub,
  Mul,
  Div,
  Mod,
  Eq,
  Ne,
  Gt,
  Ge,
  Lt,
  Le,
  BitAnd,
  BitOr,
  BitXor,
  ShiftLeft,
  ShiftRight,
  Or,
  And,
};

enum class TokenKind : uint8_t {
  /// End of file marker
  Eof,

  /// Empty token placeholder
  Empty,

  /// Identifier token containing a variable or function name
  Ident,

  /// Literals
  Literal,

  /// Assignments
  Assign,

  /// Addition operator `+`
  Add,
  /// Subtraction operator `-`
  Sub,
  /// Multiplication operator `*`
  Mul,
  /// Division operator `/`
  Div,
  /// Modulo operator `%`
  Mod,

  /// Logical NOT operator `!`
  Not,
  /// Logical AND operator `&&`
  And,
  /// Logical OR operator `||`
  Or,

  /// Bitwise AND operator `&`
  BitAnd,
  /// Bitwise OR operator `|`
  BitOr,
  /// Bitwise XOR operator `^`
  BitXor,
  /// Shift left operator `<<`
  ShiftLeft,
  /// Shift right operator `>>`
  ShiftRight,

  /// Equality comparison operator `==`
  Eq,
  /// Inequality comparison operator `!=`
  Ne,

  /// Less than operator `<`
  Lt,
  /// Less than or equal operator `<=`
  Le,
  /// Greater than operator `>`
  Gt,
  /// Greater than or equal operator `>=`
  Ge,

  /// Left parenthesis `()`
  LeftParen,
  /// Right parenthesis `)`
  RightParen,

  /// Left brace `{`
  LeftBrace,
  /// Right brace `}`
  RightBrace,

  /// Left bracket `[`
  LeftBracket,
  /// Right bracket `]`
  RightBracket,

  /// Function keyword `fn`
  Function,
  /// While loop keyword `while`
  While,
  /// If conditional keyword `if`
  If,
  /// Else conditional keyword `else`
  Else,
  /// Return keyword `return`
  Return,

  /// Comma separator `,`
  Comma,
  /// Semicolon terminator `;`
  Semicolon,
  /// Backslash character `\`
  Backslash,
};

struct Token {
  TokenKind kind = TokenKind::Empty;

  std::variant<std::monostate, LiteralKind, AssignmentKind> sub_kind;
  std::string_view value;

  std::optional<LiteralKind> literal_kind() {
    if (std::holds_alternative<LiteralKind>(sub_kind)) {
      return std::get<LiteralKind>(sub_kind);
    }
    return std::nullopt;
  }

  std::optional<AssignmentKind> assign_kind() {
    if (std::holds_alternative<AssignmentKind>(sub_kind)) {
      return std::get<AssignmentKind>(sub_kind);
    }
    return std::nullopt;
  }
};

class Lexer {
public:
  explicit Lexer(const std::string &source);

  Token next_token();

private:
  Token consume_string();
  Token consume_identifier();
  Token consume_number();

  std::string_view source;
};

} // namespace belalang

#endif
