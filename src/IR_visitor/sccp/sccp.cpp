#include "IR_visitor/sccp/sccp.h"

#include <algorithm>
#include <cstdint>
#include <queue>
#include <string>
#include <unordered_map>
#include <vector>

// ─── Lattice ───────────────────────────────────────────────────────────

enum class LatticeKind { kTop, kConstant, kBottom };

struct LatticeCell {
  LatticeKind kind = LatticeKind::kTop;
  int32_t value = 0;

  bool operator==(const LatticeCell &o) const {
    if (kind != o.kind) return false;
    if (kind == LatticeKind::kConstant) return value == o.value;
    return true;
  }
  bool operator!=(const LatticeCell &o) const { return !(*this == o); }
};

// ─── SCCPer: friended class that does all the work ─────────────────────

class SCCPer {
 public:
  void Run(std::shared_ptr<IRRootNode> root);

 private:
  // ── Helpers (all private-field access happens here) ──
  static bool IsVariable(const std::string &s) {
    return !s.empty() && s[0] == '%';
  }
  static bool IsConstantString(const std::string &s) {
    return !s.empty() && !IsVariable(s);
  }
  static int32_t ParseConstant(const std::string &s) {
    int64_t v = std::stoll(s);
    if (v > 0x7FFFFFFFLL && v <= 0xFFFFFFFFLL)
      v = int32_t(uint32_t(v));
    return int32_t(v);
  }

  static LatticeCell GetLattice(
      const std::unordered_map<std::string, LatticeCell> &lattice,
      const std::string &name) {
    if (IsConstantString(name))
      return {LatticeKind::kConstant, ParseConstant(name)};
    auto it = lattice.find(name);
    if (it != lattice.end()) return it->second;
    return {LatticeKind::kTop, 0};
  }

  static LatticeCell Meet(
      const std::unordered_map<std::string, LatticeCell> &lattice,
      const LatticeCell &cur, const std::string &val_str) {
    LatticeCell other = GetLattice(lattice, val_str);
    if (cur.kind == LatticeKind::kTop) return other;
    if (other.kind == LatticeKind::kTop) return cur;
    if (cur.kind == LatticeKind::kConstant &&
        other.kind == LatticeKind::kConstant) {
      if (cur.value == other.value) return cur;
      return {LatticeKind::kBottom, 0};
    }
    return {LatticeKind::kBottom, 0};
  }

  // Constant folding
  static int32_t FoldArithmetic(int32_t a, int32_t b, const std::string &op,
                                 bool is_unsigned);
  static bool FoldCompare(int32_t a, int32_t b,
                          IRCompareInstructionNode::Operator op);

  // Check whether an instruction uses a given variable
  static bool InsUsesVar(IRInstructionNode *ins, const std::string &var);

  // Replace old_var with new_str in a single instruction
  static void ReplaceInIns(IRInstructionNode *ins, const std::string &old_var,
                            const std::string &new_str);

  // Replace all uses of old_var with new_str across all blocks/instructions/phis
  static void ReplaceAllUses(
      std::vector<std::shared_ptr<IRBlockNode>> &blocks,
      const std::string &old_var, const std::string &new_str);

  // Collect all operand variable names from an instruction
  static void CollectOperandVars(IRInstructionNode *ins,
                                  std::vector<std::string> &out);
  // Collect all operand variable names from a phi
  static void CollectPhiOperandVars(IRPhiInstructionNode *phi,
                                     std::vector<std::string> &out);

  // Main per-function SCCP driver
  void RunOnFunction(IRFunctionNode *func);
};

// ─── Constant folding impl ─────────────────────────────────────────────

int32_t SCCPer::FoldArithmetic(int32_t a, int32_t b, const std::string &op,
                                bool is_unsigned) {
  uint32_t ua = uint32_t(a), ub = uint32_t(b);
  if (op == "+") return int32_t(ua + ub);
  if (op == "-") return int32_t(ua - ub);
  if (op == "*") return int32_t(ua * ub);
  if (op == "/") {
    if (b == 0) return a;
    return is_unsigned ? int32_t(ua / ub) : a / b;
  }
  if (op == "%") {
    if (b == 0) return 0;
    return is_unsigned ? int32_t(ua % ub) : a % b;
  }
  if (op == "<<") return int32_t(ua << (ub & 31));
  if (op == ">>") {
    return is_unsigned ? int32_t(ua >> (ub & 31)) : a >> (ub & 31);
  }
  if (op == "&") return int32_t(ua & ub);
  if (op == "|") return int32_t(ua | ub);
  if (op == "^") return int32_t(ua ^ ub);
  return 0;
}

