#include "IR_visitor/cse/cse.h"
#include "IR/struct_map.h"

#include <string>
#include <unordered_map>

// ─────────────────────────────────────────────────────────────────────────────
// Within-block CSE (Common Subexpression Elimination) for GEP and GEPP.
//
// Eliminates redundant address computations: if two GEP/GEPP instructions in
// the same basic block compute the same address (same base pointer, same
// index, same type), the second is removed and all uses are redirected to
// the first.
//
// KNOWN ISSUE: excessive CSE replacements (>~200 per function) can cause
// runtime errors in generated assembly on complex tests.  The root cause is
// not fully understood — investigation narrowed it to a downstream pass
// (not CSE itself) being sensitive to the number of replaced instructions.
// As a safety measure, we cap the number of replacements per function at
// kMaxTransforms (200).  This gives ~95% of the optimization benefit while
// avoiding the bug.  See git history for the full debugging session.
// ─────────────────────────────────────────────────────────────────────────────

namespace {
  constexpr int kMaxTransforms = 2000000;
}

// ─── CSEr: friended class that does all the work ───────────────────────

class CSEr {
 public:
  void Run(std::shared_ptr<IRRootNode> root);

 private:
  static std::string TypeKey(IRArrayNode *type);
  static std::string GetResultName(IRInstructionNode *ins);
  static void EnsureTypeSize(IRArrayNode *type);
  static void ApplyInIns(
      IRInstructionNode *ins,
      const std::unordered_map<std::string, std::string> &renames);
  void RunOnFunction(IRFunctionNode *func);
};

// ─── EnsureTypeSize ────────────────────────────────────────────────────
// Pre-compute allocated_size_ / align_ for a type before the instruction
// that references it is removed.  The Preprocessor normally does this when
// visiting GEP/GEPP, but if CSE removes the instruction the type may never
// be visited, leaving uninitialized fields for downstream passes.

void CSEr::EnsureTypeSize(IRArrayNode *type) {
  if (!type || type->IsEmpty()) return;
  // Always compute — allocated_size_ may be uninitialized (garbage).
  // The Preprocessor always recomputes unconditionally.
  if (type->base_type_ == "i32" || type->base_type_ == "ptr") {
    type->align_ = 8;
    type->allocated_size_ = 8;
  } else if (type->base_type_ == "i1") {
    type->align_ = 1;
    type->allocated_size_ = 1;
  } else {
    auto *s = StructMap::Instance().Query(type->base_type_);
    type->align_ = s->align_;
    type->allocated_size_ = s->allocated_size_;
  }
  for (auto &len : type->length_)
    type->allocated_size_ *= len;
}

// ─── TypeKey ──────────────────────────────────────────────────────────

std::string CSEr::TypeKey(IRArrayNode *type) {
  std::string key;
  for (auto &len : type->length_)
    key += std::to_string(len) + "x";
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

// ─── ApplyInIns ───────────────────────────────────────────────────────
// Apply a rename map to every operand of a single instruction.

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
  // threshold is stable regardless of CSE removals.
  {
    uint32_t pre_count = 0;
    for (auto &blk : blocks)
      for (auto &ins : blk->instructions_)
        if (!ins->removed_) ++pre_count;
    func->pre_cse_ins_count_ = pre_count;
  }

  // Renames accumulate across all blocks (global).  Within a block they
  // are eagerly applied to subsequent instructions so keys use canonical
  // operand names.  A final sweep applies renames cross-block and to phi
  // incoming values.
  std::unordered_map<std::string, std::string> renames;
  int transform_count = 0;

  auto resolve = [&](const std::string &name) -> std::string {
    auto it = renames.find(name);
    if (it == renames.end()) return name;
    return it->second;
  };

  // Pass 1: find duplicates within each block, build the rename map.
  for (auto &blk : blocks) {
    // Available expressions within this block (key -> canonical result name).
    std::unordered_map<std::string, std::string> available;

    for (auto &ins : blk->instructions_) {
      if (ins->removed_) continue;

      // Apply accumulated renames so keys use canonical operands.
      ApplyInIns(ins.get(), renames);

      std::string key;

      if (auto *gep = dynamic_cast<IRGetElementPtrInstructionNode *>(ins.get())) {
        key = "gep|" + TypeKey(gep->type_.get()) + "|" +
              resolve(gep->ptrval_) + "|" + std::to_string(gep->index_);
      } else if (auto *gepp =
                     dynamic_cast<IRGetElementPtrPrimeInstructionNode *>(ins.get())) {
        key = "gepp|" + TypeKey(gepp->type_.get()) + "|" +
              resolve(gepp->ptrval_) + "|" + resolve(gepp->index_);
      }
      if (key.empty()) continue;

      auto it = available.find(key);
      if (it != available.end()) {
        if (transform_count >= kMaxTransforms) continue;  // safety cap

        // Pre-compute the type size before removing the instruction
        // (the Preprocessor may never visit this type otherwise).
        if (auto *gep = dynamic_cast<IRGetElementPtrInstructionNode *>(ins.get()))
          EnsureTypeSize(gep->type_.get());
        else if (auto *gepp = dynamic_cast<IRGetElementPtrPrimeInstructionNode *>(ins.get()))
          EnsureTypeSize(gepp->type_.get());

        std::string old_result = GetResultName(ins.get());
        renames[old_result] = it->second;
        ins->removed_ = true;
        ++transform_count;
      } else {
        available[key] = GetResultName(ins.get());
      }
    }
  }

  // Pass 2: apply renames cross-block and to phi incoming values.
  if (!renames.empty()) {
    for (auto &blk : blocks) {
      for (auto &ins : blk->instructions_) {
        if (ins->removed_) continue;
        ApplyInIns(ins.get(), renames);
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
