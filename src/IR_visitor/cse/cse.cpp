#include "IR_visitor/cse/cse.h"
#include "IR/IR_node.h"
#include "liveness_analysis/dominator_tree.h"

#include <algorithm>
#include <string>
#include <unordered_map>
#include <unordered_set>
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
// Build a map from block ID → set of variable names defined by moves in
// that block.  These are post-phi-elimination variables with potentially
// multiple definitions.
// ---------------------------------------------------------------------------
CSE::MoveDefMap CSE::BuildMoveDefs(std::shared_ptr<IRFunctionNode> func,
                                   std::unordered_set<std::string> &all_move_names) {
  MoveDefMap move_defs;
  for (auto &block : func->blocks_) {
    uint32_t bid = block->GetID();
    for (auto &ins : block->instructions_) {
      if (ins->removed_) continue;
      if (auto *mv = dynamic_cast<IRMoveInstructionNode *>(ins.get())) {
        move_defs[bid].insert(mv->result_);
        all_move_names.insert(mv->result_);
      }
    }
  }
  return move_defs;
}

// ---------------------------------------------------------------------------
// Encode an operand for the CSE key.  If the operand is a move-defined
// variable, the reaching definition block is appended so that different
// definitions produce different keys.
// ---------------------------------------------------------------------------
std::string CSE::EncodeOperand(
    const std::string &name,
    uint32_t cur_block_id,
    size_t cur_ins_idx,
    const MoveDefMap &move_defs,
    const std::unordered_set<std::string> &all_move_names,
    const std::unordered_map<uint32_t, uint32_t> &ir_to_pos,
    const std::vector<int32_t> &idom,
    const std::vector<std::shared_ptr<IRBlockNode>> &blocks) {

  // Fast path: if this name is never defined by a move, it's an SSA name.
  if (!all_move_names.count(name)) return name;

  // Walk up the dominator tree from cur_block_id to find the nearest
  // move-definition of |name| that reaches this use.
  // For the current block, only moves BEFORE cur_ins_idx count.
  // For ancestor blocks, any move counts (the whole block executes before
  // dominated blocks).

  auto pos_of = [&](uint32_t ir_id) -> int32_t {
    auto it = ir_to_pos.find(ir_id);
    return (it != ir_to_pos.end()) ? static_cast<int32_t>(it->second) : -1;
  };

  int32_t cur_pos = pos_of(cur_block_id);
  if (cur_pos < 0) return name;  // shouldn't happen

  // Build a reverse map: dominator-tree position → IR block ID.
  // Used for O(1) lookup during the idom walk below.
  std::unordered_map<int32_t, uint32_t> pos_to_id;
  for (auto &[id, p] : ir_to_pos) {
    pos_to_id[static_cast<int32_t>(p)] = id;
  }

  // Check current block first: look for the LAST move defining |name|
  // before cur_ins_idx (the most recent definition wins).
  if (move_defs.count(cur_block_id)) {
    bool found = false;
    for (auto &blk : blocks) {
      if (blk->GetID() != cur_block_id) continue;
      size_t idx = 0;
      for (auto &ins : blk->instructions_) {
        if (idx >= cur_ins_idx) break;
        if (ins->removed_) { ++idx; continue; }
        if (auto *mv = dynamic_cast<IRMoveInstructionNode *>(ins.get())) {
          if (mv->result_ == name) {
            found = true;  // keep scanning for a later definition
          }
        }
        ++idx;
      }
      break;
    }
    if (found) return name + "@" + std::to_string(cur_block_id);
  }

  // Walk up the dominator tree.
  int32_t pos = idom[cur_pos];
  while (pos >= 0) {
    auto it = pos_to_id.find(pos);
    if (it != pos_to_id.end()) {
      uint32_t anc_id = it->second;
      if (move_defs.count(anc_id) &&
          move_defs.at(anc_id).count(name)) {
        return name + "@" + std::to_string(anc_id);
      }
    }
    pos = idom[pos];
  }

  // No move-definition found on the dominator path — the SSA definition
  // (non-move) dominates this use.  The bare name is sufficient.
  return name;
}

// ---------------------------------------------------------------------------
// Try to compute a {signature, result_name} pair for a pure instruction.
// Returns {"", ""} for non-CSE-able instructions.
// ---------------------------------------------------------------------------
std::pair<std::string, std::string> CSE::TryMakeKey(
    IRInstructionNode *ins,
    uint32_t cur_block_id,
    size_t cur_ins_idx,
    const MoveDefMap &move_defs,
    const std::unordered_set<std::string> &all_move_names,
    const std::unordered_map<uint32_t, uint32_t> &ir_to_pos,
    const std::vector<int32_t> &idom,
    const std::vector<std::shared_ptr<IRBlockNode>> &blocks) {
  if (auto *gep = dynamic_cast<IRGetElementPtrInstructionNode *>(ins)) {
    return {
      "gep|"
      + EncodeOperand(gep->ptrval_, cur_block_id, cur_ins_idx,
                       move_defs, all_move_names, ir_to_pos, idom, blocks)
      + "|" + TypeEncode(gep->type_) + "|"
      + std::to_string(gep->index_),
      gep->result_
    };
  }
  if (auto *gepp = dynamic_cast<IRGetElementPtrPrimeInstructionNode *>(ins)) {
    return {
      "gepp|"
      + EncodeOperand(gepp->ptrval_, cur_block_id, cur_ins_idx,
                       move_defs, all_move_names, ir_to_pos, idom, blocks)
      + "|"
      + EncodeOperand(gepp->index_, cur_block_id, cur_ins_idx,
                       move_defs, all_move_names, ir_to_pos, idom, blocks)
      + "|" + TypeEncode(gepp->type_),
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

  const std::vector<int32_t> &idom() const { return idom_; }
  const std::unordered_map<uint32_t, uint32_t> &ir_to_pos() const { return ir_to_pos_; }

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

    // --- pre-scan move definitions (post-phi-elimination reassignments) ---
    std::unordered_set<std::string> all_move_names;
    MoveDefMap move_defs = BuildMoveDefs(func, all_move_names);

    std::unordered_map<std::string, std::string> subst;
    std::unordered_map<std::string, std::pair<std::string, uint32_t>> seen;

    bool changed;
    do {
      changed = false;

      for (auto &block : func->blocks_) {
        uint32_t cur_id = block->GetID();

        size_t ins_idx = 0;
        for (auto &ins : block->instructions_) {
          if (ins->removed_) { ++ins_idx; continue; }

          ReplaceOperands(ins.get(), subst);

          auto [sig, result_name] = TryMakeKey(
              ins.get(), cur_id, ins_idx,
              move_defs, all_move_names, dom_info.ir_to_pos(), dom_info.idom(),
              func->blocks_);
          if (sig.empty()) { ++ins_idx; continue; }

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
          ++ins_idx;
        }
      }

      if (changed) ApplySubstitutions(func, subst);

    } while (changed);
  }
}
