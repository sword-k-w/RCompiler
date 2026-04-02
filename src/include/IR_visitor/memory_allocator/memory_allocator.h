#pragma once

#include "IR_visitor/base/IR_visitor_base.h"

uint32_t Align4(uint32_t);

class MemoryAllocator : public IRVisitorBase {
public:
  MemoryAllocator() = default;
  void Visit(IRArrayNode *) override;
  void Visit(IRStructNode *) override;
  void Visit(IRArithmeticInstructionNode *) override;
  void Visit(IRNegationInstructionNode *) override;
  void Visit(IRBranchInstructionNode *) override;
  void Visit(IRJumpInstructionNode *) override;
  void Visit(IRReturnInstructionNode *) override;
  void Visit(IRAllocateInstructionNode *) override;
  void Visit(IRLoadInstructionNode *) override;
  void Visit(IRStoreVariableInstructionNode *) override;
  void Visit(IRStoreConstInstructionNode *) override;
  void Visit(IRGetElementPtrInstructionNode *) override;
  void Visit(IRGetElementPtrPrimeInstructionNode *) override;
  void Visit(IRCompareInstructionNode *) override;
  void Visit(IRArgumentNode *) override;
  void Visit(IRCallInstructionNode *) override;
  void Visit(IRSelectInstructionNode *) override;
  void Visit(IRBlockNode *) override;
  void Visit(IRParameterNode *) override;
  void Visit(IRFunctionNode *) override;
  void Visit(IRRootNode *) override;
private:
  uint32_t *current_stack_;
  uint32_t current_parameter_register_;
};