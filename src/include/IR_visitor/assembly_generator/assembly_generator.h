#pragma once

#include <ostream>
#include <map>
#include "IR_visitor/base/IR_visitor_base.h"

class AssemblyGenerator : public IRVisitorBase {
public:
  AssemblyGenerator() = delete;
  AssemblyGenerator(const std::string &, const std::string &, std::ostream &);
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
  std::string builtin_begin_;
  std::string builtin_end_;
  std::ostream &os_;

  uint32_t current_stack_;
  std::string current_func_name_;
  uint32_t current_a_reg_used_;
  std::map<std::string, IRNode *> *current_variables_;

  std::pair<StorageType, uint32_t> GetVariableAddress(const std::string &);
  void TransferToTreg(uint32_t, uint32_t);
  std::string VariableToReg(const std::string &, uint32_t); // the second argument is the default t reg if the variable is stored in memory.
  void VariableForceToReg(const std::string &, const std::string &);
  std::string GetResultReg(StorageType, uint32_t, uint32_t); // the third argument is the default t reg if the result is stored in memory.
  void RegToVariable(StorageType, uint32_t, const std::string &);
  void SaveRegister();
  void RestoreRegister();
};