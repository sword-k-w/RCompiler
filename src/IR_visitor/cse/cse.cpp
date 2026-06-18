#include "IR_visitor/cse/cse.h"
#include "IR/IR_node.h"
#include "liveness_analysis/dominator_tree.h"

#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

// ---------------------------------------------------------------------------
// Type encoding for signature keys
// ---------------------------------------------------------------------------
std::string CSE::TypeEncode(const std::shared_ptr<IRArrayNode> &type) {
  if (!type || type->IsEmpty()) return "void";
  std::string s = type->base_type_;
  for (auto len : type->length_) {
    s += "[" + std::to_string(len) + "]";
  }
  return s;
}

// ---------------------------------------------------------------------------
// Replace every string operand of |ins| whose value is a key in |subst|
// with the corresponding canonical name.
// ---------------------------------------------------------------------------
bool CSE::ReplaceOperands(
    IRInstructionNode *ins,
    const std::unordered_map<std::string, std::string> &subst) {
  if (subst.empty()) return false;

  auto replace = [&](std::string &field) -> bool {
    auto it = subst.find(field);
    if (it != subst.end()) { field = it->second; return true; }
    return false;
  };

  bool changed = false;

  if (auto *gep = dynamic_cast<IRGetElementPtrInstructionNode *>(ins)) {
    changed |= replace(gep->ptrval_);
    return changed;
  }
  if (auto *gepp = dynamic_cast<IRGetElementPtrPrimeInstructionNode *>(ins)) {
    changed |= replace(gepp->ptrval_);
    changed |= replace(gepp->index_);
    return changed;
  }
  if (auto *arith = dynamic_cast<IRArithmeticInstructionNode *>(ins)) {
    changed |= replace(arith->operand1_);
    changed |= replace(arith->operand2_);
    return changed;
  }
  if (auto *cmp = dynamic_cast<IRCompareInstructionNode *>(ins)) {
    changed |= replace(cmp->operand1_);
    changed |= replace(cmp->operand2_);
    return changed;
  }
  if (auto *br = dynamic_cast<IRBranchInstructionNode *>(ins)) {
    changed |= replace(br->condition_);
    return changed;
  }
  if (auto *ret = dynamic_cast<IRReturnInstructionNode *>(ins)) {
    if (!ret->type_->IsEmpty()) changed |= replace(ret->name_);
    return changed;
  }
  if (auto *load = dynamic_cast<IRLoadInstructionNode *>(ins)) {
    changed |= replace(load->pointer_);
    return changed;
  }
  if (auto *store = dynamic_cast<IRStoreVariableInstructionNode *>(ins)) {
    changed |= replace(store->value_);
    changed |= replace(store->pointer_);
    return changed;
  }
  if (auto *storec = dynamic_cast<IRStoreConstInstructionNode *>(ins)) {
    changed |= replace(storec->pointer_);
    return changed;
  }
  if (auto *mv = dynamic_cast<IRMoveInstructionNode *>(ins)) {
    changed |= replace(mv->source_);
    return changed;
  }
  if (auto *sel = dynamic_cast<IRSelectInstructionNode *>(ins)) {
    changed |= replace(sel->cond_);
    return changed;
  }
  if (auto *neg = dynamic_cast<IRNegationInstructionNode *>(ins)) {
    changed |= replace(neg->operand_);
    return changed;
  }
  if (auto *call = dynamic_cast<IRCallInstructionNode *>(ins)) {
    for (auto &arg : call->arguments_) {
      changed |= replace(arg->value_);
    }
    return changed;
  }
  return changed;
}

// ---------------------------------------------------------------------------
// Try to compute a {signature, result_name} pair for a pure instruction.
// Returns {"", ""} for non-CSE-able instructions.
// ---------------------------------------------------------------------------
std::pair<std::string, std::string> CSE::TryMakeKey(IRInstructionNode *ins) {
  if (auto *gep = dynamic_cast<IRGetElementPtrInstructionNode *>(ins)) {
    return {
      "gep|" + gep->ptrval_ + "|" + TypeEncode(gep->type_) + "|"
             + std::to_string(gep->index_),
      gep->result_
    };
  }
  if (auto *gepp = dynamic_cast<IRGetElementPtrPrimeInstructionNode *>(ins)) {
    return {
      "gepp|" + gepp->ptrval_ + "|" + gepp->index_ + "|"
              + TypeEncode(gepp->type_),
      gepp->result_
    };
  }
  return {};
}

// ---------------------------------------------------------------------------
// Apply accumulated substitutions to all instructions in a function.
// ---------------------------------------------------------------------------
void CSE::ApplySubstitutions(
    std::shared_ptr<IRFunctionNode> func,
    const std::unordered_map<std::string, std::string> &subst) {
  if (subst.empty()) return;
  for (auto &block : func->blocks_) {
    for (auto &ins : block->instructions_) {
      if (ins->removed_) continue;
      ReplaceOperands(ins.get(), subst);
    }
  }
}

