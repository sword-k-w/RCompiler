#include "IR_visitor/cse/cse.h"

#include <string>
#include <unordered_map>

// ─── Debug flags for narrowing down the 'f**k RISCV' RE ──────────────
// Set to false to disable a specific CSE category and test on OJ.

namespace cse_debug {
  constexpr bool kCSE_GEP   = false;  // constant-index GEP
  constexpr bool kCSE_GEPP  = true;   // variable-index GEP'
  constexpr bool kCrossBlockRename = true;  // propagate renames to other blocks
  constexpr bool kGEPP_NoPhiBlocks = false;  // if true, skip GEPP CSE in blocks with phi nodes
  constexpr bool kGEPP_NoRenameOperands = false; // if true, use raw operands (no resolve)
  constexpr bool kCSE_UseMove = true;   // if true, use Move instr instead of rename+remove
  constexpr bool kCSE_DryRun  = false;  // if true, find duplicates but DON'T modify IR at all
}

// ─── CSEr: friended class that does all the work ───────────────────────

class CSEr {
 public:
  void Run(std::shared_ptr<IRRootNode> root);

 private:
  static std::string TypeKey(IRArrayNode *type);
  static std::string GetResultName(IRInstructionNode *ins);
  static void ApplyInIns(
      IRInstructionNode *ins,
      const std::unordered_map<std::string, std::string> &renames);
  void RunOnFunction(IRFunctionNode *func);
};

// ─── TypeKey ──────────────────────────────────────────────────────────

std::string CSEr::TypeKey(IRArrayNode *type) {
  std::string key;
  for (auto &len : type->length_) {
    key += std::to_string(len) + "x";
  }
  key += type->base_type_;
  return key;
}

// ─── GetResultName ────────────────────────────────────────────────────

std::string CSEr::GetResultName(IRInstructionNode *ins) {
  if (auto *a = dynamic_cast<IRArithmeticInstructionNode *>(ins))
    return a->result_;
  if (auto *c = dynamic_cast<IRCompareInstructionNode *>(ins))
    return c->result_;
  if (auto *n = dynamic_cast<IRNegationInstructionNode *>(ins))
    return n->result_;
  if (auto *g = dynamic_cast<IRGetElementPtrInstructionNode *>(ins))
    return g->result_;
  if (auto *gp = dynamic_cast<IRGetElementPtrPrimeInstructionNode *>(ins))
    return gp->result_;
  return "";
}

// ─── apply operand renames within a single instruction ────────────────

void CSEr::ApplyInIns(
    IRInstructionNode *ins,
    const std::unordered_map<std::string, std::string> &renames) {
  if (renames.empty()) return;
  auto apply = [&](std::string &s) {
    auto it = renames.find(s);
    if (it != renames.end()) s = it->second;
  };

  if (auto *a = dynamic_cast<IRArithmeticInstructionNode *>(ins)) {
    apply(a->operand1_); apply(a->operand2_);
  } else if (auto *c = dynamic_cast<IRCompareInstructionNode *>(ins)) {
    apply(c->operand1_); apply(c->operand2_);
  } else if (auto *b = dynamic_cast<IRBranchInstructionNode *>(ins)) {
    apply(b->condition_);
  } else if (auto *m = dynamic_cast<IRMoveInstructionNode *>(ins)) {
    apply(m->source_);
  } else if (auto *s = dynamic_cast<IRSelectInstructionNode *>(ins)) {
    apply(s->cond_);
  } else if (auto *n = dynamic_cast<IRNegationInstructionNode *>(ins)) {
    apply(n->operand_);
  } else if (auto *r = dynamic_cast<IRReturnInstructionNode *>(ins)) {
    apply(r->name_);
  } else if (auto *cl = dynamic_cast<IRCallInstructionNode *>(ins)) {
    for (auto &arg : cl->arguments_) apply(arg->value_);
  } else if (auto *l = dynamic_cast<IRLoadInstructionNode *>(ins)) {
    apply(l->pointer_);
  } else if (auto *sv = dynamic_cast<IRStoreVariableInstructionNode *>(ins)) {
    apply(sv->value_); apply(sv->pointer_);
  } else if (auto *sc = dynamic_cast<IRStoreConstInstructionNode *>(ins)) {
    apply(sc->pointer_);
  } else if (auto *g = dynamic_cast<IRGetElementPtrInstructionNode *>(ins)) {
    apply(g->ptrval_);
  } else if (auto *gp = dynamic_cast<IRGetElementPtrPrimeInstructionNode *>(ins)) {
    apply(gp->ptrval_); apply(gp->index_);
  }
}

// ─── RunOnFunction ────────────────────────────────────────────────────

