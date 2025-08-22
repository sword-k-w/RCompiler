#include "gtest/gtest.h"
#include "data_loader/data_loader.h"
#include "lexer/lexer.h"
#include "parser/parser.hpp"
#include "parser/node/crate.h"

#include "visitor/printer/printer.h"

template<class T>
T *TestCode(const std::string &s) {
  Lexer a(s);
  auto tokens = a.Run();
  Parser b(tokens);
  try {
    return b.Run<T>();
  } catch (Error &err) {
    for (auto &x : tokens) {
      x.Print(std::cerr);
      std::cerr << '\n';
    }
    std::cerr << tokens.size() << '\n';
    std::cerr << err.Info() << '\n';
    EXPECT_EQ(0, 1);
    return nullptr;
  }
}

TEST(ParserTest, ExpressionBasicTest1) {
  TestCode<ExpressionNode>("1");
}

// TEST(ParserTest, ExpressionBasicTest2) {
//   TestCode<ExpressionNode>("1+2");
// }
//
// TEST(ParserTest, ExpressionBasicTest3) {
//   TestCode<ExpressionNode>("a.b.c");
// }
//
// TEST(ParserTest, ExpressionBasicTest4) {
//   TestCode<ExpressionNode>("(a+b)*d[4]");
// }
//
// TEST(ParserTest, ExpressionBasicTest5) {
//   TestCode<ExpressionNode>("*a+&mut b");
// }
//
// TEST(ParserTest, ExpressionBasicTest6) {
//   TestCode<ExpressionNode>("a.b()+c()*e(f,g)");
// }

void TestTestcase(const std::string &s, bool print) {
  std::string input = LoadFromFile(s);
  CrateNode *root = TestCode<CrateNode>(input);
  if (print && root != nullptr) {
    Printer printer(std::cerr);
    printer.Prepare();
    printer.Visit(root);
  }
}

// TEST(ParserTest, TestcaseTest_Array1) {
//   TestTestcase("../testcase/semantic-1/array1/array1.rx", false);
// }
//
// TEST(ParserTest, TestcaseTest_Array2) {
//   TestTestcase("../testcase/semantic-1/array2/array2.rx", false);
// }
//
// TEST(ParserTest, TestcaseTest_Array3) {
//   TestTestcase("../testcase/semantic-1/array3/array3.rx", false);
// }
//
// TEST(ParserTest, TestcaseTest_Array4) {
//   TestTestcase("../testcase/semantic-1/array4/array4.rx", false);
// }
//
// TEST(ParserTest, TestcaseTest_Array5) {
//   TestTestcase("../testcase/semantic-1/array5/array5.rx", false);
// }
//
// TEST(ParserTest, TestcaseTest_Array6) {
//   TestTestcase("../testcase/semantic-1/array6/array6.rx", false);
// }
//
// TEST(ParserTest, TestcaseTest_Array7) {
//   TestTestcase("../testcase/semantic-1/array7/array7.rx", false);
// }
//
// TEST(ParserTest, TestcaseTest_Array8) {
//   TestTestcase("../testcase/semantic-1/array8/array8.rx", false);
// }
//
// TEST(ParserTest, TestcaseTest_Tmp) {
//   TestTestcase("../testcase/tmp.rx", true);
// }
