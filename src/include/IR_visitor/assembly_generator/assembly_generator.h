#pragma once

#include <ostream>
#include <map>
#include <set>
#include "IR_visitor/base/IR_visitor_base.h"

class AssemblyGenerator : public IRVisitorBase {
public:
  AssemblyGenerator() = delete;
  AssemblyGenerator(const std::string &, std::ostream &, std::ostream *builtin_os = nullptr);
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
  void Visit(IRPhiInstructionNode *) override {}
  void Visit(IRMoveInstructionNode *) override;
  void Visit(IRSelectInstructionNode *) override;
  void Visit(IRBlockNode *) override;
  void Visit(IRParameterNode *) override;
  void Visit(IRFunctionNode *) override;
  void Visit(IRRootNode *) override;
private:
  std::string builtin_begin_;
  std::ostream &os_;
  std::ostream *builtin_os_;

  uint32_t current_stack_;
  std::string current_func_name_;
  uint32_t current_a_reg_used_;
  std::unordered_map<std::string, IRNode *> *current_variables_;
  std::unordered_map<std::string, std::pair<StorageType, uint32_t>> *variable_storage_;

  uint32_t branch_cnt_{0};

  bool large_function_{false};

  std::unordered_map<uint32_t, uint32_t> next_block_map_;
  std::set<uint32_t> referenced_blocks_;
  std::unordered_map<uint32_t, uint32_t> block_label_map_;
  uint32_t next_label_id_{0};

  // Map constant value → register name for pre-loaded constants
  // (t3, t4 — registers that no other code path writes to).
  std::unordered_map<int64_t, std::string> const_cache_;

  IRFunctionNode *cur_func_;
  uint32_t cur_block_;
  bool registers_saved_{false};
  uint32_t cur_ins_index_{0};
  std::deque<std::shared_ptr<IRInstructionNode>> *cur_instructions_{nullptr};

  bool NextInstructionIsCall();

  std::pair<StorageType, uint32_t> GetVariableAddress(const std::string &);
  void TransferToTreg(uint32_t, uint32_t, const std::string &);
  std::string VariableToReg(const std::string &, uint32_t, const std::string &);
  void VariableForceToReg(const std::string &, const std::string &, const std::string &);
  std::string GetResultReg(StorageType, uint32_t, uint32_t);
  void RegToVariable(StorageType, uint32_t, const std::string &, const std::string &);
  void SaveRegister();
  void RestoreRegister();
  void FlushSavedRegisters();
  void DataMove(const std::string &, StorageType, uint32_t, std::shared_ptr<IRArrayNode>);
  void DataMoveFromReg(const std::string &, StorageType, uint32_t, std::shared_ptr<IRArrayNode>);
};
