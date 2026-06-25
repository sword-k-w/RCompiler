#include "IR_visitor/cse/cse.h"

#include <string>
#include <unordered_map>

// ─── CSEr: friended class that does all the work ───────────────────────

class CSEr {
 public:
  void Run(std::shared_ptr<IRRootNode> root);

 private:
  // Generate a type key from an IRArrayNode for GEP identity comparison.
  static std::string TypeKey(IRArrayNode *type);

  // Replace old_var with new_str in a single instruction.
  static void ReplaceInIns(IRInstructionNode *ins, const std::string &old_var,
                            const std::string &new_str);

  // Get the result name of an instruction.
  static std::string GetResultName(IRInstructionNode *ins);

  // Run CSE on one function (local value numbering per basic block).
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

// ─── ReplaceInIns ─────────────────────────────────────────────────────

void CSEr::ReplaceInIns(IRInstructionNode *ins, const std::string &old_var,
                         const std::string &new_str) {
  if (auto *a = dynamic_cast<IRArithmeticInstructionNode *>(ins)) {
    if (a->operand1_ == old_var) a->operand1_ = new_str;
    if (a->operand2_ == old_var) a->operand2_ = new_str;
    return;
  }
  if (auto *c = dynamic_cast<IRCompareInstructionNode *>(ins)) {
    if (c->operand1_ == old_var) c->operand1_ = new_str;
    if (c->operand2_ == old_var) c->operand2_ = new_str;
    return;
  }
  if (auto *b = dynamic_cast<IRBranchInstructionNode *>(ins)) {
    if (b->condition_ == old_var) b->condition_ = new_str;
    return;
  }
  if (auto *m = dynamic_cast<IRMoveInstructionNode *>(ins)) {
    if (m->source_ == old_var) m->source_ = new_str;
    return;
  }
  if (auto *s = dynamic_cast<IRSelectInstructionNode *>(ins)) {
    if (s->cond_ == old_var) s->cond_ = new_str;
    return;
  }
  if (auto *n = dynamic_cast<IRNegationInstructionNode *>(ins)) {
    if (n->operand_ == old_var) n->operand_ = new_str;
    return;
  }
  if (auto *r = dynamic_cast<IRReturnInstructionNode *>(ins)) {
    if (r->name_ == old_var) r->name_ = new_str;
    return;
  }
  if (auto *cl = dynamic_cast<IRCallInstructionNode *>(ins)) {
    for (auto &arg : cl->arguments_)
      if (arg->value_ == old_var) arg->value_ = new_str;
    return;
  }
  if (auto *l = dynamic_cast<IRLoadInstructionNode *>(ins)) {
    if (l->pointer_ == old_var) l->pointer_ = new_str;
    return;
  }
  if (auto *sv = dynamic_cast<IRStoreVariableInstructionNode *>(ins)) {
    if (sv->value_ == old_var) sv->value_ = new_str;
    if (sv->pointer_ == old_var) sv->pointer_ = new_str;
    return;
  }
  if (auto *sc = dynamic_cast<IRStoreConstInstructionNode *>(ins)) {
    if (sc->pointer_ == old_var) sc->pointer_ = new_str;
    return;
  }
  if (auto *g = dynamic_cast<IRGetElementPtrInstructionNode *>(ins)) {
    if (g->ptrval_ == old_var) g->ptrval_ = new_str;
    return;
  }
  if (auto *gp = dynamic_cast<IRGetElementPtrPrimeInstructionNode *>(ins)) {
    if (gp->ptrval_ == old_var) gp->ptrval_ = new_str;
    if (gp->index_ == old_var) gp->index_ = new_str;
    return;
  }
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

// ─── RunOnFunction ────────────────────────────────────────────────────

void CSEr::RunOnFunction(IRFunctionNode *func) {
  auto &blocks = func->blocks_;

  for (auto &blk : blocks) {
    // Map: operation key -> first result name that computed it
    std::unordered_map<std::string, std::string> available;

    for (auto &ins : blk->instructions_) {
      if (ins->removed_) continue;

      std::string key;

      // ── Build key for each pure instruction type ──

      if (auto *gep = dynamic_cast<IRGetElementPtrInstructionNode *>(ins.get())) {
        key = "gep|" + TypeKey(gep->type_.get()) + "|" + gep->ptrval_ +
              "|" + std::to_string(gep->index_);
      } else if (auto *gepp =
                     dynamic_cast<IRGetElementPtrPrimeInstructionNode *>(ins.get())) {
        key = "gepp|" + TypeKey(gepp->type_.get()) + "|" + gepp->ptrval_ +
              "|" + gepp->index_;
      } else if (auto *a = dynamic_cast<IRArithmeticInstructionNode *>(ins.get())) {
        key = "arith|" + a->op_ + "|" + a->type_ + "|" + a->operand1_ +
              "|" + a->operand2_ + "|" + (a->is_unsigned_ ? "u" : "s");
      } else if (auto *c = dynamic_cast<IRCompareInstructionNode *>(ins.get())) {
        key = "cmp|" + std::to_string(static_cast<int>(c->op_)) + "|" +
              c->type_ + "|" + c->operand1_ + "|" + c->operand2_;
      } else if (auto *n = dynamic_cast<IRNegationInstructionNode *>(ins.get())) {
        key = "neg|" + std::to_string(n->is_minus_) + "|" + n->type_ +
              "|" + n->operand_;
      } else {
        // Side-effecting or non-pure instruction — skip.
        continue;
      }

      // ── Check if this operation already exists ──
      auto it = available.find(key);
      if (it != available.end()) {
        // Duplicate found! Replace all uses with the earlier result.
        const std::string &earlier = it->second;
        std::string old_result = GetResultName(ins.get());

        // Replace in all blocks
        for (auto &other_blk : blocks) {
          for (auto &other_ins : other_blk->instructions_) {
            if (other_ins->removed_) continue;
            ReplaceInIns(other_ins.get(), old_result, earlier);
          }
          for (auto &phi : other_blk->phi_) {
            for (auto &pair : phi->val_) {
              if (pair.first == old_result) pair.first = earlier;
            }
          }
        }
        ins->removed_ = true;
      } else {
        // First occurrence — record it
        available[key] = GetResultName(ins.get());
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
