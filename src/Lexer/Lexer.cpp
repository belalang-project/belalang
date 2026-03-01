#include <belalang/Lexer/Lexer.hpp>
#include <cctype>

namespace belalang {

Lexer::Lexer(const std::string &source) : source(source) {};

Token Lexer::next_token() {
  char first = source[0];

  if (std::isalpha(first)) {
    return consume_identifier();
  }

  if (std::isdigit(first)) {
    return consume_number();
  }

  if (first == '"') {
    return consume_string();
  }

  return Token{TokenKind::Eof};
}

Token Lexer::consume_identifier() {
  size_t len = 0;

  while (len < source.size() && std::isalnum(source[len])) {
    len++;
  }

  std::string_view ident = source.substr(0, len);
  source.remove_prefix(len);

  return Token{TokenKind::Ident, {}, ident};
}

Token Lexer::consume_string() {
  size_t len = 0;
  source.remove_prefix(1); // remove opening "

  while (source[len] != '"') {
    len++;
  }

  std::string_view str = source.substr(0, len);
  source.remove_prefix(len + 1); // remove string contents + closing "

  return Token{TokenKind::Literal, LiteralKind::String, str};
}

Token Lexer::consume_number() {
  size_t len = 0;

  while (std::isdigit(source[len])) {
    len++;
  }

  std::string_view num = source.substr(0, len);
  source.remove_prefix(len);

  return Token{TokenKind::Literal, LiteralKind::Integer, num};
}

} // namespace belalang