// ---------------------------------------------------------------------------
// Dominance helper using DominatorTreeSolver (Tarjan, near-linear).
// Takes successor lists (already extracted from IR terminators by CSE::Run).
// ---------------------------------------------------------------------------
class DomInfo {
public:
  // |n| = number of blocks.  |succs| maps position → successor positions.
  DomInfo(size_t n,
          const std::vector<std::vector<uint32_t>> &succs,
          const std::vector<uint32_t> &pos_to_ir) {
    n_ = n;
    if (n_ == 0) return;

    for (size_t i = 0; i < n_; ++i)
      ir_to_pos_[pos_to_ir[i]] = static_cast<uint32_t>(i);

    DominatorTreeSolver dts;
    dts.Init(static_cast<uint32_t>(n_));

    for (size_t i = 0; i < n_; ++i) {
      uint32_t u = static_cast<uint32_t>(i) + 1;
      for (auto v_pos : succs[i]) {
        uint32_t v = v_pos + 1;
        dts.AddEdge(0, u, v);
        dts.AddEdge(1, v, u);
      }
    }

    dts.Tarjan(1);

    idom_.resize(n_);
    for (size_t i = 0; i < n_; ++i)
      idom_[i] = static_cast<int32_t>(dts.Query(static_cast<uint32_t>(i) + 1)) - 1;
  }

  bool Dominates(uint32_t ir_a, uint32_t ir_b) const {
    if (n_ == 0) return false;
    int32_t a = Pos(ir_a), b = Pos(ir_b);
    if (a < 0 || b < 0) return false;
    while (b != 0) {
      if (b == a) return true;
      b = idom_[b];
    }
    return a == 0;
  }

private:
  int32_t Pos(uint32_t ir_id) const {
    auto it = ir_to_pos_.find(ir_id);
    return (it != ir_to_pos_.end()) ? static_cast<int32_t>(it->second) : -1;
  }

  size_t n_{0};
  std::unordered_map<uint32_t, uint32_t> ir_to_pos_;
  std::vector<int32_t> idom_;
};

// ---------------------------------------------------------------------------
// Public entry point
// ---------------------------------------------------------------------------
void CSE::Run(std::shared_ptr<IRRootNode> root) {
  for (auto &func : root->functions_) {

    // --- build dominator info ---
    size_t nb = func->blocks_.size();
    std::unordered_map<uint32_t, uint32_t> ir_to_pos;
    std::vector<uint32_t> pos_to_ir(nb);
    std::vector<std::vector<uint32_t>> succs(nb);

    for (size_t i = 0; i < nb; ++i) {
      uint32_t id = func->blocks_[i]->GetID();
      ir_to_pos[id] = static_cast<uint32_t>(i);
      pos_to_ir[i] = id;
    }
    for (size_t i = 0; i < nb; ++i) {
      for (auto &ins : func->blocks_[i]->instructions_) {
        if (ins->removed_) continue;
        if (auto *br = dynamic_cast<IRBranchInstructionNode *>(ins.get())) {
          auto it_t = ir_to_pos.find(br->true_branch_);
          if (it_t != ir_to_pos.end()) succs[i].push_back(it_t->second);
          auto it_f = ir_to_pos.find(br->false_branch_);
          if (it_f != ir_to_pos.end()) succs[i].push_back(it_f->second);
          break;
        }
        if (auto *j = dynamic_cast<IRJumpInstructionNode *>(ins.get())) {
          auto it = ir_to_pos.find(j->destination_);
          if (it != ir_to_pos.end()) succs[i].push_back(it->second);
          break;
        }
      }
    }

    DomInfo dom_info(nb, succs, pos_to_ir);

    std::unordered_map<std::string, std::string> subst;
    std::unordered_map<std::string, std::pair<std::string, uint32_t>> seen;

    bool changed;
    do {
      changed = false;

      for (auto &block : func->blocks_) {
        uint32_t cur_id = block->GetID();

        for (auto &ins : block->instructions_) {
          if (ins->removed_) continue;

          ReplaceOperands(ins.get(), subst);

          auto [sig, result_name] = TryMakeKey(ins.get());
          if (sig.empty()) continue;

          auto it = seen.find(sig);
          if (it != seen.end()) {
            auto &[canon_name, canon_block] = it->second;
            if (dom_info.Dominates(canon_block, cur_id)) {
              if (result_name != canon_name) {
                auto chain = subst.find(canon_name);
                const std::string &ultimate =
                    (chain != subst.end()) ? chain->second : canon_name;
                subst[result_name] = ultimate;
                ins->removed_ = true;
                changed = true;
              }
            }
          } else {
            seen[sig] = {result_name, cur_id};
          }
        }
      }

      if (changed) ApplySubstitutions(func, subst);

    } while (changed);
  }
}
