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
#include "IR/function_map.h"

#include "mem2reg/mem2reg.h"
#include "mem2reg/eliminator.h"
#include "IR_visitor/function_inliner/function_inliner.h"
#include "IR_visitor/sccp/sccp.h"
#include "IR_visitor/cse/cse.h"
#include "IR_visitor/phi_eliminator/phi_eliminator.h"
#include "IR_visitor/empty_block_eliminator/empty_block_eliminator.h"
#include "reg_alloc/reg_alloc.h"

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

    FunctionInliner::Run(IR_root);

    // IRPrinter dbg("builtin.ll", std::cerr);
    // IR_root->Accept(&dbg);

    Mem2reg(IR_root);
    EliminateCriticalEdge(IR_root);

    SCCP(IR_root);

    CSE(IR_root);

    ReplacePhiWithMoves(IR_root);

    EliminateEmptyBlocks(IR_root);

    Preprocessor preprocessor;
    IR_root->Accept(&preprocessor);
    FunctionMap::Instance().Accept(&preprocessor);

    // Fold zero-offset GEPs into Moves for register coalescing.
    Preprocessor::FoldZeroOffsetGEPs(IR_root);

    MemoryAllocator memory_allocator;
    IR_root->Accept(&memory_allocator);
    FunctionMap::Instance().Accept(&memory_allocator);
    RegAlloc reg_alloc;
    IR_root->Accept(&reg_alloc);
    AssemblyGenerator assembly_generator(LoadFromFile("builtin_gcc.s"), out);
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
