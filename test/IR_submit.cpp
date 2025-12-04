#include <iostream>
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

int main() {
  std::string code = LoadInput();
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
  IRPrinter printer("builtin.ll", std::cout);
  IR_root->Accept(&printer);
  return 0;
}