#pragma once

#include <set>
#include "CFG.h"
#include "IR_visitor/base/IR_visitor_base.h"

class CFGBuilder : public IRVisitorBase {
public:
  CFGBuilder() = delete;
  explicit CFGBuilder(std::shared_ptr<CFG>);
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
private:
  std::shared_ptr<CFG> cfg_;
  uint32_t cur_block_{0};
  std::set<uint32_t> cur_def_;
  std::set<uint32_t> cur_use_;
  void Merge(IRInstructionNode *);
  void AddDef(IRInstructionNode *, const std::string &, bool);
  void AddUse(IRInstructionNode *, const std::string &, bool);
};