bool SCCPer::FoldCompare(int32_t a, int32_t b,
                          IRCompareInstructionNode::Operator op) {
  uint32_t ua = uint32_t(a), ub = uint32_t(b);
  switch (op) {
    case IRCompareInstructionNode::kEq:  return a == b;
    case IRCompareInstructionNode::kNe:  return a != b;
    case IRCompareInstructionNode::kUgt: return ua > ub;
    case IRCompareInstructionNode::kUge: return ua >= ub;
    case IRCompareInstructionNode::kUlt: return ua < ub;
    case IRCompareInstructionNode::kUle: return ua <= ub;
    case IRCompareInstructionNode::kSgt: return a > b;
    case IRCompareInstructionNode::kSge: return a >= b;
    case IRCompareInstructionNode::kSlt: return a < b;
    case IRCompareInstructionNode::kSle: return a <= b;
  }
  return false;
}

// ─── Operand helpers (access private instruction fields) ───────────────

bool SCCPer::InsUsesVar(IRInstructionNode *ins, const std::string &var) {
  if (auto *arith = dynamic_cast<IRArithmeticInstructionNode *>(ins))
    return arith->operand1_ == var || arith->operand2_ == var;
  if (auto *cmp = dynamic_cast<IRCompareInstructionNode *>(ins))
    return cmp->operand1_ == var || cmp->operand2_ == var;
  if (auto *br = dynamic_cast<IRBranchInstructionNode *>(ins))
    return br->condition_ == var;
  if (auto *mv = dynamic_cast<IRMoveInstructionNode *>(ins))
    return mv->source_ == var;
  if (auto *sel = dynamic_cast<IRSelectInstructionNode *>(ins))
    return sel->cond_ == var;
  if (auto *neg = dynamic_cast<IRNegationInstructionNode *>(ins))
    return neg->operand_ == var;
  if (auto *ret = dynamic_cast<IRReturnInstructionNode *>(ins))
    return ret->name_ == var;
  if (auto *call = dynamic_cast<IRCallInstructionNode *>(ins)) {
    for (auto &arg : call->arguments_)
      if (arg->value_ == var) return true;
    return false;
  }
  if (auto *load = dynamic_cast<IRLoadInstructionNode *>(ins))
    return load->pointer_ == var;
  if (auto *store_v = dynamic_cast<IRStoreVariableInstructionNode *>(ins))
    return store_v->value_ == var || store_v->pointer_ == var;
  if (auto *store_c = dynamic_cast<IRStoreConstInstructionNode *>(ins))
    return store_c->pointer_ == var;
  if (auto *gep = dynamic_cast<IRGetElementPtrInstructionNode *>(ins))
    return gep->ptrval_ == var;
  if (auto *gepp = dynamic_cast<IRGetElementPtrPrimeInstructionNode *>(ins))
    return gepp->ptrval_ == var || gepp->index_ == var;
  return false;
}

