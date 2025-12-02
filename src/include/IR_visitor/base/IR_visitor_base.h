#pragma once

#include "IR/IR_node.h"

class IRVisitorBase {
public:
  virtual void Visit(IRStructNode *) = 0;
  virtual void Visit(IRArithmeticInstructionNode *) = 0;
  virtual void Visit(IRNegationInstructionNode *) = 0;
  virtual void Visit(IRBranchInstructionNode *) = 0;
  virtual void Visit(IRJumpInstructionNode *) = 0;
  virtual void Visit(IRReturnInstructionNode *) = 0;
  virtual void Visit(IRAllocateInstructionNode *) = 0;
  virtual void Visit(IRLoadInstructionNode *) = 0;
  virtual void Visit(IRStoreVariableInstructionNode *) = 0;
  virtual void Visit(IRStoreConstInstructionNode *) = 0;
  virtual void Visit(IRGetElementPtrInstructionNode *) = 0;
  virtual void Visit(IRGetElementPtrPrimeInstructionNode *) = 0;
  virtual void Visit(IRCompareInstructionNode *) = 0;
  virtual void Visit(IRArgumentNode *) = 0;
  virtual void Visit(IRCallInstructionNode *) = 0;
  virtual void Visit(IRSelectInstructionNode *) = 0;
  virtual void Visit(IRBlockNode *) = 0;
  virtual void Visit(IRParameterNode *) = 0;
  virtual void Visit(IRFunctionNode *) = 0;
  virtual void Visit(IRRootNode *) = 0;
};