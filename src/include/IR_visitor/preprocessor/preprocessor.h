#pragma once

#include "IR_visitor/base/IR_visitor_base.h"

class Preprocessor : public IRVisitorBase {
public:
  Preprocessor() = default;
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
  void Visit(IRPhiInstructionNode *) override;
  void Visit(IRMoveInstructionNode *) override;
  void Visit(IRSelectInstructionNode *) override;
  void Visit(IRBlockNode *) override;
  void Visit(IRParameterNode *) override;
  void Visit(IRFunctionNode *) override;
  void Visit(IRRootNode *) override;

  // Fold zero-index GEP(const) instructions: replace all uses of the result
  // with the ptrval and remove the GEP.  The index == 0 guarantees the byte
  // offset is zero, so the values are identical.
  // Must be called after Preprocessor::Accept (type sizes must be computed).
  static void FoldZeroOffsetGEPs(std::shared_ptr<IRRootNode> IR_root);

private:
  static void ReplaceVarInIns(IRInstructionNode *ins, const std::string &old_var,
                               const std::string &new_var);
  std::unordered_map<std::string, IRNode *> *current_variables_;
};