void SCCPer::ReplaceInIns(IRInstructionNode *ins, const std::string &old_var,
                           const std::string &new_str) {
  if (auto *arith = dynamic_cast<IRArithmeticInstructionNode *>(ins)) {
    if (arith->operand1_ == old_var) arith->operand1_ = new_str;
    if (arith->operand2_ == old_var) arith->operand2_ = new_str;
    return;
  }
  if (auto *cmp = dynamic_cast<IRCompareInstructionNode *>(ins)) {
    if (cmp->operand1_ == old_var) cmp->operand1_ = new_str;
    if (cmp->operand2_ == old_var) cmp->operand2_ = new_str;
    return;
  }
  if (auto *br = dynamic_cast<IRBranchInstructionNode *>(ins)) {
    if (br->condition_ == old_var) br->condition_ = new_str;
    return;
  }
  if (auto *mv = dynamic_cast<IRMoveInstructionNode *>(ins)) {
    if (mv->source_ == old_var) mv->source_ = new_str;
    return;
  }
  if (auto *sel = dynamic_cast<IRSelectInstructionNode *>(ins)) {
    if (sel->cond_ == old_var) sel->cond_ = new_str;
    return;
  }
  if (auto *neg = dynamic_cast<IRNegationInstructionNode *>(ins)) {
    if (neg->operand_ == old_var) neg->operand_ = new_str;
    return;
  }
  if (auto *ret = dynamic_cast<IRReturnInstructionNode *>(ins)) {
    if (ret->name_ == old_var) ret->name_ = new_str;
    return;
  }
  if (auto *call = dynamic_cast<IRCallInstructionNode *>(ins)) {
    for (auto &arg : call->arguments_)
      if (arg->value_ == old_var) arg->value_ = new_str;
    return;
  }
  if (auto *load = dynamic_cast<IRLoadInstructionNode *>(ins)) {
    if (load->pointer_ == old_var) load->pointer_ = new_str;
    return;
  }
  if (auto *store_v = dynamic_cast<IRStoreVariableInstructionNode *>(ins)) {
    if (store_v->value_ == old_var) store_v->value_ = new_str;
    if (store_v->pointer_ == old_var) store_v->pointer_ = new_str;
    return;
  }
  if (auto *store_c = dynamic_cast<IRStoreConstInstructionNode *>(ins)) {
    if (store_c->pointer_ == old_var) store_c->pointer_ = new_str;
    return;
  }
  if (auto *gep = dynamic_cast<IRGetElementPtrInstructionNode *>(ins)) {
    if (gep->ptrval_ == old_var) gep->ptrval_ = new_str;
    return;
  }
  if (auto *gepp = dynamic_cast<IRGetElementPtrPrimeInstructionNode *>(ins)) {
    if (gepp->ptrval_ == old_var) gepp->ptrval_ = new_str;
    if (gepp->index_ == old_var) gepp->index_ = new_str;
    return;
  }
}

void SCCPer::ReplaceAllUses(
    std::vector<std::shared_ptr<IRBlockNode>> &blocks,
    const std::string &old_var, const std::string &new_str) {
  for (auto &blk : blocks) {
    for (auto &ins : blk->instructions_) {
      if (ins->removed_) continue;
      ReplaceInIns(ins.get(), old_var, new_str);
    }
    for (auto &phi : blk->phi_) {
      for (auto &pair : phi->val_) {
        if (pair.first == old_var) pair.first = new_str;
      }
    }
  }
}

// ─── Operand collection for use-map ──────────────────────────────────

void SCCPer::CollectOperandVars(IRInstructionNode *ins,
                                 std::vector<std::string> &out) {
  if (auto *a = dynamic_cast<IRArithmeticInstructionNode *>(ins)) {
    if (IsVariable(a->operand1_)) out.push_back(a->operand1_);
    if (IsVariable(a->operand2_)) out.push_back(a->operand2_);
  } else if (auto *c = dynamic_cast<IRCompareInstructionNode *>(ins)) {
    if (IsVariable(c->operand1_)) out.push_back(c->operand1_);
    if (IsVariable(c->operand2_)) out.push_back(c->operand2_);
  } else if (auto *b = dynamic_cast<IRBranchInstructionNode *>(ins)) {
    if (IsVariable(b->condition_)) out.push_back(b->condition_);
  } else if (auto *m = dynamic_cast<IRMoveInstructionNode *>(ins)) {
    if (IsVariable(m->source_)) out.push_back(m->source_);
  } else if (auto *s = dynamic_cast<IRSelectInstructionNode *>(ins)) {
    if (IsVariable(s->cond_)) out.push_back(s->cond_);
  } else if (auto *n = dynamic_cast<IRNegationInstructionNode *>(ins)) {
    if (IsVariable(n->operand_)) out.push_back(n->operand_);
  } else if (auto *r = dynamic_cast<IRReturnInstructionNode *>(ins)) {
    if (IsVariable(r->name_)) out.push_back(r->name_);
  } else if (auto *cl = dynamic_cast<IRCallInstructionNode *>(ins)) {
    for (auto &arg : cl->arguments_)
      if (IsVariable(arg->value_)) out.push_back(arg->value_);
  } else if (auto *l = dynamic_cast<IRLoadInstructionNode *>(ins)) {
    if (IsVariable(l->pointer_)) out.push_back(l->pointer_);
  } else if (auto *sv = dynamic_cast<IRStoreVariableInstructionNode *>(ins)) {
    if (IsVariable(sv->value_)) out.push_back(sv->value_);
    if (IsVariable(sv->pointer_)) out.push_back(sv->pointer_);
  } else if (auto *sc = dynamic_cast<IRStoreConstInstructionNode *>(ins)) {
    if (IsVariable(sc->pointer_)) out.push_back(sc->pointer_);
  } else if (auto *g = dynamic_cast<IRGetElementPtrInstructionNode *>(ins)) {
    if (IsVariable(g->ptrval_)) out.push_back(g->ptrval_);
  } else if (auto *gp = dynamic_cast<IRGetElementPtrPrimeInstructionNode *>(ins)) {
    if (IsVariable(gp->ptrval_)) out.push_back(gp->ptrval_);
    if (IsVariable(gp->index_)) out.push_back(gp->index_);
  }
}

