#include "gtest/gtest.h"
#include "lexer/lexer.h"
#include "parser/parser.hpp"

#include "parser/node/crate.h"

TEST(ParserTest, ExpressionBasicTest1) {
  Lexer a("1");
  Parser b(a.Run());
  try {
    b.Run<ExpressionNode>();
  } catch (Error &err) {
    std::cerr << err.Info() << '\n';
    EXPECT_EQ(0, 1);
  }
}

TEST(ParserTest, ExpressionBasicTest2) {
  Lexer a("1+2");
  Parser b(a.Run());
  try {
    b.Run<ExpressionNode>();
  } catch (Error &err) {
    std::cerr << err.Info() << '\n';
    EXPECT_EQ(0, 1);
  }
}