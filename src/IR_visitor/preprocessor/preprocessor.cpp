#include "IR_visitor/preprocessor/preprocessor.h"
#include "IR/struct_map.h"
#include "IR/IR_node.h"
#include "IR_visitor/memory_allocator/memory_allocator.h"
#include <iostream>
#include <unordered_set>
#include <vector>

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
  for (auto &func : IR_root->functions_) {
    // --- Pass 1: count uses and record GEP(const) defs + offsets ---
    std::unordered_map<std::string, IRGetElementPtrInstructionNode *> def_map;
    std::unordered_map<std::string, uint32_t> gep_offset;
    std::unordered_map<std::string, uint32_t> use_count;

    auto count_use = [&](const std::string &var) {
      if (!var.empty() && var[0] == '%') use_count[var]++;
    };

    for (auto &block : func->blocks_) {
      for (auto &ins : block->instructions_) {
        if (ins->removed_) continue;
        if (auto *gep = dynamic_cast<IRGetElementPtrInstructionNode *>(ins.get())) {
          count_use(gep->ptrval_);
          def_map[gep->result_] = gep;
        } else if (auto *gepp = dynamic_cast<IRGetElementPtrPrimeInstructionNode *>(ins.get())) {
          count_use(gepp->ptrval_);
          count_use(gepp->index_);
        } else if (auto *load = dynamic_cast<IRLoadInstructionNode *>(ins.get())) {
          count_use(load->pointer_);
        } else if (auto *store_v = dynamic_cast<IRStoreVariableInstructionNode *>(ins.get())) {
          count_use(store_v->pointer_);
        } else if (auto *store_c = dynamic_cast<IRStoreConstInstructionNode *>(ins.get())) {
          count_use(store_c->pointer_);
        } else if (auto *mv = dynamic_cast<IRMoveInstructionNode *>(ins.get())) {
          count_use(mv->source_);
        } else if (auto *arith = dynamic_cast<IRArithmeticInstructionNode *>(ins.get())) {
          count_use(arith->operand1_);
          count_use(arith->operand2_);
        } else if (auto *cmp = dynamic_cast<IRCompareInstructionNode *>(ins.get())) {
          count_use(cmp->operand1_);
          count_use(cmp->operand2_);
        } else if (auto *neg = dynamic_cast<IRNegationInstructionNode *>(ins.get())) {
          count_use(neg->operand_);
        } else if (auto *branch = dynamic_cast<IRBranchInstructionNode *>(ins.get())) {
          count_use(branch->condition_);
        } else if (auto *ret = dynamic_cast<IRReturnInstructionNode *>(ins.get())) {
          count_use(ret->name_);
        } else if (auto *sel = dynamic_cast<IRSelectInstructionNode *>(ins.get())) {
          count_use(sel->cond_);
        } else if (auto *call = dynamic_cast<IRCallInstructionNode *>(ins.get())) {
          for (auto &arg : call->arguments_) count_use(arg->value_);
        }
      }
    }

    // Compute offsets for all GEP(const) instructions (now that type sizes
    // are available from preprocessing).  Do this in a separate loop over
    // the shared_ptrs in the deque so we have stable pointers.
    for (auto &block : func->blocks_) {
      for (auto &ins : block->instructions_) {
        if (ins->removed_) continue;
        auto *gep = dynamic_cast<IRGetElementPtrInstructionNode *>(ins.get());
        if (!gep) continue;
        // Compute the byte offset: struct field offset or array elem * index.
        uint32_t offset = 0;
        if (gep->type_->length_.empty()) {
          uint32_t align = 1;
          auto *sn = StructMap::Instance().Query(gep->type_->base_type_);
          for (uint32_t i = 0; i < gep->index_; ++i) {
            if (align == 1 && sn->members_[i]->align_ == 8) {
              align = 8;
              offset = Align8(offset);
            }
            auto ms = sn->members_[i]->allocated_size_;
            if (align == 8 && sn->members_[i]->align_ == 1)
              ms = Align8(ms);
            offset += ms;
          }
          if (sn->members_[gep->index_]->align_ == 8)
            offset = Align8(offset);
        } else {
          offset = gep->type_->allocated_size_ / gep->type_->length_[0]
                 * gep->index_;
        }
        gep_offset[gep->result_] = offset;
      }
    }

    // --- Pass 2: collect fold decisions ---
    // Only "true terminals" initiate a fold: GEP'(var), Load, Store,
    // StoreConst, and GEP(const) with use_count > 1 (used outside the
    // GEP chain).  GEP(const) with use_count == 1 are pure intermediates
    // that only the true terminal walks through.
    // Process blocks in reverse instruction order so later instructions
    // consume intermediates first.
    struct Fold {
      IRInstructionNode *terminal;
      std::string *ptr_field;
      std::vector<IRGetElementPtrInstructionNode *> intermediates;
      std::string ultimate_base;
      int32_t accumulated;
    };
    std::vector<Fold> folds;
    std::unordered_set<IRGetElementPtrInstructionNode *> claimed;

    for (auto &block : func->blocks_) {
      // Reverse iteration: true terminals appear after their intermediates.
      for (auto it = block->instructions_.rbegin();
           it != block->instructions_.rend(); ++it) {
        auto &ins = *it;
        if (ins->removed_) continue;

        // Determine if this is a true terminal.
        bool is_terminal = false;
        std::string *ptr_field = nullptr;
        if (auto *gepp = dynamic_cast<IRGetElementPtrPrimeInstructionNode *>(ins.get())) {
          is_terminal = true;
          ptr_field = &gepp->ptrval_;
        } else if (auto *load = dynamic_cast<IRLoadInstructionNode *>(ins.get())) {
          is_terminal = true;
          ptr_field = &load->pointer_;
        } else if (auto *store_v = dynamic_cast<IRStoreVariableInstructionNode *>(ins.get())) {
          is_terminal = true;
          ptr_field = &store_v->pointer_;
        } else if (auto *store_c = dynamic_cast<IRStoreConstInstructionNode *>(ins.get())) {
          is_terminal = true;
          ptr_field = &store_c->pointer_;
        } else if (auto *gep = dynamic_cast<IRGetElementPtrInstructionNode *>(ins.get())) {
          // GEP(const) with use_count > 1 has another consumer; it's a true
          // terminal.  Otherwise it's a pure intermediate — skip it here.
          if (use_count[gep->result_] > 1) {
            is_terminal = true;
            ptr_field = &gep->ptrval_;
          }
        }
        if (!is_terminal || !ptr_field || ptr_field->empty() || (*ptr_field)[0] != '%')
          continue;

        // Walk backward through single-use, unclaimed GEP(const) defs.
        std::string base = *ptr_field;
        int32_t accumulated = 0;
        std::vector<IRGetElementPtrInstructionNode *> intermediates;

        while (true) {
          auto dit = def_map.find(base);
          if (dit == def_map.end()) break;
          IRGetElementPtrInstructionNode *def_gep = dit->second;
          if (use_count[def_gep->result_] > 1) break;
          if (claimed.count(def_gep)) break;
          accumulated += static_cast<int32_t>(gep_offset[def_gep->result_]);
          base = def_gep->ptrval_;
          intermediates.push_back(def_gep);
          claimed.insert(def_gep);
        }

        if (intermediates.empty()) continue;

        folds.push_back({ins.get(), ptr_field,
                         std::move(intermediates), base, accumulated});
      }
    }

    // --- Pass 3: apply folds ---
    std::unordered_set<IRGetElementPtrInstructionNode *> removed_geps;
    for (auto &f : folds) {
      for (auto *gep : f.intermediates) {
        gep->removed_ = true;
        removed_geps.insert(gep);
      }

      if (f.accumulated == 0) {
        // Zero accumulated offset: terminal can use ultimate_base directly.
        *f.ptr_field = f.ultimate_base;
      } else {
        // Non-zero: replace the first intermediate with an ADD.
        auto *first = f.intermediates[0];
        // Un-remove it — it becomes the ADD instruction.
        first->removed_ = false;
        removed_geps.erase(first);

        std::string add_result = first->result_;
        auto add_ins = std::make_shared<IRArithmeticInstructionNode>(
            add_result, "+", "ptr", f.ultimate_base,
            std::to_string(f.accumulated), false);

        // Find and replace in the deque.
        for (auto &block : func->blocks_) {
          for (auto &ins : block->instructions_) {
            if (ins.get() == first) {
              ins = add_ins;
              goto replaced;
            }
          }
        }
        replaced:
        // Update the function's variables map.
        auto var_it = func->variables_.find(add_result);
        if (var_it != func->variables_.end())
          var_it->second = add_ins.get();

        *f.ptr_field = add_result;
      }
    }

    // --- Pass 4: propagate removed intermediates' uses to ultimate base ---
    // Any remaining references to removed intermediates (e.g. across blocks
    // via Move instructions) need to be rewritten.
    for (auto *gep : removed_geps) {
      std::string old_var = gep->result_;
      // Find the ultimate base by walking the fold data
      for (auto &f : folds) {
        for (auto *intermediate : f.intermediates) {
          if (intermediate == gep) {
            for (auto &other_block : func->blocks_) {
              for (auto &other : other_block->instructions_) {
                if (other->removed_) continue;
                ReplaceVarInIns(other.get(), old_var, f.ultimate_base);
              }
            }
            auto var_it = func->variables_.find(old_var);
            if (var_it != func->variables_.end())
              func->variables_.erase(var_it);
            goto next_gep;
          }
        }
      }
      next_gep:;
    }
  }
}
