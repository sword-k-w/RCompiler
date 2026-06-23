#include "IR_visitor/preprocessor/preprocessor.h"
#include "IR/struct_map.h"
#include "IR/IR_node.h"
#include "IR_visitor/memory_allocator/memory_allocator.h"
#include <iostream>

void Preprocessor::Visit(IRArrayNode *node) {
  if (node->IsEmpty()) {
    return;
  }
  if (node->base_type_ == "i32" || node->base_type_ == "ptr") {
    node->align_ = 8;
    node->allocated_size_ = 8;
    for (auto &length : node->length_) {
      node->allocated_size_ *= length;
    }
  } else if (node->base_type_ == "i1") {
    node->align_ = 1;
    node->allocated_size_ = 1;
    for (auto &length: node->length_) {
      node->allocated_size_ *= length;
    }
  } else {
    auto struct_node = StructMap::Instance().Query(node->base_type_);
    node->align_ = struct_node->align_;
    node->allocated_size_ = struct_node->allocated_size_;
    for (auto &length: node->length_) {
      node->allocated_size_ *= length;
    }
  }
}

void Preprocessor::Visit(IRStructNode *node) {
  node->align_ = 1;
  node->allocated_size_ = 0;
  for (auto &array_node : node->members_) {
    array_node->Accept(this);
    auto inner_size = array_node->allocated_size_;
    auto inner_align = array_node->align_;
    if (inner_align == 1 && node->align_ == 8) {
      inner_size = Align8(inner_size);
    }
    if (node->align_ == 1 && inner_align == 8) {
      node->allocated_size_ = Align8(node->allocated_size_);
      node->align_ = 8;
    }
    node->allocated_size_ += inner_size;
  }
}

void Preprocessor::Visit(IRArithmeticInstructionNode *node) {
  (*current_variables_)[node->result_] = node;
}

void Preprocessor::Visit(IRNegationInstructionNode *node) {
  (*current_variables_)[node->result_] = node;
}

void Preprocessor::Visit(IRBranchInstructionNode *node) {}

void Preprocessor::Visit(IRJumpInstructionNode *node) {}

void Preprocessor::Visit(IRReturnInstructionNode *node) {
  if (!node->type_->IsEmpty()) {
    node->type_->Accept(this);
  }
}

void Preprocessor::Visit(IRAllocateInstructionNode *node) {
  node->type_->Accept(this);
  (*current_variables_)[node->result_] = node;
}

void Preprocessor::Visit(IRLoadInstructionNode *node) {
  node->type_->Accept(this);
  (*current_variables_)[node->result_] = node;
}

void Preprocessor::Visit(IRStoreVariableInstructionNode *node) {
  node->type_->Accept(this);
}

void Preprocessor::Visit(IRStoreConstInstructionNode *node) {}

void Preprocessor::Visit(IRGetElementPtrInstructionNode *node) {
  node->type_->Accept(this);
  (*current_variables_)[node->result_] = node;
}

void Preprocessor::Visit(IRGetElementPtrPrimeInstructionNode *node) {
  node->type_->Accept(this);
  (*current_variables_)[node->result_] = node;
}

void Preprocessor::Visit(IRCompareInstructionNode *node) {
  (*current_variables_)[node->result_] = node;
}

void Preprocessor::Visit(IRArgumentNode *node) {
  node->type_->Accept(this);
}

void Preprocessor::Visit(IRCallInstructionNode *node) {
  if (node->result_type_ != nullptr) {
    node->result_type_->Accept(this);
    (*current_variables_)[node->result_] = node;
  }
  for (auto &argument : node->arguments_) {
    argument->Accept(this);
  }
}

void Preprocessor::Visit(IRPhiInstructionNode *node) {}

void Preprocessor::Visit(IRMoveInstructionNode *node) {
  node->type_->Accept(this);
  (*current_variables_)[node->result_] = node;
}

void Preprocessor::Visit(IRSelectInstructionNode *node) {
  (*current_variables_)[node->result_] = node;
}

void Preprocessor::Visit(IRBlockNode *node) {
  for (auto &instruction : node->instructions_) {
    if (instruction->removed_) {
      continue;
    }
    instruction->Accept(this);
  }
}

void Preprocessor::Visit(IRParameterNode *node) {
  node->type_->Accept(this);
  (*current_variables_)[node->name_] = node;
}

void Preprocessor::Visit(IRFunctionNode *node) {
  if (!node->type_->IsEmpty()) {
    node->type_->Accept(this);
  }
  current_variables_ = &node->variables_;
  for (auto parameter : node->parameters_) {
    parameter->Accept(this);
  }
  for (auto block : node->blocks_) {
    block->Accept(this);
  }
}

