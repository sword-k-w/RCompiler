#include "gtest/gtest.h"
#include "lexer/lexer.h"
#include "parser/parser.hpp"

#include "parser/node/crate.h"

TEST(ParserTest, ExpressionTest) {
  Lexer a("1+2");
  Parser b(a.Run());
  try {
    b.Run<ExpressionNode>();
  } catch (Error &err) {
    std::cerr << err.Info() << '\n';
  }
}