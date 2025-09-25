#include "gtest/gtest.h"
#include "data_loader/data_loader.h"
#include "lexer/lexer.h"
#include "parser/parser.hpp"
#include "parser/node/crate.h"

#include "visitor/printer/printer.h"
#include "visitor/checker/first_checker.h"
#include "visitor/checker/second_checker.h"
#include "visitor/checker/third_checker.h"
#include "semantic/scope/scope.h"

void TestCode(const std::string &s, bool expect_result) {
  Lexer a(s);
  auto tokens = a.Run();
  Parser b(tokens);
  try {
    auto root = b.Run<CrateNode>();
    FirstChecker fc;
    fc.Run(root.get());
    Printer printer(std::cerr);
    printer.Prepare();
    printer.Visit(root.get());
    SecondChecker sc;
    root->Accept(&sc);
    ThirdChecker tc;
    root->Accept(&tc);
  } catch (Error &err) {
    std::cerr << err.Info() << '\n';
    EXPECT_EQ(expect_result, false);
    return;
  }
  EXPECT_EQ(expect_result, true);
}

void TestTestcase(const std::string &s, bool expect_result) {
  std::string input = LoadFromFile(s);
  TestCode(input, expect_result);
}

TEST(SemanticTest, TmpTest) {
  TestTestcase("../RCompiler-Testcases/tmp.rx", false);
}

TEST(SemanticTest, TestcaseTest_Array1) {
  TestTestcase("../RCompiler-Testcases/semantic-1/array1/array1.rx", true);
}

TEST(SemanticTest, TestcaseTest_Array2) {
  TestTestcase("../RCompiler-Testcases/semantic-1/array2/array2.rx", true);
}

TEST(SemanticTest, TestcaseTest_Array3) {
  TestTestcase("../RCompiler-Testcases/semantic-1/array3/array3.rx", true);
}

TEST(SemanticTest, TestcaseTest_Array4) {
  TestTestcase("../RCompiler-Testcases/semantic-1/array4/array4.rx", false);
}

TEST(SemanticTest, TestcaseTest_Array5) {
  TestTestcase("../RCompiler-Testcases/semantic-1/array5/array5.rx", false);
}

TEST(SemanticTest, TestcaseTest_Array6) {
  TestTestcase("../RCompiler-Testcases/semantic-1/array6/array6.rx", false);
}

TEST(SemanticTest, TestcaseTest_Array7) {
  TestTestcase("../RCompiler-Testcases/semantic-1/array7/array7.rx", false);
}

TEST(SemanticTest, TestcaseTest_Array8) {
  TestTestcase("../RCompiler-Testcases/semantic-1/array8/array8.rx", false);
}