void SCCPer::CollectPhiOperandVars(IRPhiInstructionNode *phi,
                                    std::vector<std::string> &out) {
  for (auto &pair : phi->val_)
    if (IsVariable(pair.first)) out.push_back(pair.first);
}

// ─── Per-function SCCP ─────────────────────────────────────────────────

void SCCPer::RunOnFunction(IRFunctionNode *func) {
  auto &blocks = func->blocks_;
  int N = int(blocks.size());
  if (N == 0) return;

  // --- Build CFG info ---
  std::vector<std::vector<uint32_t>> succs(N), preds(N);
  for (auto &blk : blocks) {
    uint32_t u = blk->GetID();
    for (auto &ins : blk->instructions_) {
      if (ins->removed_) continue;
      if (auto *br = dynamic_cast<IRBranchInstructionNode *>(ins.get())) {
        succs[u].push_back(br->true_branch_);
        succs[u].push_back(br->false_branch_);
        preds[br->true_branch_].push_back(u);
        preds[br->false_branch_].push_back(u);
        break;
      }
      if (auto *j = dynamic_cast<IRJumpInstructionNode *>(ins.get())) {
        succs[u].push_back(j->destination_);
        preds[j->destination_].push_back(u);
        break;
      }
    }
  }

  // --- Build use-map: var_name -> list of (block_id, ins*) that use it ---
  // Also track phi uses: var_name -> list of (block_id, phi*) that reference it
  struct UseEntry {
    uint32_t block_id;
    IRInstructionNode *ins;  // nullptr means this is a phi use
    IRPhiInstructionNode *phi;
  };
  std::unordered_map<std::string, std::vector<UseEntry>> use_map;
  for (auto &blk : blocks) {
    uint32_t bid = blk->GetID();
    for (auto &ins : blk->instructions_) {
      if (ins->removed_) continue;
      std::vector<std::string> vars;
      CollectOperandVars(ins.get(), vars);
      for (auto &v : vars)
        use_map[v].push_back({bid, ins.get(), nullptr});
    }
    for (auto &phi : blk->phi_) {
      std::vector<std::string> vars;
      CollectPhiOperandVars(phi.get(), vars);
      for (auto &v : vars)
        use_map[v].push_back({bid, nullptr, phi.get()});
    }
  }

  // --- Initialize lattice and worklists ---
  std::unordered_map<std::string, LatticeCell> lattice;
  std::vector<bool> executable(N, false);
  std::queue<uint32_t> flow_worklist;
  std::queue<std::pair<std::string, uint32_t>> ssa_worklist;

  executable[0] = true;
  flow_worklist.push(0);

  for (auto &param : func->parameters_) {
    lattice[param->name_] = {LatticeKind::kBottom, 0};
  }

  // --- Lambda: evaluate a phi ---
  auto EvalPhi = [&](IRPhiInstructionNode *phi, uint32_t blk_id) {
    LatticeCell result{LatticeKind::kTop, 0};
    bool first = true;
    for (auto &pair : phi->val_) {
      if (!executable[pair.second]) continue;
      if (first) {
        result = GetLattice(lattice, pair.first);
        first = false;
      } else {
        result = Meet(lattice, result, pair.first);
      }
    }
    auto &cur = lattice[phi->result_];
    if (result != cur) {
      cur = result;
      ssa_worklist.push({phi->result_, blk_id});
    }
  };

  // --- Lambda: evaluate an instruction ---
  auto EvalIns = [&](IRInstructionNode *ins, uint32_t blk_id) {
    // Arithmetic
    if (auto *node = dynamic_cast<IRArithmeticInstructionNode *>(ins)) {
      auto lhs = GetLattice(lattice, node->operand1_);
      auto rhs = GetLattice(lattice, node->operand2_);
      LatticeCell result;
      if (lhs.kind == LatticeKind::kBottom ||
          rhs.kind == LatticeKind::kBottom)
        result = {LatticeKind::kBottom, 0};
      else if (lhs.kind == LatticeKind::kConstant &&
               rhs.kind == LatticeKind::kConstant)
        result = {LatticeKind::kConstant,
                  FoldArithmetic(lhs.value, rhs.value, node->op_,
                                 node->is_unsigned_)};
      else
        result = {LatticeKind::kTop, 0};
      auto &cur = lattice[node->result_];
      if (result != cur) { cur = result; ssa_worklist.push({node->result_, blk_id}); }
      return;
    }
    // Negation
    if (auto *node = dynamic_cast<IRNegationInstructionNode *>(ins)) {
      auto op = GetLattice(lattice, node->operand_);
      LatticeCell result;
      if (op.kind == LatticeKind::kBottom)
        result = {LatticeKind::kBottom, 0};
      else if (op.kind == LatticeKind::kConstant)
        result = {LatticeKind::kConstant,
                  node->is_minus_ ? int32_t(uint32_t(0) - uint32_t(op.value))
                                  : int32_t(~uint32_t(op.value))};
      else
        result = {LatticeKind::kTop, 0};
      auto &cur = lattice[node->result_];
      if (result != cur) { cur = result; ssa_worklist.push({node->result_, blk_id}); }
      return;
    }
    // Compare
    if (auto *node = dynamic_cast<IRCompareInstructionNode *>(ins)) {
      auto lhs = GetLattice(lattice, node->operand1_);
      auto rhs = GetLattice(lattice, node->operand2_);
      LatticeCell result;
      if (lhs.kind == LatticeKind::kBottom ||
          rhs.kind == LatticeKind::kBottom)
        result = {LatticeKind::kBottom, 0};
      else if (lhs.kind == LatticeKind::kConstant &&
               rhs.kind == LatticeKind::kConstant)
        result = {LatticeKind::kConstant,
                  FoldCompare(lhs.value, rhs.value, node->op_) ? 1 : 0};
      else
        result = {LatticeKind::kTop, 0};
      auto &cur = lattice[node->result_];
      if (result != cur) { cur = result; ssa_worklist.push({node->result_, blk_id}); }
      return;
    }
    // Branch
    if (auto *node = dynamic_cast<IRBranchInstructionNode *>(ins)) {
      auto cond = GetLattice(lattice, node->condition_);
      if (cond.kind == LatticeKind::kConstant) {
        uint32_t taken = (cond.value != 0) ? node->true_branch_
                                           : node->false_branch_;
        if (!executable[taken]) { executable[taken] = true; flow_worklist.push(taken); }
      } else {
        for (uint32_t t : {node->true_branch_, node->false_branch_}) {
          bool was_exec = executable[t];
          if (!was_exec) { executable[t] = true; flow_worklist.push(t); }
          else {
            // New predecessor for already-executable block — re-evaluate phis
            for (auto &phi : blocks[t]->phi_) EvalPhi(phi.get(), t);
          }
        }
      }
      return;
    }
    // Jump
    if (auto *node = dynamic_cast<IRJumpInstructionNode *>(ins)) {
      bool was_exec = executable[node->destination_];
      if (!was_exec) {
        executable[node->destination_] = true;
        flow_worklist.push(node->destination_);
      } else {
        for (auto &phi : blocks[node->destination_]->phi_)
          EvalPhi(phi.get(), node->destination_);
      }
      return;
    }
    // Move
    if (auto *node = dynamic_cast<IRMoveInstructionNode *>(ins)) {
      auto src = GetLattice(lattice, node->source_);
      auto &cur = lattice[node->result_];
      if (src != cur) { cur = src; ssa_worklist.push({node->result_, blk_id}); }
      return;
    }
    // Select
    if (auto *node = dynamic_cast<IRSelectInstructionNode *>(ins)) {
      auto cond = GetLattice(lattice, node->cond_);
      LatticeCell result;
      if (cond.kind == LatticeKind::kBottom)
        result = {LatticeKind::kBottom, 0};
      else if (cond.kind == LatticeKind::kConstant)
        result = {LatticeKind::kConstant, cond.value};
      else
        result = {LatticeKind::kTop, 0};
      auto &cur = lattice[node->result_];
      if (result != cur) { cur = result; ssa_worklist.push({node->result_, blk_id}); }
      return;
    }
    // Call
    if (auto *node = dynamic_cast<IRCallInstructionNode *>(ins)) {
      if (!node->result_type_->IsEmpty()) {
        LatticeCell result{LatticeKind::kBottom, 0};
        auto &cur = lattice[node->result_];
        if (result != cur) { cur = result; ssa_worklist.push({node->result_, blk_id}); }
      }
      return;
    }
    // Load
    if (auto *node = dynamic_cast<IRLoadInstructionNode *>(ins)) {
      LatticeCell result{LatticeKind::kBottom, 0};
      auto &cur = lattice[node->result_];
      if (result != cur) { cur = result; ssa_worklist.push({node->result_, blk_id}); }
      return;
    }
    // Allocate
    if (auto *node = dynamic_cast<IRAllocateInstructionNode *>(ins)) {
      LatticeCell result{LatticeKind::kBottom, 0};
      auto &cur = lattice[node->result_];
      if (result != cur) { cur = result; ssa_worklist.push({node->result_, blk_id}); }
      return;
    }
    // GEP
    if (auto *node = dynamic_cast<IRGetElementPtrInstructionNode *>(ins)) {
      LatticeCell result{LatticeKind::kBottom, 0};
      auto &cur = lattice[node->result_];
      if (result != cur) { cur = result; ssa_worklist.push({node->result_, blk_id}); }
      return;
    }
    // GEP'
    if (auto *node = dynamic_cast<IRGetElementPtrPrimeInstructionNode *>(ins)) {
      LatticeCell result{LatticeKind::kBottom, 0};
      auto &cur = lattice[node->result_];
      if (result != cur) { cur = result; ssa_worklist.push({node->result_, blk_id}); }
      return;
    }
    // StoreVariable, StoreConst, Return, Argument: no result to track
  };

  // --- Fixpoint loop ---
  while (!flow_worklist.empty() || !ssa_worklist.empty()) {
    while (!flow_worklist.empty()) {
      uint32_t u = flow_worklist.front(); flow_worklist.pop();
      auto &blk = blocks[u];
      for (auto &phi : blk->phi_) EvalPhi(phi.get(), u);
      for (auto &ins : blk->instructions_) {
        if (ins->removed_) continue;
        EvalIns(ins.get(), u);
      }
    }
    while (!ssa_worklist.empty()) {
      auto [var, blk_id] = ssa_worklist.front(); ssa_worklist.pop();
      // Re-evaluate all uses of this variable (in any block)
      auto it = use_map.find(var);
      if (it != use_map.end()) {
        for (auto &use : it->second) {
          if (!executable[use.block_id]) continue;
          if (use.phi != nullptr) {
            // Phi use: re-evaluate the phi
            bool refs = false;
            for (auto &pair : use.phi->val_) {
              if (pair.first == var) { refs = true; break; }
            }
            if (refs) EvalPhi(use.phi, use.block_id);
          } else if (use.ins != nullptr && !use.ins->removed_) {
            EvalIns(use.ins, use.block_id);
          }
        }
      }
    }
  }

  // ═══════════════════════════════════════════════════════════════════
  // TRANSFORMATION
  // ═══════════════════════════════════════════════════════════════════

  // Step A: Replace variable operands with constant values
  for (auto &blk : blocks) {
    if (!executable[blk->GetID()]) continue;
    for (auto &ins : blk->instructions_) {
      if (ins->removed_) continue;
      // Arithmetic
      if (auto *a = dynamic_cast<IRArithmeticInstructionNode *>(ins.get())) {
        if (IsVariable(a->operand1_)) {
          auto it = lattice.find(a->operand1_);
          if (it != lattice.end() && it->second.kind == LatticeKind::kConstant)
            a->operand1_ = std::to_string(it->second.value);
        }
        if (IsVariable(a->operand2_)) {
          auto it = lattice.find(a->operand2_);
          if (it != lattice.end() && it->second.kind == LatticeKind::kConstant)
            a->operand2_ = std::to_string(it->second.value);
        }
        continue;
      }
      // Compare
      if (auto *c = dynamic_cast<IRCompareInstructionNode *>(ins.get())) {
        if (IsVariable(c->operand1_)) {
          auto it = lattice.find(c->operand1_);
          if (it != lattice.end() && it->second.kind == LatticeKind::kConstant)
            c->operand1_ = std::to_string(it->second.value);
        }
        if (IsVariable(c->operand2_)) {
          auto it = lattice.find(c->operand2_);
          if (it != lattice.end() && it->second.kind == LatticeKind::kConstant)
            c->operand2_ = std::to_string(it->second.value);
        }
        continue;
      }
      // Branch
      if (auto *b = dynamic_cast<IRBranchInstructionNode *>(ins.get())) {
        if (IsVariable(b->condition_)) {
          auto it = lattice.find(b->condition_);
          if (it != lattice.end() && it->second.kind == LatticeKind::kConstant)
            b->condition_ = std::to_string(it->second.value);
        }
        continue;
      }
      // Move
      if (auto *m = dynamic_cast<IRMoveInstructionNode *>(ins.get())) {
        if (IsVariable(m->source_)) {
          auto it = lattice.find(m->source_);
          if (it != lattice.end() && it->second.kind == LatticeKind::kConstant)
            m->source_ = std::to_string(it->second.value);
        }
        continue;
      }
      // Select
      if (auto *s = dynamic_cast<IRSelectInstructionNode *>(ins.get())) {
        if (IsVariable(s->cond_)) {
          auto it = lattice.find(s->cond_);
          if (it != lattice.end() && it->second.kind == LatticeKind::kConstant)
            s->cond_ = std::to_string(it->second.value);
        }
        continue;
      }
      // Negation
      if (auto *n = dynamic_cast<IRNegationInstructionNode *>(ins.get())) {
        if (IsVariable(n->operand_)) {
          auto it = lattice.find(n->operand_);
          if (it != lattice.end() && it->second.kind == LatticeKind::kConstant)
            n->operand_ = std::to_string(it->second.value);
        }
        continue;
      }
      // Return
      if (auto *r = dynamic_cast<IRReturnInstructionNode *>(ins.get())) {
        if (IsVariable(r->name_)) {
          auto it = lattice.find(r->name_);
          if (it != lattice.end() && it->second.kind == LatticeKind::kConstant)
            r->name_ = std::to_string(it->second.value);
        }
        continue;
      }
      // Call
      if (auto *cl = dynamic_cast<IRCallInstructionNode *>(ins.get())) {
        for (auto &arg : cl->arguments_) {
          if (IsVariable(arg->value_)) {
            auto it = lattice.find(arg->value_);
            if (it != lattice.end() && it->second.kind == LatticeKind::kConstant)
              arg->value_ = std::to_string(it->second.value);
          }
        }
        continue;
      }
      // Load
      if (auto *l = dynamic_cast<IRLoadInstructionNode *>(ins.get())) {
        if (IsVariable(l->pointer_)) {
          auto it = lattice.find(l->pointer_);
          if (it != lattice.end() && it->second.kind == LatticeKind::kConstant)
            l->pointer_ = std::to_string(it->second.value);
        }
        continue;
      }
      // StoreVariable
      if (auto *sv = dynamic_cast<IRStoreVariableInstructionNode *>(ins.get())) {
        if (IsVariable(sv->value_)) {
          auto it = lattice.find(sv->value_);
          if (it != lattice.end() && it->second.kind == LatticeKind::kConstant)
            sv->value_ = std::to_string(it->second.value);
        }
        if (IsVariable(sv->pointer_)) {
          auto it = lattice.find(sv->pointer_);
          if (it != lattice.end() && it->second.kind == LatticeKind::kConstant)
            sv->pointer_ = std::to_string(it->second.value);
        }
        continue;
      }
      // StoreConst
      if (auto *sc = dynamic_cast<IRStoreConstInstructionNode *>(ins.get())) {
        if (IsVariable(sc->pointer_)) {
          auto it = lattice.find(sc->pointer_);
          if (it != lattice.end() && it->second.kind == LatticeKind::kConstant)
            sc->pointer_ = std::to_string(it->second.value);
        }
        continue;
      }
      // GEP
      if (auto *g = dynamic_cast<IRGetElementPtrInstructionNode *>(ins.get())) {
        if (IsVariable(g->ptrval_)) {
          auto it = lattice.find(g->ptrval_);
          if (it != lattice.end() && it->second.kind == LatticeKind::kConstant)
            g->ptrval_ = std::to_string(it->second.value);
        }
        continue;
      }
      // GEP'
      if (auto *gp = dynamic_cast<IRGetElementPtrPrimeInstructionNode *>(ins.get())) {
        if (IsVariable(gp->ptrval_)) {
          auto it = lattice.find(gp->ptrval_);
          if (it != lattice.end() && it->second.kind == LatticeKind::kConstant)
            gp->ptrval_ = std::to_string(it->second.value);
        }
        if (IsVariable(gp->index_)) {
          auto it = lattice.find(gp->index_);
          if (it != lattice.end() && it->second.kind == LatticeKind::kConstant)
            gp->index_ = std::to_string(it->second.value);
        }
        continue;
      }
    }
    // Replace phi operand values
    for (auto &phi : blk->phi_) {
      for (auto &pair : phi->val_) {
        if (IsVariable(pair.first)) {
          auto it = lattice.find(pair.first);
          if (it != lattice.end() && it->second.kind == LatticeKind::kConstant)
            pair.first = std::to_string(it->second.value);
        }
      }
    }
  }

  // Step B: Fold all-constant instructions (iterate to fixpoint)
  bool changed;
  do {
    changed = false;
    for (auto &blk : blocks) {
      if (!executable[blk->GetID()]) continue;
      for (auto &ins : blk->instructions_) {
        if (ins->removed_) continue;
        if (auto *a = dynamic_cast<IRArithmeticInstructionNode *>(ins.get())) {
          if (IsConstantString(a->operand1_) && IsConstantString(a->operand2_)) {
            int32_t x = ParseConstant(a->operand1_);
            int32_t y = ParseConstant(a->operand2_);
            std::string rs = std::to_string(
                FoldArithmetic(x, y, a->op_, a->is_unsigned_));
            ReplaceAllUses(blocks, a->result_, rs);
            a->removed_ = true;
            changed = true;
          }
          continue;
        }
        if (auto *c = dynamic_cast<IRCompareInstructionNode *>(ins.get())) {
          if (IsConstantString(c->operand1_) && IsConstantString(c->operand2_)) {
            int32_t x = ParseConstant(c->operand1_);
            int32_t y = ParseConstant(c->operand2_);
            std::string rs = FoldCompare(x, y, c->op_) ? "1" : "0";
            ReplaceAllUses(blocks, c->result_, rs);
            c->removed_ = true;
            changed = true;
          }
          continue;
        }
        if (auto *n = dynamic_cast<IRNegationInstructionNode *>(ins.get())) {
          if (IsConstantString(n->operand_)) {
            int32_t op = ParseConstant(n->operand_);
            int32_t v = n->is_minus_ ? int32_t(uint32_t(0) - uint32_t(op))
                                     : int32_t(~uint32_t(op));
            std::string rs = std::to_string(v);
            ReplaceAllUses(blocks, n->result_, rs);
            n->removed_ = true;
            changed = true;
          }
          continue;
        }
        if (auto *s = dynamic_cast<IRSelectInstructionNode *>(ins.get())) {
          if (IsConstantString(s->cond_)) {
            ReplaceAllUses(blocks, s->result_, s->cond_);
            s->removed_ = true;
            changed = true;
          }
          continue;
        }
      }
    }
  } while (changed);

  // Step C: Convert constant-condition branches to jumps
  for (auto &blk : blocks) {
    if (!executable[blk->GetID()]) continue;
    for (auto &ins : blk->instructions_) {
      if (ins->removed_) continue;
      if (auto *br = dynamic_cast<IRBranchInstructionNode *>(ins.get())) {
        if (IsConstantString(br->condition_)) {
          int32_t v = ParseConstant(br->condition_);
          uint32_t taken = (v != 0) ? br->true_branch_ : br->false_branch_;
          br->removed_ = true;
          blk->instructions_.push_back(
              std::make_shared<IRJumpInstructionNode>(taken));
          break;
        }
      }
    }
  }

  // Step D: Mark dead-block instructions removed, and remove phi operands
  // from successor blocks that reference dead predecessors
  for (auto &blk : blocks) {
    if (executable[blk->GetID()]) continue;
    uint32_t dead_id = blk->GetID();
    for (auto &ins : blk->instructions_) ins->removed_ = true;
    blk->phi_.clear();
    // Remove phi operands in successors that reference this dead block
    for (uint32_t s : succs[dead_id]) {
      for (auto &phi : blocks[s]->phi_) {
        auto &vals = phi->val_;
        vals.erase(std::remove_if(vals.begin(), vals.end(),
            [dead_id](const std::pair<std::string, uint32_t> &p) {
              return p.second == dead_id;
            }), vals.end());
      }
    }
  }
}

// ─── Top-level entry point ─────────────────────────────────────────────

void SCCPer::Run(std::shared_ptr<IRRootNode> root) {
  for (auto &func : root->functions_) {
    if (func->IsBuiltin()) continue;
    RunOnFunction(func.get());
  }
}

void SCCP(std::shared_ptr<IRRootNode> root) {
  SCCPer().Run(root);
}
