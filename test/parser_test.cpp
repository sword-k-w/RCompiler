#include "gtest/gtest.h"
#include "lexer/lexer.h"
#include "parser/parser.hpp"

#include "parser/node/crate.h"

#include "visitor/printer/printer.h"

template<class T>
void TestCode(const std::string &s) {
  Lexer a(s);
  auto tokens = a.Run();
  Parser b(tokens);
  try {
    T *root = b.Run<T>();
    Printer printer(std::cerr);
    printer.Prepare();
    printer.Visit(root);
  } catch (Error &err) {
    for (auto &x : tokens) {
      x.Print(std::cerr);
      std::cerr << '\n';
    }
    std::cerr << err.Info() << '\n';
    EXPECT_EQ(0, 1);
  }
}

TEST(ParserTest, ExpressionBasicTest1) {
  TestCode<ExpressionNode>("1");
}

TEST(ParserTest, ExpressionBasicTest2) {
  TestCode<ExpressionNode>("1+2");
}

// TEST(ParserTest, ExpressionBasicTest3) {
//   TestCode<ExpressionNode>("a.b.c");
// }
//
// TEST(ParserTest, ExpressionBasicTest4) {
//   TestCode<ExpressionNode>("(a+b)*d[4]");
// }

TEST(ParserTest, ExpressionBasicTest5) {
  TestCode<ExpressionNode>("*a+&mut b");
}
