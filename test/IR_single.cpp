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
#include "IR_visitor/preprocessor/preprocessor.h"
#include "IR_visitor/memory_allocator/memory_allocator.h"
#include "IR_visitor/assembly_generator/assembly_generator.h"
#include <fstream>
#include "IR/struct_map.h"

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

    IRPrinter printer("builtin.ll", out);
    IR_root->Accept(&printer);
  } catch (Error &err) {
    std::cerr << err.Info() << '\n';
  }
}

int main() {
  std::string code = LoadInput();
  TestCode(code, std::cout);
  return 0;
}