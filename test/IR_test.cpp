#include "gtest/gtest.h"
#include "data_loader/data_loader.h"
#include "lexer/lexer.h"
#include "parser/parser.hpp"
#include "parser/node/crate.h"

#include "visitor/printer/printer.h"
#include "visitor/checker/first_checker.h"
#include "visitor/checker/second_checker.h"
#include "visitor/checker/third_checker.h"
#include "visitor/IR_generator/IR_generator.h"
#include "IR_visitor/printer/IR_printer.h"
#include <fstream>

void TestCode(const std::string &code, std::ostream &out) {
  try {
    Lexer lexer(code);
    auto tokens = lexer.Run();
    Parser parser(tokens);
    auto root = parser.Run<CrateNode>();
    FirstChecker fc;
    fc.Run(root.get());
    SecondChecker sc;
    root->Accept(&sc);
    ThirdChecker tc;
    root->Accept(&tc);
    auto IR_root = std::make_shared<IRRootNode>();
    IRGenerator gen(IR_root);
    root->Accept(&gen);
    IRPrinter printer("../builtin.ll", out);
    IR_root->Accept(&printer);
  } catch (Error &err) {
    std::cerr << err.Info() << '\n';
    EXPECT_EQ(true, false);
  }
}

// TEST(IRTest, TestcaseTest) {
//   for (int t = 1; t <= 50; ++t) {
//     std::cerr << "Testing testcase" << t << "...\n";
//     std::string folder = "../testcases/IR-1/src/comprehensive" + std::to_string(t);
//     std::string input = LoadFromFile(folder + "/comprehensive" + std::to_string(t) + ".rx");
//     std::string output_file = "../tmp/" + std::to_string(t) + ".ll";
//     std::ofstream out(output_file);
//     if (!out.is_open()) {
//       throw std::runtime_error("Failed to open file for writing: " + output_file);
//     }
//     TestCode(input, out);
//     std::cerr << '\n';
//   }
// }

TEST(IRTest, MyTest) {
  for (int t = 8; t <= 9; ++t) {
    std::cerr << "Testing my test" << t << "...\n";
    std::string input = LoadFromFile("../tmp_data/" + std::to_string(t) + ".rx");
    std::string output_file = "../tmp_data/" + std::to_string(t) + ".ll";
    std::ofstream out(output_file);
    if (!out.is_open()) {
      throw std::runtime_error("Failed to open file for writing: " + output_file);
    }
    TestCode(input, out);
    std::cerr << '\n';
  }
}