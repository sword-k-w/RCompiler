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

  // Get the result name of an instruction.
  static std::string GetResultName(IRInstructionNode *ins);

  // Run CSE on one function.
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

// ─── RunOnFunction ────────────────────────────────────────────────────

void CSEr::RunOnFunction(IRFunctionNode *func) {
  auto &blocks = func->blocks_;

  // Global rename map: old_name -> canonical_name.
  // Accumulated across blocks so replacements found in one block propagate
  // to all other blocks.
  std::unordered_map<std::string, std::string> renames;

  // Resolve a name through the rename chain to its canonical form.
  // Includes path compression so repeated lookups remain amortized O(1).
  auto resolve = [&](const std::string &name) -> std::string {
    auto it = renames.find(name);
    if (it == renames.end()) return name;
    std::string canonical = it->second;
    while (true) {
      auto next = renames.find(canonical);
      if (next == renames.end()) break;
      canonical = next->second;
    }
    if (canonical != it->second) it->second = canonical;
    return canonical;
  };

  // ── Pass 1: find duplicates, build rename map ──
  for (auto &blk : blocks) {
    // Available expressions within this block (key -> canonical result name).
    std::unordered_map<std::string, std::string> available;

    for (auto &ins : blk->instructions_) {
      if (ins->removed_) continue;

      std::string key;

      // Build key with resolved operands so that earlier renames
      // (from this block or previous blocks) are reflected.
      if (auto *gep = dynamic_cast<IRGetElementPtrInstructionNode *>(ins.get())) {
        key = "gep|" + TypeKey(gep->type_.get()) + "|" +
              resolve(gep->ptrval_) + "|" + std::to_string(gep->index_);
      } else if (auto *gepp =
                     dynamic_cast<IRGetElementPtrPrimeInstructionNode *>(ins.get())) {
        key = "gepp|" + TypeKey(gepp->type_.get()) + "|" +
              resolve(gepp->ptrval_) + "|" + resolve(gepp->index_);
      } else if (auto *a = dynamic_cast<IRArithmeticInstructionNode *>(ins.get())) {
        key = "arith|" + a->op_ + "|" + a->type_ + "|" +
              resolve(a->operand1_) + "|" + resolve(a->operand2_) + "|" +
              (a->is_unsigned_ ? "u" : "s");
      } else if (auto *c = dynamic_cast<IRCompareInstructionNode *>(ins.get())) {
        key = "cmp|" + std::to_string(static_cast<int>(c->op_)) + "|" +
              c->type_ + "|" + resolve(c->operand1_) + "|" + resolve(c->operand2_);
      } else if (auto *n = dynamic_cast<IRNegationInstructionNode *>(ins.get())) {
        key = "neg|" + std::to_string(n->is_minus_) + "|" + n->type_ +
              "|" + resolve(n->operand_);
      } else {
        // Side-effecting or non-pure instruction — skip.
        continue;
      }

      auto it = available.find(key);
      if (it != available.end()) {
        // Duplicate found — record the rename.
        std::string old_result = GetResultName(ins.get());
        renames[old_result] = resolve(it->second);
        ins->removed_ = true;
      } else {
        available[key] = GetResultName(ins.get());
      }
    }
  }

  // ── Pass 2: apply all renames in a single sweep over all blocks ──
  if (renames.empty()) return;

  auto apply = [&](std::string &s) {
    auto it = renames.find(s);
    if (it != renames.end()) s = it->second;
  };

  for (auto &blk : blocks) {
    for (auto &ins : blk->instructions_) {
      if (ins->removed_) continue;

      if (auto *a = dynamic_cast<IRArithmeticInstructionNode *>(ins.get())) {
        apply(a->operand1_); apply(a->operand2_);
      } else if (auto *c = dynamic_cast<IRCompareInstructionNode *>(ins.get())) {
        apply(c->operand1_); apply(c->operand2_);
      } else if (auto *b = dynamic_cast<IRBranchInstructionNode *>(ins.get())) {
        apply(b->condition_);
      } else if (auto *m = dynamic_cast<IRMoveInstructionNode *>(ins.get())) {
        apply(m->source_);
      } else if (auto *s = dynamic_cast<IRSelectInstructionNode *>(ins.get())) {
        apply(s->cond_);
      } else if (auto *n = dynamic_cast<IRNegationInstructionNode *>(ins.get())) {
        apply(n->operand_);
      } else if (auto *r = dynamic_cast<IRReturnInstructionNode *>(ins.get())) {
        apply(r->name_);
      } else if (auto *cl = dynamic_cast<IRCallInstructionNode *>(ins.get())) {
        for (auto &arg : cl->arguments_) apply(arg->value_);
      } else if (auto *l = dynamic_cast<IRLoadInstructionNode *>(ins.get())) {
        apply(l->pointer_);
      } else if (auto *sv = dynamic_cast<IRStoreVariableInstructionNode *>(ins.get())) {
        apply(sv->value_); apply(sv->pointer_);
      } else if (auto *sc = dynamic_cast<IRStoreConstInstructionNode *>(ins.get())) {
        apply(sc->pointer_);
      } else if (auto *g = dynamic_cast<IRGetElementPtrInstructionNode *>(ins.get())) {
        apply(g->ptrval_);
      } else if (auto *gp = dynamic_cast<IRGetElementPtrPrimeInstructionNode *>(ins.get())) {
        apply(gp->ptrval_); apply(gp->index_);
      }
    }
    for (auto &phi : blk->phi_) {
      for (auto &pair : phi->val_) {
        apply(pair.first);
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