void CSEr::RunOnFunction(IRFunctionNode *func) {
  auto &blocks = func->blocks_;

  // Save pre-CSE instruction count so the assembly generator's long-jump
  // threshold decision is stable regardless of how many insns CSE removes.
  {
    uint32_t pre_count = 0;
    for (auto &blk : blocks)
      for (auto &ins : blk->instructions_)
        if (!ins->removed_) ++pre_count;
    func->pre_cse_ins_count_ = pre_count;
  }

  // Process each block independently for CSE discovery.  Within a block,
  // renames are eagerly applied to subsequent instructions via ApplyInIns.
  // After all blocks are processed, a final pass propagates every rename
  // to all instructions and phi nodes in all blocks (cross-block uses).

  // Renames accumulate across all blocks.  Within a block, they are
  // eagerly applied to subsequent instructions (so keys are computed
  // with canonical operand names).  After all blocks, a final sweep
  // propagates every rename to all blocks for cross-block uses.
  std::unordered_map<std::string, std::string> renames;

  auto resolve = [&](const std::string &name) -> std::string {
    auto it = renames.find(name);
    if (it == renames.end()) return name;
    return it->second;  // chains are length 1 (canonical names are never renamed)
  };

  for (auto &blk : blocks) {
    // Available expressions within this block (key -> canonical result name).
    std::unordered_map<std::string, std::string> available;

    for (auto &ins : blk->instructions_) {
      if (ins->removed_) continue;

      // Eagerly apply accumulated renames so keys use canonical operands.
      ApplyInIns(ins.get(), renames);

      std::string key;

      if (cse_debug::kCSE_GEP) {
        if (auto *gep = dynamic_cast<IRGetElementPtrInstructionNode *>(ins.get())) {
          key = "gep|" + TypeKey(gep->type_.get()) + "|" +
                resolve(gep->ptrval_) + "|" + std::to_string(gep->index_);
        }
      }
      if (key.empty() && cse_debug::kCSE_GEPP) {
        // Skip GEPP CSE in blocks with phi nodes when the flag is set.
        if (cse_debug::kGEPP_NoPhiBlocks && !blk->phi_.empty())
          goto skip_cse;
        if (auto *gepp =
                   dynamic_cast<IRGetElementPtrPrimeInstructionNode *>(ins.get())) {
          if (cse_debug::kGEPP_NoRenameOperands) {
            // Use raw operands without resolving through the rename map.
            key = "gepp|" + TypeKey(gepp->type_.get()) + "|" +
                  gepp->ptrval_ + "|" + gepp->index_;
          } else {
            key = "gepp|" + TypeKey(gepp->type_.get()) + "|" +
                  resolve(gepp->ptrval_) + "|" + resolve(gepp->index_);
          }
        }
      }
      skip_cse:
      if (key.empty()) {
        continue;
      }

      auto it = available.find(key);
      if (it != available.end()) {
        if constexpr (!cse_debug::kCSE_DryRun) {
          std::string old_result = GetResultName(ins.get());
          if constexpr (cse_debug::kCSE_UseMove) {
            // Replace the duplicate GEP/GEPP with a Move from the canonical.
            auto ptr_type = std::make_shared<IRArrayNode>();
            ptr_type->base_type_ = "ptr";
            ins = std::make_shared<IRMoveInstructionNode>(
                old_result, it->second, ptr_type);
          } else {
            renames[old_result] = it->second;
            ins->removed_ = true;
          }
        }
      } else {
        available[key] = GetResultName(ins.get());
      }
    }
  }

  // Phi fixup: ALWAYS update phi incoming values that reference renamed
  // variables.  This is essential for correctness — a GEPP removed in
  // block A (which has no phi nodes) may still be referenced by a phi in
  // successor block B.  Skipping this produces 'no storage assigned'.
  //
  // Cross-block instruction rename (the ApplyInIns loop) is controlled
  // separately by kCrossBlockRename for debugging.
  if (!renames.empty()) {
    for (auto &blk : blocks) {
      if (cse_debug::kCrossBlockRename) {
        for (auto &ins : blk->instructions_) {
          if (ins->removed_) continue;
          ApplyInIns(ins.get(), renames);
        }
      }
      for (auto &phi : blk->phi_) {
        for (auto &pair : phi->val_) {
          auto it = renames.find(pair.first);
          if (it != renames.end()) pair.first = it->second;
        }
      }
    }
  }
}

// ─── Top-level entry point ─────────────────────────────────────────────

void CSEr::Run(std::shared_ptr<IRRootNode> root) {
  for (auto &func : root->functions_) {
    if (func->IsBuiltin()) continue;
    RunOnFunction(func.get());
  }
}

void CSE(std::shared_ptr<IRRootNode> root) {
  CSEr().Run(root);
}
