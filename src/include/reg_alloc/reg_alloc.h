#pragma once

#include "IR_visitor/base/IR_visitor_base.h"

class RegAlloc : public IRVisitorBase {
public:
  RegAlloc() = default;

  void Visit(IRArrayNode *) override {}
  void Visit(IRStructNode *) override {}
  void Visit(IRArithmeticInstructionNode *) override {}
  void Visit(IRNegationInstructionNode *) override {}
  void Visit(IRBranchInstructionNode *) override {}
  void Visit(IRJumpInstructionNode *) override {}
  void Visit(IRReturnInstructionNode *) override {}
  void Visit(IRAllocateInstructionNode *) override {}
  void Visit(IRLoadInstructionNode *) override {}
  void Visit(IRStoreVariableInstructionNode *) override {}
  void Visit(IRStoreConstInstructionNode *) override {}
  void Visit(IRGetElementPtrInstructionNode *) override {}
  void Visit(IRGetElementPtrPrimeInstructionNode *) override {}
  void Visit(IRCompareInstructionNode *) override {}
  void Visit(IRArgumentNode *) override {}
  void Visit(IRCallInstructionNode *) override {}
  void Visit(IRPhiInstructionNode *) override {}
  void Visit(IRMoveInstructionNode *) override {}
  void Visit(IRSelectInstructionNode *) override {}
  void Visit(IRBlockNode *) override {}
  void Visit(IRParameterNode *) override {}
  void Visit(IRFunctionNode *) override;
  void Visit(IRRootNode *) override;
private:
  static bool IsPromotableType(IRInstructionNode *);
  static std::vector<uint32_t> LeafColorPool(const IRFunctionNode *);
};