// TEST(SemanticTest, TestcaseTest_Basic1) {
//   TestTestcase("../RCompiler-Testcases/semantic-1/basic1/basic1.rx", true);
// }
//
// TEST(SemanticTest, TestcaseTest_Basic2) {
//   TestTestcase("../RCompiler-Testcases/semantic-1/basic2/basic2.rx", false);
// }
//
// TEST(SemanticTest, TestcaseTest_Basic3) {
//   TestTestcase("../RCompiler-Testcases/semantic-1/basic3/basic3.rx", false);
// }
//
// TEST(SemanticTest, TestcaseTest_Basic4) {
//   TestTestcase("../RCompiler-Testcases/semantic-1/basic4/basic4.rx", true);
// }
//
// TEST(SemanticTest, TestcaseTest_Basic5) {
//   TestTestcase("../RCompiler-Testcases/semantic-1/basic5/basic5.rx", false);
// }
//
// TEST(SemanticTest, TestcaseTest_Basic6) {
//   TestTestcase("../RCompiler-Testcases/semantic-1/basic6/basic6.rx", false);
// }
//
// TEST(SemanticTest, TestcaseTest_Basic7) {
//   TestTestcase("../RCompiler-Testcases/semantic-1/basic7/basic7.rx", false);
// }
//
// TEST(SemanticTest, TestcaseTest_Basic8) {
//   TestTestcase("../RCompiler-Testcases/semantic-1/basic8/basic8.rx", false);
// }
//
// TEST(SemanticTest, TestcaseTest_Basic9) {
//   TestTestcase("../RCompiler-Testcases/semantic-1/basic9/basic9.rx", false);
// }
//
// TEST(SemanticTest, TestcaseTest_Basic10) {
//   TestTestcase("../RCompiler-Testcases/semantic-1/basic10/basic10.rx", false);
// }
//
// TEST(SemanticTest, TestcaseTest_Basic11) {
//   TestTestcase("../RCompiler-Testcases/semantic-1/basic11/basic11.rx", true);
// }
//
// TEST(SemanticTest, TestcaseTest_Basic12) {
//   TestTestcase("../RCompiler-Testcases/semantic-1/basic12/basic12.rx", false);
// }
//
// TEST(SemanticTest, TestcaseTest_Basic13) {
//   TestTestcase("../RCompiler-Testcases/semantic-1/basic13/basic13.rx", false);
// }
//
// TEST(SemanticTest, TestcaseTest_Basic14) {
//   TestTestcase("../RCompiler-Testcases/semantic-1/basic14/basic14.rx", false);
// }
//
// TEST(SemanticTest, TestcaseTest_Basic15) {
//   TestTestcase("../RCompiler-Testcases/semantic-1/basic15/basic15.rx", false);
// }
//
// TEST(SemanticTest, TestcaseTest_Basic16) {
//   TestTestcase("../RCompiler-Testcases/semantic-1/basic16/basic16.rx", false);
// }
//
// TEST(SemanticTest, TestcaseTest_Basic17) {
//   TestTestcase("../RCompiler-Testcases/semantic-1/basic17/basic17.rx", true);
// }
//
// TEST(SemanticTest, TestcaseTest_Basic18) {
//   TestTestcase("../RCompiler-Testcases/semantic-1/basic18/basic18.rx", true);
// }
//
// TEST(SemanticTest, TestcaseTest_Basic19) {
//   TestTestcase("../RCompiler-Testcases/semantic-1/basic19/basic19.rx", true);
// }
//
// TEST(SemanticTest, TestcaseTest_Basic20) {
//   TestTestcase("../RCompiler-Testcases/semantic-1/basic20/basic20.rx", true);
// }
//
// TEST(SemanticTest, TestcaseTest_Basic21) {
//   TestTestcase("../RCompiler-Testcases/semantic-1/basic21/basic21.rx", true);
// }
//
// TEST(SemanticTest, TestcaseTest_Basic22) {
//   TestTestcase("../RCompiler-Testcases/semantic-1/basic22/basic22.rx", true);
// }
//
// TEST(SemanticTest, TestcaseTest_Basic23) {
//   TestTestcase("../RCompiler-Testcases/semantic-1/basic23/basic23.rx", true);
// }
//
// TEST(SemanticTest, TestcaseTest_Basic24) {
//   TestTestcase("../RCompiler-Testcases/semantic-1/basic24/basic24.rx", true);
// }
//
// TEST(SemanticTest, TestcaseTest_Basic25) {
//   TestTestcase("../RCompiler-Testcases/semantic-1/basic25/basic25.rx", true);
// }
//
// TEST(SemanticTest, TestcaseTest_Basic26) {
//   TestTestcase("../RCompiler-Testcases/semantic-1/basic26/basic26.rx", true);
// }
//
// TEST(SemanticTest, TestcaseTest_Basic27) {
//   TestTestcase("../RCompiler-Testcases/semantic-1/basic27/basic27.rx", true);
// }
//
// TEST(SemanticTest, TestcaseTest_Basic28) {
//   TestTestcase("../RCompiler-Testcases/semantic-1/basic28/basic28.rx", false);
// }
//
// TEST(SemanticTest, TestcaseTest_Basic29) {
//   TestTestcase("../RCompiler-Testcases/semantic-1/basic29/basic29.rx", true);
// }
//
// TEST(SemanticTest, TestcaseTest_Basic30) {
//   TestTestcase("../RCompiler-Testcases/semantic-1/basic30/basic30.rx", false);
// }
//
// TEST(SemanticTest, TestcaseTest_Basic31) {
//   TestTestcase("../RCompiler-Testcases/semantic-1/basic31/basic31.rx", false);
// }
//
// TEST(SemanticTest, TestcaseTest_Basic32) {
//   TestTestcase("../RCompiler-Testcases/semantic-1/basic32/basic32.rx", false);
// }
//
// TEST(SemanticTest, TestcaseTest_Basic33) {
//   TestTestcase("../RCompiler-Testcases/semantic-1/basic33/basic33.rx", false);
// }
//
// TEST(SemanticTest, TestcaseTest_Basic34) {
//   TestTestcase("../RCompiler-Testcases/semantic-1/basic34/basic34.rx", false);
// }
//
// TEST(SemanticTest, TestcaseTest_Basic35) {
//   TestTestcase("../RCompiler-Testcases/semantic-1/basic35/basic35.rx", false);
// }
//
// TEST(SemanticTest, TestcaseTest_Basic36) {
//   TestTestcase("../RCompiler-Testcases/semantic-1/basic36/basic36.rx", true);
// }
//
// TEST(SemanticTest, TestcaseTest_Basic37) {
//   TestTestcase("../RCompiler-Testcases/semantic-1/basic37/basic37.rx", true);
// }
//
// TEST(SemanticTest, TestcaseTest_Basic38) {
//   TestTestcase("../RCompiler-Testcases/semantic-1/basic38/basic38.rx", false);
// }
//
// TEST(SemanticTest, TestcaseTest_Basic39) {
//   TestTestcase("../RCompiler-Testcases/semantic-1/basic39/basic39.rx", true);
// }
//
// TEST(SemanticTest, TestcaseTest_Basic40) {
//   TestTestcase("../RCompiler-Testcases/semantic-1/basic40/basic40.rx", true);
// }

