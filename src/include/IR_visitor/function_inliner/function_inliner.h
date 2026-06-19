#pragma once

#include "IR/IR_node.h"
#include "IR/name_allocator.h"

#include <set>

class FunctionInliner {
public:
  static void Run(std::shared_ptr<IRRootNode>);

private:
  static bool InlineCallsInFunction(std::shared_ptr<IRFunctionNode>,
                                    std::shared_ptr<IRRootNode>,
                                    uint32_t max_insn,
                                    NameAllocator &,
                                    std::set<IRFunctionNode *> &);
  static uint32_t CountInstructions(IRFunctionNode *);
  static bool HasRecursiveCall(IRFunctionNode *);
  static std::shared_ptr<IRInstructionNode> CloneInstruction(
      std::shared_ptr<IRInstructionNode>,
      const std::unordered_map<std::string, std::string> &,
      const std::unordered_map<uint32_t, uint32_t> &);
  static void InlineCall(std::shared_ptr<IRFunctionNode>,
                         std::shared_ptr<IRBlockNode>,
                         size_t call_idx,
                         IRFunctionNode *,
                         std::shared_ptr<IRCallInstructionNode>,
                         NameAllocator &);
};
