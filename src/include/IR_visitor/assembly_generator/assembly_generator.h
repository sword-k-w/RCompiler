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

  // Safety hook: BeforeWrite(rd) flushes any pending deferred kMemory store
  // before rd is overwritten.  Currently a no-op (deferred-store optimization
  // removed), retained so future implementations can't forget this safeguard.
  void FlushDeferredStore();
  void BeforeWrite(const std::string &rd) {
    // Deferred-store optimization removed — no-op, retained as a safety hook.
    (void)rd;
  }

  IRFunctionNode *cur_func_;
  uint32_t cur_block_;
  // Per-register liveness: true = hardware a-reg holds the canonical value;
  // false = hardware is call garbage, the save slot is the source of truth.
  bool a_reg_valid_[8]{true, true, true, true, true, true, true, true};
  bool ra_saved_{false};

  std::pair<StorageType, uint32_t> GetVariableAddress(const std::string &);
  void TransferToTreg(uint32_t, uint32_t, const std::string &);
  std::string VariableToReg(const std::string &, uint32_t, const std::string &);
  void VariableForceToReg(const std::string &, const std::string &, const std::string &);
  std::string GetResultReg(StorageType, uint32_t, uint32_t);
  void RegToVariable(StorageType, uint32_t, const std::string &, const std::string &);
  void SaveRegister();
  void EnsureARegValid(uint32_t addr);
  void ReloadConstCache();
  void FlushSavedRegisters();
  void DataMove(const std::string &, StorageType, uint32_t, std::shared_ptr<IRArrayNode>);
  void DataMoveFromReg(const std::string &, StorageType, uint32_t, std::shared_ptr<IRArrayNode>);

  // Wrappers that automatically pass the constant cache to PrintMem/PrintIA/PrintIStar.
  void EmitMem(const std::string &type, const std::string &r, const std::string &rs1, int32_t imm);
  void EmitIA(const std::string &type, const std::string &rd, const std::string &rs1, int32_t imm);
  void EmitIStar(const std::string &type, const std::string &rd, const std::string &rs1, int32_t imm);

  // Wrappers for instruction patterns not covered by the above.
  // Each calls BeforeWrite(rd) before emission — no caller should need
  // to remember.
  void EmitR(const std::string &op, const std::string &rd,
             const std::string &rs1, const std::string &rs2);
  void EmitUnary(const std::string &op, const std::string &rd, const std::string &rs);
  void EmitLI(const std::string &rd, const std::string &val);
  void EmitLI(const std::string &rd, int64_t val);
  void EmitMV(const std::string &rd, const std::string &rs);

  // GEP chain folding: compute the byte offset for a constant-index GEP.
  uint32_t ComputeGEPOffset(class IRGetElementPtrInstructionNode *node);
};
