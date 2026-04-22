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
#include "IR/function_map.h"

#include "IR/struct_map.h"

void TestCode(const std::string &code, std::ostream &out) {
  try {
    FunctionMap::Instance().Clear();
    FunctionMap::Instance().Init();
    StructMap::Instance().Clear();

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

    Preprocessor preprocessor;
    IR_root->Accept(&preprocessor);
    FunctionMap::Instance().Accept(&preprocessor);
    MemoryAllocator memory_allocator;
    IR_root->Accept(&memory_allocator);
    FunctionMap::Instance().Accept(&memory_allocator);
    AssemblyGenerator assembly_generator(LoadFromFile("builtin_begin.s"), LoadFromFile("builtin_end.s"), out);
    IR_root->Accept(&assembly_generator);
  } catch (Error &err) {
    std::cerr << err.Info() << '\n';
    exit(-1);
  }
}

int main() {
  std::string code = LoadInput();
  TestCode(code, std::cout);
  return 0;
}