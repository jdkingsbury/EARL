#include "lexer.hpp"
#include "token.hpp"
#include "test-suite.hpp"

test_errno_t
test_lexer_next_token(void)
{
  char *filepath = "test/sample-input/basic-lexer-variables.1.in";
  std::vector<std::string> keywords = { "let", "return" };
  std::vector<std::string> types = { "int", "str" };
  std::string comment = "#";

  Lexer lexer = lex_file(filepath, keywords, types, comment);

  TEST_ASSERT_EQ(lexer.peek()->lexeme(), keywords[0]);
  TEST_ASSERT_EQ(lexer.next()->type(), TokenType::Keyword);
  TEST_ASSERT_EQ(lexer.peek()->lexeme(), "a");
  TEST_ASSERT_EQ(lexer.next()->type(), TokenType::Ident);
  TEST_ASSERT_EQ(lexer.peek()->lexeme(), ":");
  TEST_ASSERT_EQ(lexer.next()->type(), TokenType::Colon);
  TEST_ASSERT_EQ(lexer.peek()->lexeme(), "int");
  TEST_ASSERT_EQ(lexer.next()->type(), TokenType::Type);
  TEST_ASSERT_EQ(lexer.peek()->lexeme(), "=");
  TEST_ASSERT_EQ(lexer.next()->type(), TokenType::Equals);
  TEST_ASSERT_EQ(lexer.peek()->lexeme(), "1");
  TEST_ASSERT_EQ(lexer.next()->type(), TokenType::Intlit);
  TEST_ASSERT_EQ(lexer.peek()->lexeme(), ";");
  TEST_ASSERT_EQ(lexer.next()->type(), TokenType::Semicolon);
  TEST_ASSERT_EQ(lexer.peek()->type(), TokenType::Eof);

  return TEST_OK;
}