void Preprocessor::Visit(IRRootNode *node) {
  for (auto &struct_node : node->structs_) {
    struct_node->Accept(this);
  }
  for (auto &function_node : node->functions_) {
    function_node->Accept(this);
  }
}

void Preprocessor::ReplaceVarInIns(IRInstructionNode *ins, const std::string &old_var,
                                    const std::string &new_var) {
  if (auto *arith = dynamic_cast<IRArithmeticInstructionNode *>(ins)) {
    if (arith->operand1_ == old_var) arith->operand1_ = new_var;
    if (arith->operand2_ == old_var) arith->operand2_ = new_var;
  } else if (auto *cmp = dynamic_cast<IRCompareInstructionNode *>(ins)) {
    if (cmp->operand1_ == old_var) cmp->operand1_ = new_var;
    if (cmp->operand2_ == old_var) cmp->operand2_ = new_var;
  } else if (auto *neg = dynamic_cast<IRNegationInstructionNode *>(ins)) {
    if (neg->operand_ == old_var) neg->operand_ = new_var;
  } else if (auto *branch = dynamic_cast<IRBranchInstructionNode *>(ins)) {
    if (branch->condition_ == old_var) branch->condition_ = new_var;
  } else if (auto *ret = dynamic_cast<IRReturnInstructionNode *>(ins)) {
    if (ret->name_ == old_var) ret->name_ = new_var;
  } else if (auto *load = dynamic_cast<IRLoadInstructionNode *>(ins)) {
    if (load->pointer_ == old_var) load->pointer_ = new_var;
  } else if (auto *store_v = dynamic_cast<IRStoreVariableInstructionNode *>(ins)) {
    if (store_v->pointer_ == old_var) store_v->pointer_ = new_var;
    if (store_v->value_ == old_var) store_v->value_ = new_var;
  } else if (auto *store_c = dynamic_cast<IRStoreConstInstructionNode *>(ins)) {
    if (store_c->pointer_ == old_var) store_c->pointer_ = new_var;
  } else if (auto *gep = dynamic_cast<IRGetElementPtrInstructionNode *>(ins)) {
    if (gep->ptrval_ == old_var) gep->ptrval_ = new_var;
  } else if (auto *gepp = dynamic_cast<IRGetElementPtrPrimeInstructionNode *>(ins)) {
    if (gepp->ptrval_ == old_var) gepp->ptrval_ = new_var;
    if (gepp->index_ == old_var) gepp->index_ = new_var;
  } else if (auto *mv = dynamic_cast<IRMoveInstructionNode *>(ins)) {
    if (mv->source_ == old_var) mv->source_ = new_var;
  } else if (auto *sel = dynamic_cast<IRSelectInstructionNode *>(ins)) {
    if (sel->cond_ == old_var) sel->cond_ = new_var;
  } else if (auto *call = dynamic_cast<IRCallInstructionNode *>(ins)) {
    for (auto &arg : call->arguments_) {
      if (arg->value_ == old_var) arg->value_ = new_var;
    }
  }
}

void Preprocessor::FoldZeroOffsetGEPs(std::shared_ptr<IRRootNode> IR_root) {
  // A GEP(const) with index_ == 0 always has byte offset 0:
  //   - Struct field 0 is always at offset 0 (first member).
  //   - Array element 0 has offset 0 * elem_size = 0.
  // Replace all uses of the result with the ptrval and remove the GEP,
  // eliminating the intermediate variable entirely.
  for (auto &func : IR_root->functions_) {
    for (auto &block : func->blocks_) {
      for (auto &ins : block->instructions_) {
        if (ins->removed_) continue;
        auto *gep = dynamic_cast<IRGetElementPtrInstructionNode *>(ins.get());
        if (!gep || gep->index_ != 0) continue;

        std::string old_var = gep->result_;
        std::string new_var = gep->ptrval_;

        // Replace all uses of old_var with new_var across all blocks.
        for (auto &other_block : func->blocks_) {
          for (auto &other : other_block->instructions_) {
            if (other->removed_) continue;
            ReplaceVarInIns(other.get(), old_var, new_var);
          }
        }

        // Remove the GEP and its variable entry.
        ins->removed_ = true;
        auto var_it = func->variables_.find(old_var);
        if (var_it != func->variables_.end()) {
          func->variables_.erase(var_it);
        }
      }
    }
  }
}
