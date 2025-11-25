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
  try {
    Lexer a(s);
    auto tokens = a.Run();
    Parser b(tokens);
    auto root = b.Run<CrateNode>();
    // Printer printer(std::cerr);
    // printer.Prepare();
    // printer.Visit(root.get());
    FirstChecker fc;
    fc.Run(root.get());
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

// TEST(SemanticTest, TmpTest) {
//   TestTestcase("../my_data/tmp.txt", true);
// }

TEST(SemanticTest, TestcaseTest_Array1) {
  TestTestcase("../RCompiler-Testcases/semantic-1/src/array1/array1.rx", true);
}

TEST(SemanticTest, TestcaseTest_Array2) {
  TestTestcase("../RCompiler-Testcases/semantic-1/src/array2/array2.rx", true);
}

TEST(SemanticTest, TestcaseTest_Array3) {
  TestTestcase("../RCompiler-Testcases/semantic-1/src/array3/array3.rx", true);
}

TEST(SemanticTest, TestcaseTest_Array4) {
  TestTestcase("../RCompiler-Testcases/semantic-1/src/array4/array4.rx", false);
}

TEST(SemanticTest, TestcaseTest_Array5) {
  TestTestcase("../RCompiler-Testcases/semantic-1/src/array5/array5.rx", false);
}

TEST(SemanticTest, TestcaseTest_Array6) {
  TestTestcase("../RCompiler-Testcases/semantic-1/src/array6/array6.rx", false);
}

TEST(SemanticTest, TestcaseTest_Array7) {
  TestTestcase("../RCompiler-Testcases/semantic-1/src/array7/array7.rx", false);
}

TEST(SemanticTest, TestcaseTest_Array8) {
  TestTestcase("../RCompiler-Testcases/semantic-1/src/array8/array8.rx", false);
}

TEST(SemanticTest, TestcaseTest_Autoref1) {
  TestTestcase("../RCompiler-Testcases/semantic-1/src/autoref1/autoref1.rx", true);
}

TEST(SemanticTest, TestcaseTest_Autoref2) {
  TestTestcase("../RCompiler-Testcases/semantic-1/src/autoref2/autoref2.rx", true);
}

TEST(SemanticTest, TestcaseTest_Autoref3) {
  TestTestcase("../RCompiler-Testcases/semantic-1/src/autoref3/autoref3.rx", true);
}

TEST(SemanticTest, TestcaseTest_Autoref4) {
  TestTestcase("../RCompiler-Testcases/semantic-1/src/autoref4/autoref4.rx", true);
}

TEST(SemanticTest, TestcaseTest_Autoref5) {
  TestTestcase("../RCompiler-Testcases/semantic-1/src/autoref5/autoref5.rx", true);
}

TEST(SemanticTest, TestcaseTest_Autoref6) {
  TestTestcase("../RCompiler-Testcases/semantic-1/src/autoref6/autoref6.rx", true);
}

TEST(SemanticTest, TestcaseTest_Autoref7) {
  TestTestcase("../RCompiler-Testcases/semantic-1/src/autoref7/autoref7.rx", false);
}

TEST(SemanticTest, TestcaseTest_Autoref8) {
  TestTestcase("../RCompiler-Testcases/semantic-1/src/autoref8/autoref8.rx", false);
}

TEST(SemanticTest, TestcaseTest_Autoref9) {
  TestTestcase("../RCompiler-Testcases/semantic-1/src/autoref9/autoref9.rx", true);
}

TEST(SemanticTest, TestcaseTest_Basic1) {
  TestTestcase("../RCompiler-Testcases/semantic-1/src/basic1/basic1.rx", true);
}

TEST(SemanticTest, TestcaseTest_Basic2) {
  TestTestcase("../RCompiler-Testcases/semantic-1/src/basic2/basic2.rx", false);
}

TEST(SemanticTest, TestcaseTest_Basic3) {
  TestTestcase("../RCompiler-Testcases/semantic-1/src/basic3/basic3.rx", false);
}

TEST(SemanticTest, TestcaseTest_Basic4) {
  TestTestcase("../RCompiler-Testcases/semantic-1/src/basic4/basic4.rx", true);
}

TEST(SemanticTest, TestcaseTest_Basic5) {
  TestTestcase("../RCompiler-Testcases/semantic-1/src/basic5/basic5.rx", false);
}

TEST(SemanticTest, TestcaseTest_Basic6) {
  TestTestcase("../RCompiler-Testcases/semantic-1/src/basic6/basic6.rx", false);
}

TEST(SemanticTest, TestcaseTest_Basic7) {
  TestTestcase("../RCompiler-Testcases/semantic-1/src/basic7/basic7.rx", false);
}

TEST(SemanticTest, TestcaseTest_Basic8) {
  TestTestcase("../RCompiler-Testcases/semantic-1/src/basic8/basic8.rx", false);
}

TEST(SemanticTest, TestcaseTest_Basic9) {
  TestTestcase("../RCompiler-Testcases/semantic-1/src/basic9/basic9.rx", false);
}

TEST(SemanticTest, TestcaseTest_Basic10) {
  TestTestcase("../RCompiler-Testcases/semantic-1/src/basic10/basic10.rx", false);
}

TEST(SemanticTest, TestcaseTest_Basic11) {
  TestTestcase("../RCompiler-Testcases/semantic-1/src/basic11/basic11.rx", true);
}

TEST(SemanticTest, TestcaseTest_Basic12) {
  TestTestcase("../RCompiler-Testcases/semantic-1/src/basic12/basic12.rx", false);
}

TEST(SemanticTest, TestcaseTest_Basic13) {
  TestTestcase("../RCompiler-Testcases/semantic-1/src/basic13/basic13.rx", false);
}

TEST(SemanticTest, TestcaseTest_Basic14) {
  TestTestcase("../RCompiler-Testcases/semantic-1/src/basic14/basic14.rx", false);
}

TEST(SemanticTest, TestcaseTest_Basic15) {
  TestTestcase("../RCompiler-Testcases/semantic-1/src/basic15/basic15.rx", false);
}

TEST(SemanticTest, TestcaseTest_Basic16) {
  TestTestcase("../RCompiler-Testcases/semantic-1/src/basic16/basic16.rx", false);
}

TEST(SemanticTest, TestcaseTest_Basic17) {
  TestTestcase("../RCompiler-Testcases/semantic-1/src/basic17/basic17.rx", true);
}

TEST(SemanticTest, TestcaseTest_Basic18) {
  TestTestcase("../RCompiler-Testcases/semantic-1/src/basic18/basic18.rx", true);
}

TEST(SemanticTest, TestcaseTest_Basic19) {
  TestTestcase("../RCompiler-Testcases/semantic-1/src/basic19/basic19.rx", true);
}

TEST(SemanticTest, TestcaseTest_Basic20) {
  TestTestcase("../RCompiler-Testcases/semantic-1/src/basic20/basic20.rx", true);
}

TEST(SemanticTest, TestcaseTest_Basic21) {
  TestTestcase("../RCompiler-Testcases/semantic-1/src/basic21/basic21.rx", true);
}

TEST(SemanticTest, TestcaseTest_Basic22) {
  TestTestcase("../RCompiler-Testcases/semantic-1/src/basic22/basic22.rx", true);
}

TEST(SemanticTest, TestcaseTest_Basic23) {
  TestTestcase("../RCompiler-Testcases/semantic-1/src/basic23/basic23.rx", true);
}

TEST(SemanticTest, TestcaseTest_Basic24) {
  TestTestcase("../RCompiler-Testcases/semantic-1/src/basic24/basic24.rx", true);
}

TEST(SemanticTest, TestcaseTest_Basic25) {
  TestTestcase("../RCompiler-Testcases/semantic-1/src/basic25/basic25.rx", true);
}

TEST(SemanticTest, TestcaseTest_Basic26) {
  TestTestcase("../RCompiler-Testcases/semantic-1/src/basic26/basic26.rx", true);
}

TEST(SemanticTest, TestcaseTest_Basic27) {
  TestTestcase("../RCompiler-Testcases/semantic-1/src/basic27/basic27.rx", true);
}

TEST(SemanticTest, TestcaseTest_Basic28) {
  TestTestcase("../RCompiler-Testcases/semantic-1/src/basic28/basic28.rx", false);
}

TEST(SemanticTest, TestcaseTest_Basic29) {
  TestTestcase("../RCompiler-Testcases/semantic-1/src/basic29/basic29.rx", true);
}

TEST(SemanticTest, TestcaseTest_Basic30) {
  TestTestcase("../RCompiler-Testcases/semantic-1/src/basic30/basic30.rx", false);
}

TEST(SemanticTest, TestcaseTest_Basic31) {
  TestTestcase("../RCompiler-Testcases/semantic-1/src/basic31/basic31.rx", false);
}

TEST(SemanticTest, TestcaseTest_Basic32) {
  TestTestcase("../RCompiler-Testcases/semantic-1/src/basic32/basic32.rx", false);
}

TEST(SemanticTest, TestcaseTest_Basic33) {
  TestTestcase("../RCompiler-Testcases/semantic-1/src/basic33/basic33.rx", false);
}

TEST(SemanticTest, TestcaseTest_Basic34) {
  TestTestcase("../RCompiler-Testcases/semantic-1/src/basic34/basic34.rx", false);
}

TEST(SemanticTest, TestcaseTest_Basic35) {
  TestTestcase("../RCompiler-Testcases/semantic-1/src/basic35/basic35.rx", false);
}

TEST(SemanticTest, TestcaseTest_Basic36) {
  TestTestcase("../RCompiler-Testcases/semantic-1/src/basic36/basic36.rx", true);
}

TEST(SemanticTest, TestcaseTest_Basic37) {
  TestTestcase("../RCompiler-Testcases/semantic-1/src/basic37/basic37.rx", true);
}

TEST(SemanticTest, TestcaseTest_Basic38) {
  TestTestcase("../RCompiler-Testcases/semantic-1/src/basic38/basic38.rx", false);
}

TEST(SemanticTest, TestcaseTest_Basic39) {
  TestTestcase("../RCompiler-Testcases/semantic-1/src/basic39/basic39.rx", true);
}

TEST(SemanticTest, TestcaseTest_Basic40) {
  TestTestcase("../RCompiler-Testcases/semantic-1/src/basic40/basic40.rx", true);
}

TEST(SemanticTest, TestcaseTest_Expr1) {
  TestTestcase("../RCompiler-Testcases/semantic-1/src/expr1/expr1.rx", false);
}

TEST(SemanticTest, TestcaseTest_Expr2) {
  TestTestcase("../RCompiler-Testcases/semantic-1/src/expr2/expr2.rx", false);
}

TEST(SemanticTest, TestcaseTest_Expr3) {
  TestTestcase("../RCompiler-Testcases/semantic-1/src/expr3/expr3.rx", false);
}

TEST(SemanticTest, TestcaseTest_Expr4) {
  TestTestcase("../RCompiler-Testcases/semantic-1/src/expr4/expr4.rx", false);
}

TEST(SemanticTest, TestcaseTest_Expr5) {
  TestTestcase("../RCompiler-Testcases/semantic-1/src/expr5/expr5.rx", false);
}

TEST(SemanticTest, TestcaseTest_Expr6) {
  TestTestcase("../RCompiler-Testcases/semantic-1/src/expr6/expr6.rx", false);
}

TEST(SemanticTest, TestcaseTest_Expr7) {
  TestTestcase("../RCompiler-Testcases/semantic-1/src/expr7/expr7.rx", false);
}

TEST(SemanticTest, TestcaseTest_Expr8) {
  TestTestcase("../RCompiler-Testcases/semantic-1/src/expr8/expr8.rx", false);
}

TEST(SemanticTest, TestcaseTest_Expr9) {
  TestTestcase("../RCompiler-Testcases/semantic-1/src/expr9/expr9.rx", false);
}

TEST(SemanticTest, TestcaseTest_Expr10) {
  TestTestcase("../RCompiler-Testcases/semantic-1/src/expr10/expr10.rx", false);
}

TEST(SemanticTest, TestcaseTest_Expr11) {
  TestTestcase("../RCompiler-Testcases/semantic-1/src/expr11/expr11.rx", false);
}

TEST(SemanticTest, TestcaseTest_Expr12) {
  TestTestcase("../RCompiler-Testcases/semantic-1/src/expr12/expr12.rx", false);
}

TEST(SemanticTest, TestcaseTest_Expr13) {
  TestTestcase("../RCompiler-Testcases/semantic-1/src/expr13/expr13.rx", true);
}

TEST(SemanticTest, TestcaseTest_Expr14) {
  TestTestcase("../RCompiler-Testcases/semantic-1/src/expr14/expr14.rx", false);
}

TEST(SemanticTest, TestcaseTest_Expr15) {
  TestTestcase("../RCompiler-Testcases/semantic-1/src/expr15/expr15.rx", false);
}

TEST(SemanticTest, TestcaseTest_Expr16) {
  TestTestcase("../RCompiler-Testcases/semantic-1/src/expr16/expr16.rx", false);
}

TEST(SemanticTest, TestcaseTest_Expr17) {
  TestTestcase("../RCompiler-Testcases/semantic-1/src/expr17/expr17.rx", true);
}

TEST(SemanticTest, TestcaseTest_Expr18) {
  TestTestcase("../RCompiler-Testcases/semantic-1/src/expr18/expr18.rx", false);
}

TEST(SemanticTest, TestcaseTest_Expr19) {
  TestTestcase("../RCompiler-Testcases/semantic-1/src/expr19/expr19.rx", true);
}

TEST(SemanticTest, TestcaseTest_Expr20) {
  TestTestcase("../RCompiler-Testcases/semantic-1/src/expr20/expr20.rx", false);
}

TEST(SemanticTest, TestcaseTest_Expr21) {
  TestTestcase("../RCompiler-Testcases/semantic-1/src/expr21/expr21.rx", false);
}

TEST(SemanticTest, TestcaseTest_Expr22) {
  TestTestcase("../RCompiler-Testcases/semantic-1/src/expr22/expr22.rx", false);
}

TEST(SemanticTest, TestcaseTest_Expr23) {
  TestTestcase("../RCompiler-Testcases/semantic-1/src/expr23/expr23.rx", false);
}

TEST(SemanticTest, TestcaseTest_Expr24) {
  TestTestcase("../RCompiler-Testcases/semantic-1/src/expr24/expr24.rx", false);
}

TEST(SemanticTest, TestcaseTest_Expr25) {
  TestTestcase("../RCompiler-Testcases/semantic-1/src/expr25/expr25.rx", false);
}

TEST(SemanticTest, TestcaseTest_Expr26) {
  TestTestcase("../RCompiler-Testcases/semantic-1/src/expr26/expr26.rx", false);
}

TEST(SemanticTest, TestcaseTest_Expr27) {
  TestTestcase("../RCompiler-Testcases/semantic-1/src/expr27/expr27.rx", false);
}

TEST(SemanticTest, TestcaseTest_Expr28) {
  TestTestcase("../RCompiler-Testcases/semantic-1/src/expr28/expr28.rx", false);
}

TEST(SemanticTest, TestcaseTest_Expr29) {
  TestTestcase("../RCompiler-Testcases/semantic-1/src/expr29/expr29.rx", false);
}

TEST(SemanticTest, TestcaseTest_Expr30) {
  TestTestcase("../RCompiler-Testcases/semantic-1/src/expr30/expr30.rx", false);
}

TEST(SemanticTest, TestcaseTest_Expr31) {
  TestTestcase("../RCompiler-Testcases/semantic-1/src/expr31/expr31.rx", false);
}

TEST(SemanticTest, TestcaseTest_Expr32) {
  TestTestcase("../RCompiler-Testcases/semantic-1/src/expr32/expr32.rx", false);
}

TEST(SemanticTest, TestcaseTest_Expr33) {
  TestTestcase("../RCompiler-Testcases/semantic-1/src/expr33/expr33.rx", true);
}

TEST(SemanticTest, TestcaseTest_Expr34) {
  TestTestcase("../RCompiler-Testcases/semantic-1/src/expr34/expr34.rx", true);
}

TEST(SemanticTest, TestcaseTest_Expr35) {
  TestTestcase("../RCompiler-Testcases/semantic-1/src/expr35/expr35.rx", false);
}

TEST(SemanticTest, TestcaseTest_Expr36) {
  TestTestcase("../RCompiler-Testcases/semantic-1/src/expr36/expr36.rx", true);
}

TEST(SemanticTest, TestcaseTest_Expr37) {
  TestTestcase("../RCompiler-Testcases/semantic-1/src/expr37/expr37.rx", false);
}

TEST(SemanticTest, TestcaseTest_Expr38) {
  TestTestcase("../RCompiler-Testcases/semantic-1/src/expr38/expr38.rx", true);
}

TEST(SemanticTest, TestcaseTest_Expr39) {
  TestTestcase("../RCompiler-Testcases/semantic-1/src/expr39/expr39.rx", false);
}

TEST(SemanticTest, TestcaseTest_Expr40) {
  TestTestcase("../RCompiler-Testcases/semantic-1/src/expr40/expr40.rx", false);
}

TEST(SemanticTest, TestcaseTest_If1) {
  TestTestcase("../RCompiler-Testcases/semantic-1/src/if1/if1.rx", true);
}

TEST(SemanticTest, TestcaseTest_If2) {
  TestTestcase("../RCompiler-Testcases/semantic-1/src/if2/if2.rx", true);
}

TEST(SemanticTest, TestcaseTest_If3) {
  TestTestcase("../RCompiler-Testcases/semantic-1/src/if3/if3.rx", true);
}

TEST(SemanticTest, TestcaseTest_If4) {
  TestTestcase("../RCompiler-Testcases/semantic-1/src/if4/if4.rx", true);
}

TEST(SemanticTest, TestcaseTest_If5) {
  TestTestcase("../RCompiler-Testcases/semantic-1/src/if5/if5.rx", true);
}

TEST(SemanticTest, TestcaseTest_If6) {
  TestTestcase("../RCompiler-Testcases/semantic-1/src/if6/if6.rx", true);
}

TEST(SemanticTest, TestcaseTest_If7) {
  TestTestcase("../RCompiler-Testcases/semantic-1/src/if7/if7.rx", true);
}

TEST(SemanticTest, TestcaseTest_If8) {
  TestTestcase("../RCompiler-Testcases/semantic-1/src/if8/if8.rx", true);
}

TEST(SemanticTest, TestcaseTest_If9) {
  TestTestcase("../RCompiler-Testcases/semantic-1/src/if9/if9.rx", true);
}

TEST(SemanticTest, TestcaseTest_If10) {
  TestTestcase("../RCompiler-Testcases/semantic-1/src/if10/if10.rx", true);
}

TEST(SemanticTest, TestcaseTest_If11) {
  TestTestcase("../RCompiler-Testcases/semantic-1/src/if11/if11.rx", false);
}

TEST(SemanticTest, TestcaseTest_If12) {
  TestTestcase("../RCompiler-Testcases/semantic-1/src/if12/if12.rx", false);
}

TEST(SemanticTest, TestcaseTest_If13) {
  TestTestcase("../RCompiler-Testcases/semantic-1/src/if13/if13.rx", false);
}

TEST(SemanticTest, TestcaseTest_If14) {
  TestTestcase("../RCompiler-Testcases/semantic-1/src/if14/if14.rx", false);
}

TEST(SemanticTest, TestcaseTest_If15) {
  TestTestcase("../RCompiler-Testcases/semantic-1/src/if15/if15.rx", false);
}

TEST(SemanticTest, TestcaseTest_Loop1) {
  TestTestcase("../RCompiler-Testcases/semantic-1/src/loop1/loop1.rx", true);
}

TEST(SemanticTest, TestcaseTest_Loop2) {
  TestTestcase("../RCompiler-Testcases/semantic-1/src/loop2/loop2.rx", true);
}

TEST(SemanticTest, TestcaseTest_Loop3) {
  TestTestcase("../RCompiler-Testcases/semantic-1/src/loop3/loop3.rx", true);
}

// inactive
// TEST(SemanticTest, TestcaseTest_Loop4) {
//   TestTestcase("../RCompiler-Testcases/semantic-1/src/loop4/loop4.rx", true);
// }

TEST(SemanticTest, TestcaseTest_Loop5) {
  TestTestcase("../RCompiler-Testcases/semantic-1/src/loop5/loop5.rx", true);
}

TEST(SemanticTest, TestcaseTest_Loop6) {
  TestTestcase("../RCompiler-Testcases/semantic-1/src/loop6/loop6.rx", false);
}

TEST(SemanticTest, TestcaseTest_Loop7) {
  TestTestcase("../RCompiler-Testcases/semantic-1/src/loop7/loop7.rx", false);
}

TEST(SemanticTest, TestcaseTest_Loop8) {
  TestTestcase("../RCompiler-Testcases/semantic-1/src/loop8/loop8.rx", false);
}

TEST(SemanticTest, TestcaseTest_Loop9) {
  TestTestcase("../RCompiler-Testcases/semantic-1/src/loop9/loop9.rx", false);
}

TEST(SemanticTest, TestcaseTest_Loop10) {
  TestTestcase("../RCompiler-Testcases/semantic-1/src/loop10/loop10.rx", false);
}

TEST(SemanticTest, TestcaseTest_Misc1) {
  TestTestcase("../RCompiler-Testcases/semantic-1/src/misc1/misc1.rx", true);
}

TEST(SemanticTest, TestcaseTest_Misc2) {
  TestTestcase("../RCompiler-Testcases/semantic-1/src/misc2/misc2.rx", true);
}

TEST(SemanticTest, TestcaseTest_Misc3) {
  TestTestcase("../RCompiler-Testcases/semantic-1/src/misc3/misc3.rx", true);
}

TEST(SemanticTest, TestcaseTest_Misc4) {
  TestTestcase("../RCompiler-Testcases/semantic-1/src/misc4/misc4.rx", true);
}

TEST(SemanticTest, TestcaseTest_Misc5) {
  TestTestcase("../RCompiler-Testcases/semantic-1/src/misc5/misc5.rx", true);
}

TEST(SemanticTest, TestcaseTest_Misc6) {
  TestTestcase("../RCompiler-Testcases/semantic-1/src/misc6/misc6.rx", true);
}

TEST(SemanticTest, TestcaseTest_Misc7) {
  TestTestcase("../RCompiler-Testcases/semantic-1/src/misc7/misc7.rx", true);
}

TEST(SemanticTest, TestcaseTest_Misc8) {
  TestTestcase("../RCompiler-Testcases/semantic-1/src/misc8/misc8.rx", true);
}

TEST(SemanticTest, TestcaseTest_Misc9) {
  TestTestcase("../RCompiler-Testcases/semantic-1/src/misc9/misc9.rx", true);
}

TEST(SemanticTest, TestcaseTest_Misc10) {
  TestTestcase("../RCompiler-Testcases/semantic-1/src/misc10/misc10.rx", true);
}

TEST(SemanticTest, TestcaseTest_Misc11) {
  TestTestcase("../RCompiler-Testcases/semantic-1/src/misc11/misc11.rx", true);
}

TEST(SemanticTest, TestcaseTest_Misc12) {
  TestTestcase("../RCompiler-Testcases/semantic-1/src/misc12/misc12.rx", true);
}

TEST(SemanticTest, TestcaseTest_Misc13) {
  TestTestcase("../RCompiler-Testcases/semantic-1/src/misc13/misc13.rx", true);
}

TEST(SemanticTest, TestcaseTest_Misc14) {
  TestTestcase("../RCompiler-Testcases/semantic-1/src/misc14/misc14.rx", true);
}

TEST(SemanticTest, TestcaseTest_Misc15) {
  TestTestcase("../RCompiler-Testcases/semantic-1/src/misc15/misc15.rx", true);
}

TEST(SemanticTest, TestcaseTest_Misc16) {
  TestTestcase("../RCompiler-Testcases/semantic-1/src/misc16/misc16.rx", true);
}

TEST(SemanticTest, TestcaseTest_Misc17) {
  TestTestcase("../RCompiler-Testcases/semantic-1/src/misc17/misc17.rx", true);
}

TEST(SemanticTest, TestcaseTest_Misc18) {
  TestTestcase("../RCompiler-Testcases/semantic-1/src/misc18/misc18.rx", true);
}

TEST(SemanticTest, TestcaseTest_Misc19) {
  TestTestcase("../RCompiler-Testcases/semantic-1/src/misc19/misc19.rx", true);
}

TEST(SemanticTest, TestcaseTest_Misc20) {
  TestTestcase("../RCompiler-Testcases/semantic-1/src/misc20/misc20.rx", true);
}

TEST(SemanticTest, TestcaseTest_Misc21) {
  TestTestcase("../RCompiler-Testcases/semantic-1/src/misc21/misc21.rx", true);
}

TEST(SemanticTest, TestcaseTest_Misc22) {
  TestTestcase("../RCompiler-Testcases/semantic-1/src/misc22/misc22.rx", true);
}

TEST(SemanticTest, TestcaseTest_Misc23) {
  TestTestcase("../RCompiler-Testcases/semantic-1/src/misc23/misc23.rx", true);
}

TEST(SemanticTest, TestcaseTest_Misc24) {
  TestTestcase("../RCompiler-Testcases/semantic-1/src/misc24/misc24.rx", true);
}

TEST(SemanticTest, TestcaseTest_Misc25) {
  TestTestcase("../RCompiler-Testcases/semantic-1/src/misc25/misc25.rx", true);
}

TEST(SemanticTest, TestcaseTest_Misc26) {
  TestTestcase("../RCompiler-Testcases/semantic-1/src/misc26/misc26.rx", true);
}

TEST(SemanticTest, TestcaseTest_Misc27) {
  TestTestcase("../RCompiler-Testcases/semantic-1/src/misc27/misc27.rx", true);
}

TEST(SemanticTest, TestcaseTest_Misc28) {
  TestTestcase("../RCompiler-Testcases/semantic-1/src/misc28/misc28.rx", true);
}

TEST(SemanticTest, TestcaseTest_Misc29) {
  TestTestcase("../RCompiler-Testcases/semantic-1/src/misc29/misc29.rx", true);
}

TEST(SemanticTest, TestcaseTest_Misc30) {
  TestTestcase("../RCompiler-Testcases/semantic-1/src/misc30/misc30.rx", true);
}

TEST(SemanticTest, TestcaseTest_Misc31) {
  TestTestcase("../RCompiler-Testcases/semantic-1/src/misc31/misc31.rx", true);
}

TEST(SemanticTest, TestcaseTest_Misc32) {
  TestTestcase("../RCompiler-Testcases/semantic-1/src/misc32/misc32.rx", true);
}

TEST(SemanticTest, TestcaseTest_Misc33) {
  TestTestcase("../RCompiler-Testcases/semantic-1/src/misc33/misc33.rx", true);
}

TEST(SemanticTest, TestcaseTest_Misc34) {
  TestTestcase("../RCompiler-Testcases/semantic-1/src/misc34/misc34.rx", true);
}

TEST(SemanticTest, TestcaseTest_Misc35) {
  TestTestcase("../RCompiler-Testcases/semantic-1/src/misc35/misc35.rx", true);
}

TEST(SemanticTest, TestcaseTest_Misc36) {
  TestTestcase("../RCompiler-Testcases/semantic-1/src/misc36/misc36.rx", true);
}

TEST(SemanticTest, TestcaseTest_Misc37) {
  TestTestcase("../RCompiler-Testcases/semantic-1/src/misc37/misc37.rx", true);
}

TEST(SemanticTest, TestcaseTest_Misc38) {
  TestTestcase("../RCompiler-Testcases/semantic-1/src/misc38/misc38.rx", true);
}

TEST(SemanticTest, TestcaseTest_Misc39) {
  TestTestcase("../RCompiler-Testcases/semantic-1/src/misc39/misc39.rx", true);
}

TEST(SemanticTest, TestcaseTest_Misc40) {
  TestTestcase("../RCompiler-Testcases/semantic-1/src/misc40/misc40.rx", true);
}

TEST(SemanticTest, TestcaseTest_Misc41) {
  TestTestcase("../RCompiler-Testcases/semantic-1/src/misc41/misc41.rx", true);
}

TEST(SemanticTest, TestcaseTest_Misc42) {
  TestTestcase("../RCompiler-Testcases/semantic-1/src/misc42/misc42.rx", true);
}

TEST(SemanticTest, TestcaseTest_Misc43) {
  TestTestcase("../RCompiler-Testcases/semantic-1/src/misc43/misc43.rx", true);
}

TEST(SemanticTest, TestcaseTest_Misc44) {
  TestTestcase("../RCompiler-Testcases/semantic-1/src/misc44/misc44.rx", true);
}

TEST(SemanticTest, TestcaseTest_Misc45) {
  TestTestcase("../RCompiler-Testcases/semantic-1/src/misc45/misc45.rx", true);
}

TEST(SemanticTest, TestcaseTest_Misc46) {
  TestTestcase("../RCompiler-Testcases/semantic-1/src/misc46/misc46.rx", true);
}

TEST(SemanticTest, TestcaseTest_Misc47) {
  TestTestcase("../RCompiler-Testcases/semantic-1/src/misc47/misc47.rx", true);
}

TEST(SemanticTest, TestcaseTest_Misc48) {
  TestTestcase("../RCompiler-Testcases/semantic-1/src/misc48/misc48.rx", true);
}

TEST(SemanticTest, TestcaseTest_Misc49) {
  TestTestcase("../RCompiler-Testcases/semantic-1/src/misc49/misc49.rx", true);
}

TEST(SemanticTest, TestcaseTest_Misc50) {
  TestTestcase("../RCompiler-Testcases/semantic-1/src/misc50/misc50.rx", true);
}

TEST(SemanticTest, TestcaseTest_Misc51) {
  TestTestcase("../RCompiler-Testcases/semantic-1/src/misc51/misc51.rx", true);
}

TEST(SemanticTest, TestcaseTest_Misc52) {
  TestTestcase("../RCompiler-Testcases/semantic-1/src/misc52/misc52.rx", true);
}

TEST(SemanticTest, TestcaseTest_Misc53) {
  TestTestcase("../RCompiler-Testcases/semantic-1/src/misc53/misc53.rx", true);
}

TEST(SemanticTest, TestcaseTest_Misc54) {
  TestTestcase("../RCompiler-Testcases/semantic-1/src/misc54/misc54.rx", true);
}

TEST(SemanticTest, TestcaseTest_Misc55) {
  TestTestcase("../RCompiler-Testcases/semantic-1/src/misc55/misc55.rx", true);
}

TEST(SemanticTest, TestcaseTest_Misc56) {
  TestTestcase("../RCompiler-Testcases/semantic-1/src/misc56/misc56.rx", false);
}

TEST(SemanticTest, TestcaseTest_Misc57) {
  TestTestcase("../RCompiler-Testcases/semantic-1/src/misc57/misc57.rx", false);
}

TEST(SemanticTest, TestcaseTest_Misc58) {
  TestTestcase("../RCompiler-Testcases/semantic-1/src/misc58/misc58.rx", false);
}

TEST(SemanticTest, TestcaseTest_Misc59) {
  TestTestcase("../RCompiler-Testcases/semantic-1/src/misc59/misc59.rx", false);
}

TEST(SemanticTest, TestcaseTest_Misc60) {
  TestTestcase("../RCompiler-Testcases/semantic-1/src/misc60/misc60.rx", false);
}

TEST(SemanticTest, TestcaseTest_Misc61) {
  TestTestcase("../RCompiler-Testcases/semantic-1/src/misc61/misc61.rx", false);
}

TEST(SemanticTest, TestcaseTest_Misc62) {
  TestTestcase("../RCompiler-Testcases/semantic-1/src/misc62/misc62.rx", false);
}

TEST(SemanticTest, TestcaseTest_Misc63) {
  TestTestcase("../RCompiler-Testcases/semantic-1/src/misc63/misc63.rx", false);
}

TEST(SemanticTest, TestcaseTest_Misc64) {
  TestTestcase("../RCompiler-Testcases/semantic-1/src/misc64/misc64.rx", false);
}

TEST(SemanticTest, TestcaseTest_Misc65) {
  TestTestcase("../RCompiler-Testcases/semantic-1/src/misc65/misc65.rx", true);
}

TEST(SemanticTest, TestcaseTest_Return1) {
  TestTestcase("../RCompiler-Testcases/semantic-1/src/return1/return1.rx", false);
}

TEST(SemanticTest, TestcaseTest_Return2) {
  TestTestcase("../RCompiler-Testcases/semantic-1/src/return2/return2.rx", true);
}

TEST(SemanticTest, TestcaseTest_Return3) {
  TestTestcase("../RCompiler-Testcases/semantic-1/src/return3/return3.rx", false);
}

TEST(SemanticTest, TestcaseTest_Return4) {
  TestTestcase("../RCompiler-Testcases/semantic-1/src/return4/return4.rx", false);
}

TEST(SemanticTest, TestcaseTest_Return5) {
  TestTestcase("../RCompiler-Testcases/semantic-1/src/return5/return5.rx", false);
}

TEST(SemanticTest, TestcaseTest_Return6) {
  TestTestcase("../RCompiler-Testcases/semantic-1/src/return6/return6.rx", false);
}

TEST(SemanticTest, TestcaseTest_Return7) {
  TestTestcase("../RCompiler-Testcases/semantic-1/src/return7/return7.rx", false);
}

TEST(SemanticTest, TestcaseTest_Return8) {
  TestTestcase("../RCompiler-Testcases/semantic-1/src/return8/return8.rx", true);
}

TEST(SemanticTest, TestcaseTest_Return9) {
  TestTestcase("../RCompiler-Testcases/semantic-1/src/return9/return9.rx", false);
}

TEST(SemanticTest, TestcaseTest_Return10) {
  TestTestcase("../RCompiler-Testcases/semantic-1/src/return10/return10.rx", false);
}

TEST(SemanticTest, TestcaseTest_Return11) {
  TestTestcase("../RCompiler-Testcases/semantic-1/src/return11/return11.rx", false);
}

TEST(SemanticTest, TestcaseTest_Return12) {
  TestTestcase("../RCompiler-Testcases/semantic-1/src/return12/return12.rx", true);
}

TEST(SemanticTest, TestcaseTest_Return13) {
  TestTestcase("../RCompiler-Testcases/semantic-1/src/return13/return13.rx", true);
}

TEST(SemanticTest, TestcaseTest_Return14) {
  TestTestcase("../RCompiler-Testcases/semantic-1/src/return14/return14.rx", true);
}

TEST(SemanticTest, TestcaseTest_Return15) {
  TestTestcase("../RCompiler-Testcases/semantic-1/src/return15/return15.rx", true);
}

TEST(SemanticTest, TestcaseTest_Type1) {
  TestTestcase("../RCompiler-Testcases/semantic-1/src/type1/type1.rx", false);
}

TEST(SemanticTest, TestcaseTest_Type2) {
  TestTestcase("../RCompiler-Testcases/semantic-1/src/type2/type2.rx", false);
}

TEST(SemanticTest, TestcaseTest_Type3) {
  TestTestcase("../RCompiler-Testcases/semantic-1/src/type3/type3.rx", false);
}

TEST(SemanticTest, TestcaseTest_Type4) {
  TestTestcase("../RCompiler-Testcases/semantic-1/src/type4/type4.rx", false);
}

TEST(SemanticTest, TestcaseTest_Type5) {
  TestTestcase("../RCompiler-Testcases/semantic-1/src/type5/type5.rx", false);
}

TEST(SemanticTest, TestcaseTest_Type6) {
  TestTestcase("../RCompiler-Testcases/semantic-1/src/type6/type6.rx", false);
}

TEST(SemanticTest, TestcaseTest_Type7) {
  TestTestcase("../RCompiler-Testcases/semantic-1/src/type7/type7.rx", false);
}

TEST(SemanticTest, TestcaseTest_Type8) {
  TestTestcase("../RCompiler-Testcases/semantic-1/src/type8/type8.rx", false);
}

TEST(SemanticTest, TestcaseTest_Type9) {
  TestTestcase("../RCompiler-Testcases/semantic-1/src/type9/type9.rx", false);
}

TEST(SemanticTest, TestcaseTest_Type10) {
  TestTestcase("../RCompiler-Testcases/semantic-1/src/type10/type10.rx", false);
}

TEST(SemanticTest, TestcaseTest_Type11) {
  TestTestcase("../RCompiler-Testcases/semantic-1/src/type11/type11.rx", false);
}

TEST(SemanticTest, TestcaseTest_Type12) {
  TestTestcase("../RCompiler-Testcases/semantic-1/src/type12/type12.rx", false);
}

TEST(SemanticTest, TestcaseTest_Type13) {
  TestTestcase("../RCompiler-Testcases/semantic-1/src/type13/type13.rx", false);
}

TEST(SemanticTest, TestcaseTest_Type14) {
  TestTestcase("../RCompiler-Testcases/semantic-1/src/type14/type14.rx", false);
}

TEST(SemanticTest, TestcaseTest_Type15) {
  TestTestcase("../RCompiler-Testcases/semantic-1/src/type15/type15.rx", false);
}

TEST(SemanticTest, TestcaseTest_Type16) {
  TestTestcase("../RCompiler-Testcases/semantic-1/src/type16/type16.rx", false);
}

TEST(SemanticTest, TestcaseTest_Type17) {
  TestTestcase("../RCompiler-Testcases/semantic-1/src/type17/type17.rx", false);
}

TEST(SemanticTest, TestcaseTest_Type18) {
  TestTestcase("../RCompiler-Testcases/semantic-1/src/type18/type18.rx", false);
}

TEST(SemanticTest, TestcaseTest_Type19) {
  TestTestcase("../RCompiler-Testcases/semantic-1/src/type19/type19.rx", false);
}

TEST(SemanticTest, TestcaseTest_Type20) {
  TestTestcase("../RCompiler-Testcases/semantic-1/src/type20/type20.rx", false);
}

TEST(SemanticTest, TestcaseTest_Comprehensive1) {
  TestTestcase("../RCompiler-Testcases/semantic-2/src/comprehensive1/comprehensive1.rx", true);
}

TEST(SemanticTest, TestcaseTest_Comprehensive2) {
  TestTestcase("../RCompiler-Testcases/semantic-2/src/comprehensive2/comprehensive2.rx", true);
}

TEST(SemanticTest, TestcaseTest_Comprehensive3) {
  TestTestcase("../RCompiler-Testcases/semantic-2/src/comprehensive3/comprehensive3.rx", true);
}

TEST(SemanticTest, TestcaseTest_Comprehensive4) {
  TestTestcase("../RCompiler-Testcases/semantic-2/src/comprehensive4/comprehensive4.rx", true);
}

TEST(SemanticTest, TestcaseTest_Comprehensive5) {
  TestTestcase("../RCompiler-Testcases/semantic-2/src/comprehensive5/comprehensive5.rx", true);
}

TEST(SemanticTest, TestcaseTest_Comprehensive6) {
  TestTestcase("../RCompiler-Testcases/semantic-2/src/comprehensive6/comprehensive6.rx", true);
}

TEST(SemanticTest, TestcaseTest_Comprehensive7) {
  TestTestcase("../RCompiler-Testcases/semantic-2/src/comprehensive7/comprehensive7.rx", true);
}

TEST(SemanticTest, TestcaseTest_Comprehensive8) {
  TestTestcase("../RCompiler-Testcases/semantic-2/src/comprehensive8/comprehensive8.rx", true);
}

TEST(SemanticTest, TestcaseTest_Comprehensive9) {
  TestTestcase("../RCompiler-Testcases/semantic-2/src/comprehensive9/comprehensive9.rx", true);
}

TEST(SemanticTest, TestcaseTest_Comprehensive10) {
  TestTestcase("../RCompiler-Testcases/semantic-2/src/comprehensive10/comprehensive10.rx", true);
}

TEST(SemanticTest, TestcaseTest_Comprehensive11) {
  TestTestcase("../RCompiler-Testcases/semantic-2/src/comprehensive11/comprehensive11.rx", true);
}

TEST(SemanticTest, TestcaseTest_Comprehensive12) {
  TestTestcase("../RCompiler-Testcases/semantic-2/src/comprehensive12/comprehensive12.rx", true);
}

TEST(SemanticTest, TestcaseTest_Comprehensive13) {
  TestTestcase("../RCompiler-Testcases/semantic-2/src/comprehensive13/comprehensive13.rx", true);
}

TEST(SemanticTest, TestcaseTest_Comprehensive14) {
  TestTestcase("../RCompiler-Testcases/semantic-2/src/comprehensive14/comprehensive14.rx", true);
}

TEST(SemanticTest, TestcaseTest_Comprehensive15) {
  TestTestcase("../RCompiler-Testcases/semantic-2/src/comprehensive15/comprehensive15.rx", true);
}

TEST(SemanticTest, TestcaseTest_Comprehensive16) {
  TestTestcase("../RCompiler-Testcases/semantic-2/src/comprehensive16/comprehensive16.rx", true);
}

TEST(SemanticTest, TestcaseTest_Comprehensive17) {
  TestTestcase("../RCompiler-Testcases/semantic-2/src/comprehensive17/comprehensive17.rx", true);
}

TEST(SemanticTest, TestcaseTest_Comprehensive18) {
  TestTestcase("../RCompiler-Testcases/semantic-2/src/comprehensive18/comprehensive18.rx", true);
}

TEST(SemanticTest, TestcaseTest_Comprehensive19) {
  TestTestcase("../RCompiler-Testcases/semantic-2/src/comprehensive19/comprehensive19.rx", true);
}

TEST(SemanticTest, TestcaseTest_Comprehensive20) {
  TestTestcase("../RCompiler-Testcases/semantic-2/src/comprehensive20/comprehensive20.rx", true);
}

TEST(SemanticTest, TestcaseTest_Comprehensive21) {
  TestTestcase("../RCompiler-Testcases/semantic-2/src/comprehensive21/comprehensive21.rx", true);
}

TEST(SemanticTest, TestcaseTest_Comprehensive22) {
  TestTestcase("../RCompiler-Testcases/semantic-2/src/comprehensive22/comprehensive22.rx", true);
}

TEST(SemanticTest, TestcaseTest_Comprehensive23) {
  TestTestcase("../RCompiler-Testcases/semantic-2/src/comprehensive23/comprehensive23.rx", true);
}

TEST(SemanticTest, TestcaseTest_Comprehensive24) {
  TestTestcase("../RCompiler-Testcases/semantic-2/src/comprehensive24/comprehensive24.rx", true);
}

TEST(SemanticTest, TestcaseTest_Comprehensive25) {
  TestTestcase("../RCompiler-Testcases/semantic-2/src/comprehensive25/comprehensive25.rx", true);
}

TEST(SemanticTest, TestcaseTest_Comprehensive26) {
  TestTestcase("../RCompiler-Testcases/semantic-2/src/comprehensive26/comprehensive26.rx", true);
}

TEST(SemanticTest, TestcaseTest_Comprehensive27) {
  TestTestcase("../RCompiler-Testcases/semantic-2/src/comprehensive27/comprehensive27.rx", true);
}

TEST(SemanticTest, TestcaseTest_Comprehensive28) {
  TestTestcase("../RCompiler-Testcases/semantic-2/src/comprehensive28/comprehensive28.rx", true);
}

TEST(SemanticTest, TestcaseTest_Comprehensive29) {
  TestTestcase("../RCompiler-Testcases/semantic-2/src/comprehensive29/comprehensive29.rx", true);
}

TEST(SemanticTest, TestcaseTest_Comprehensive30) {
  TestTestcase("../RCompiler-Testcases/semantic-2/src/comprehensive30/comprehensive30.rx", true);
}

TEST(SemanticTest, TestcaseTest_Comprehensive31) {
  TestTestcase("../RCompiler-Testcases/semantic-2/src/comprehensive31/comprehensive31.rx", true);
}

TEST(SemanticTest, TestcaseTest_Comprehensive32) {
  TestTestcase("../RCompiler-Testcases/semantic-2/src/comprehensive32/comprehensive32.rx", true);
}

TEST(SemanticTest, TestcaseTest_Comprehensive33) {
  TestTestcase("../RCompiler-Testcases/semantic-2/src/comprehensive33/comprehensive33.rx", true);
}

TEST(SemanticTest, TestcaseTest_Comprehensive34) {
  TestTestcase("../RCompiler-Testcases/semantic-2/src/comprehensive34/comprehensive34.rx", true);
}

TEST(SemanticTest, TestcaseTest_Comprehensive35) {
  TestTestcase("../RCompiler-Testcases/semantic-2/src/comprehensive35/comprehensive35.rx", true);
}

TEST(SemanticTest, TestcaseTest_Comprehensive36) {
  TestTestcase("../RCompiler-Testcases/semantic-2/src/comprehensive36/comprehensive36.rx", true);
}

TEST(SemanticTest, TestcaseTest_Comprehensive37) {
  TestTestcase("../RCompiler-Testcases/semantic-2/src/comprehensive37/comprehensive37.rx", true);
}

TEST(SemanticTest, TestcaseTest_Comprehensive38) {
  TestTestcase("../RCompiler-Testcases/semantic-2/src/comprehensive38/comprehensive38.rx", true);
}

TEST(SemanticTest, TestcaseTest_Comprehensive39) {
  TestTestcase("../RCompiler-Testcases/semantic-2/src/comprehensive39/comprehensive39.rx", true);
}

TEST(SemanticTest, TestcaseTest_Comprehensive40) {
  TestTestcase("../RCompiler-Testcases/semantic-2/src/comprehensive40/comprehensive40.rx", true);
}

TEST(SemanticTest, TestcaseTest_Comprehensive41) {
  TestTestcase("../RCompiler-Testcases/semantic-2/src/comprehensive41/comprehensive41.rx", true);
}

TEST(SemanticTest, TestcaseTest_Comprehensive42) {
  TestTestcase("../RCompiler-Testcases/semantic-2/src/comprehensive42/comprehensive42.rx", true);
}

TEST(SemanticTest, TestcaseTest_Comprehensive43) {
  TestTestcase("../RCompiler-Testcases/semantic-2/src/comprehensive43/comprehensive43.rx", true);
}

TEST(SemanticTest, TestcaseTest_Comprehensive44) {
  TestTestcase("../RCompiler-Testcases/semantic-2/src/comprehensive44/comprehensive44.rx", true);
}

TEST(SemanticTest, TestcaseTest_Comprehensive45) {
  TestTestcase("../RCompiler-Testcases/semantic-2/src/comprehensive45/comprehensive45.rx", true);
}

TEST(SemanticTest, TestcaseTest_Comprehensive46) {
  TestTestcase("../RCompiler-Testcases/semantic-2/src/comprehensive46/comprehensive46.rx", true);
}

TEST(SemanticTest, TestcaseTest_Comprehensive47) {
  TestTestcase("../RCompiler-Testcases/semantic-2/src/comprehensive47/comprehensive47.rx", true);
}

TEST(SemanticTest, TestcaseTest_Comprehensive48) {
  TestTestcase("../RCompiler-Testcases/semantic-2/src/comprehensive48/comprehensive48.rx", true);
}

TEST(SemanticTest, TestcaseTest_Comprehensive49) {
  TestTestcase("../RCompiler-Testcases/semantic-2/src/comprehensive49/comprehensive49.rx", true);
}

TEST(SemanticTest, TestcaseTest_Comprehensive50) {
  TestTestcase("../RCompiler-Testcases/semantic-2/src/comprehensive50/comprehensive50.rx", true);
}