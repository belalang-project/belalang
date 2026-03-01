#include <belalang/Lexer/Lexer.hpp>
#include <gtest/gtest.h>

TEST(Lexer, CheckConsumeIdentifier) {
  belalang::Lexer lexer = belalang::Lexer("hello");
  belalang::Token tok = lexer.next_token();

  EXPECT_EQ(tok.kind, belalang::TokenKind::Ident);
  EXPECT_EQ(tok.value, "hello");
}

TEST(Lexer, CheckConsumeNumber) {
  belalang::Lexer lexer = belalang::Lexer("86");
  belalang::Token tok = lexer.next_token();

  EXPECT_EQ(tok.kind, belalang::TokenKind::Literal);
  EXPECT_EQ(tok.value, "86");
  EXPECT_EQ(tok.literal_kind().value(), belalang::LiteralKind::Integer);
}

TEST(Lexer, CheckConsumeString) {
  belalang::Lexer lexer = belalang::Lexer("\"Hello\"");
  belalang::Token tok = lexer.next_token();

  EXPECT_EQ(tok.kind, belalang::TokenKind::Literal);
  EXPECT_EQ(tok.value, "Hello");
  EXPECT_EQ(tok.literal_kind().value(), belalang::LiteralKind::String);
}