TEST(SemanticTest, TestcaseTest_Type1) {
  TestTestcase("../RCompiler-Testcases/semantic-1/type1/type1.rx", false);
}

TEST(SemanticTest, TestcaseTest_Type2) {
  TestTestcase("../RCompiler-Testcases/semantic-1/type2/type2.rx", false);
}

TEST(SemanticTest, TestcaseTest_Type3) {
  TestTestcase("../RCompiler-Testcases/semantic-1/type3/type3.rx", false);
}

TEST(SemanticTest, TestcaseTest_Type4) {
  TestTestcase("../RCompiler-Testcases/semantic-1/type4/type4.rx", false);
}

TEST(SemanticTest, TestcaseTest_Type5) {
  TestTestcase("../RCompiler-Testcases/semantic-1/type5/type5.rx", false);
}

TEST(SemanticTest, TestcaseTest_Type6) {
  TestTestcase("../RCompiler-Testcases/semantic-1/type6/type6.rx", false);
}

// bad testcase
// TEST(SemanticTest, TestcaseTest_Type7) {
//   TestTestcase("../RCompiler-Testcases/semantic-1/type7/type7.rx", false);
// }

// bad testcase
// TEST(SemanticTest, TestcaseTest_Type8) {
//   TestTestcase("../RCompiler-Testcases/semantic-1/type8/type8.rx", false);
// }

TEST(SemanticTest, TestcaseTest_Type9) {
  TestTestcase("../RCompiler-Testcases/semantic-1/type9/type9.rx", false);
}

TEST(SemanticTest, TestcaseTest_Type10) {
  TestTestcase("../RCompiler-Testcases/semantic-1/type10/type10.rx", false);
}

TEST(SemanticTest, TestcaseTest_Type11) {
  TestTestcase("../RCompiler-Testcases/semantic-1/type11/type11.rx", false);
}

TEST(SemanticTest, TestcaseTest_Type12) {
  TestTestcase("../RCompiler-Testcases/semantic-1/type12/type12.rx", false);
}

// TEST(SemanticTest, TestcaseTest_Type13) {
//   TestTestcase("../RCompiler-Testcases/semantic-1/type13/type13.rx", false);
// }
//
// TEST(SemanticTest, TestcaseTest_Type14) {
//   TestTestcase("../RCompiler-Testcases/semantic-1/type14/type14.rx", false);
// }
//
// TEST(SemanticTest, TestcaseTest_Type15) {
//   TestTestcase("../RCompiler-Testcases/semantic-1/type15/type15.rx", false);
// }
//
// TEST(SemanticTest, TestcaseTest_Type16) {
//   TestTestcase("../RCompiler-Testcases/semantic-1/type16/type16.rx", false);
// }
//
// TEST(SemanticTest, TestcaseTest_Type17) {
//   TestTestcase("../RCompiler-Testcases/semantic-1/type17/type17.rx", false);
// }
//
// TEST(SemanticTest, TestcaseTest_Type18) {
//   TestTestcase("../RCompiler-Testcases/semantic-1/type18/type18.rx", false);
// }
//
// TEST(SemanticTest, TestcaseTest_Type19) {
//   TestTestcase("../RCompiler-Testcases/semantic-1/type19/type19.rx", false);
// }
//
// TEST(SemanticTest, TestcaseTest_Type20) {
//   TestTestcase("../RCompiler-Testcases/semantic-1/type20/type20.rx", false);
// }