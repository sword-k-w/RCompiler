#include "IR_visitor/parameter_demoter/parameter_demoter.h"
#include "IR/IR_node.h"
#include <unordered_map>
#include <unordered_set>

void ParameterDemoter::RenameInIns(IRInstructionNode *ins,
                                   const std::string &from,
                                   const std::string &to) {
  auto apply = [&](std::string &s) {
    if (s == from) s = to;
  };

  if (auto *a = dynamic_cast<IRArithmeticInstructionNode *>(ins)) {
    apply(a->operand1_); apply(a->operand2_);
  } else if (auto *c = dynamic_cast<IRCompareInstructionNode *>(ins)) {
    apply(c->operand1_); apply(c->operand2_);
  } else if (auto *b = dynamic_cast<IRBranchInstructionNode *>(ins)) {
    apply(b->condition_);
  } else if (auto *m = dynamic_cast<IRMoveInstructionNode *>(ins)) {
    apply(m->source_);
    // result_ is the def — don't rename it (it IS the new name)
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

void ParameterDemoter::RunOnFunction(IRFunctionNode *func) {
  if (func->IsBuiltin()) return;

  // Collect parameters that are in registers (a0-a7) and need demotion.
  struct ParamInfo {
    IRParameterNode *param;
    std::string demoted_name;
  };
  std::vector<ParamInfo> params_to_demote;

  for (auto &param : func->parameters_) {
    if (param->storage_type_ == kRegister) {
      std::string demoted_name = param->name_ + ".mv";
      params_to_demote.push_back({param.get(), demoted_name});
    }
  }

  if (params_to_demote.empty()) return;

  auto &entry_block = func->blocks_[0];
  auto &insts = entry_block->instructions_;

  // Track the move instructions we create so we don't rename their
  // source operand (they intentionally reference the a-reg parameter).
  std::unordered_set<IRInstructionNode *> demotion_moves;

  // Create and insert moves at the front of the entry block.
  // Insert in reverse order so they appear in parameter order at the front.
  for (auto it = params_to_demote.rbegin(); it != params_to_demote.rend(); ++it) {
    auto move = std::make_shared<IRMoveInstructionNode>(
        it->demoted_name,       // result
        it->param->name_,       // source (the a-reg parameter)
        it->param->type_        // type
    );
    move->storage_type_ = kMemory;  // Will be promoted by RegAlloc
    move->address_ = 0;

    insts.push_front(move);
    demotion_moves.insert(move.get());

    // Register the new variable in the function's maps.
    func->variables_[it->demoted_name] = move.get();
    func->variable_storage_[it->demoted_name] = {kMemory, 0};
    // Copy size from the original parameter.  Fall back to 8 bytes
    // (aligned max scalar size) if the parameter has no recorded size,
    // so the RegAlloc address assignment doesn't skip this variable.
    auto size_it = func->variable_size_.find(it->param->name_);
    uint32_t sz = (size_it != func->variable_size_.end()) ? size_it->second : 8;
    func->variable_size_[it->demoted_name] = sz;
  }

  // Rename all uses of the original parameter names to the demoted names
  // across all instructions in all blocks.  Skip the demotion moves
  // themselves — their source operand intentionally references the a-reg
  // parameter.
  for (auto &block : func->blocks_) {
    for (auto &ins : block->instructions_) {
      if (ins->removed_) continue;
      if (demotion_moves.count(ins.get())) continue;
      for (auto &p : params_to_demote) {
        RenameInIns(ins.get(), p.param->name_, p.demoted_name);
      }
    }
  }
}

void ParameterDemoter::Run(std::shared_ptr<IRRootNode> root) {
  for (auto &func : root->functions_) {
    RunOnFunction(func.get());
  }
}

void ParameterDemoter::FixupAfterRegAlloc(std::shared_ptr<IRRootNode> root) {
  // After RegAlloc, sync move instruction destination addresses with
  // variable_storage_.  The move instructions created by parameter
  // demotion have address_=0 (top of frame), but RegAlloc step 6
  // assigns proper stack addresses for spilled variables.  Sync them.
  for (auto &func : root->functions_) {
    if (func->IsBuiltin()) continue;
    for (auto &block : func->blocks_) {
      for (auto &ins : block->instructions_) {
        if (ins->removed_) continue;
        auto *mv = dynamic_cast<IRMoveInstructionNode *>(ins.get());
        if (!mv) continue;
        if (mv->storage_type_ != kMemory) continue;  // promoted, fine
        if (mv->address_ != 0) continue;  // already has a proper address

        auto it = func->variable_storage_.find(mv->result_);
        if (it != func->variable_storage_.end() && it->second.first == kMemory) {
          mv->address_ = it->second.second;
        }
      }
    }
  }
}
