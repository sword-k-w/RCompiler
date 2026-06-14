#include "reg_alloc/reg_alloc.h"
#include "reg_alloc/interference_graph.h"
#include "liveness_analysis/CFG.h"
#include "liveness_analysis/CFG_builder.h"
#include "codegen/register.h"
#include "common/bit_set.h"
#include <iostream>

// s1-s11 (callee-saved, preferred) then a0-a7 (caller-saved, fallback)
static const std::vector<uint32_t> kColorPool = {
    9, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27,  // s1-s11
    10, 11, 12, 13, 14, 15, 16, 17                 // a0-a7
};
static const uint32_t kNumColors = 19;

bool RegAlloc::IsPromotableType(IRInstructionNode *ins) {
  if (auto *arith = dynamic_cast<IRArithmeticInstructionNode *>(ins)) return true;
  if (auto *neg = dynamic_cast<IRNegationInstructionNode *>(ins)) return true;
  if (auto *comp = dynamic_cast<IRCompareInstructionNode *>(ins)) return true;
  if (auto *sele = dynamic_cast<IRSelectInstructionNode *>(ins)) return true;
  if (auto *gep = dynamic_cast<IRGetElementPtrInstructionNode *>(ins)) return true;
  if (auto *gepp = dynamic_cast<IRGetElementPtrPrimeInstructionNode *>(ins)) return true;
  if (auto *load = dynamic_cast<IRLoadInstructionNode *>(ins)) {
    if (!load->type_->length_.empty()) return false;
    auto t = load->type_->base_type_;
    return t == "i32" || t == "i1" || t == "ptr";
  }
  if (auto *call = dynamic_cast<IRCallInstructionNode *>(ins)) {
    if (call->result_type_->IsEmpty()) return false;
    if (!call->result_type_->length_.empty()) return false;
    auto t = call->result_type_->base_type_;
    return t == "i32" || t == "i1" || t == "ptr";
  }
  if (auto *move = dynamic_cast<IRMoveInstructionNode *>(ins)) {
    if (!move->type_->length_.empty()) return false;
    auto t = move->type_->base_type_;
    return t == "i32" || t == "i1" || t == "ptr";
  }
  return false;
}

void RegAlloc::Visit(IRFunctionNode *node) {
  if (node->IsBuiltin()) return;

  // Clear stale def/use sets from previous CFG runs
  for (auto &block : node->blocks_) {
    block->def_.clear();
    block->use_.clear();
    block->in_.Clear();
    block->out_.Clear();
    for (auto &ins : block->instructions_) {
      ins->def_.clear();
      ins->use_.clear();
    }
  }

  // 1. Run CFG builder to populate def/use sets and compute liveness
  auto cfg = std::make_shared<CFG>();
  auto cfg_builder = std::make_shared<CFGBuilder>(cfg);
  node->Accept(cfg_builder.get());

  // 2. Build interference graph
  InterferenceGraph ig;

  // Precolored nodes: parameters in a0-a7
  for (auto &parameter : node->parameters_) {
    if (parameter->storage_type_ == kRegister) {
      auto id = cfg->Query(parameter->name_);
      ig.SetPrecolored(id, parameter->address_);
    }
  }

  // Build a set of precolored parameter IDs so they can be tracked
  // in liveness and properly interfere with promotable variables.
  BitSet precolored_ids;
  precolored_ids.Resize(cfg->GetVarCount() + 256);
  for (auto &parameter : node->parameters_) {
    if (parameter->storage_type_ == kRegister) {
      auto id = cfg->Query(parameter->name_);
      precolored_ids.Set(id);
    }
  }

  // Step 1: Compute the set of promotable variable IDs upfront.
  BitSet promotable_vars;
  promotable_vars.Resize(cfg->GetVarCount() + 256);
  for (auto &block : node->blocks_) {
    for (auto &inst : block->instructions_) {
      auto *ins = inst.get();
      if (ins->removed_) continue;
      if (!IsPromotableType(ins)) continue;
      for (auto def_id : ins->def_) {
        auto [not_alloc, name] = cfg->GetName(def_id);
        if (not_alloc) {
          promotable_vars.Set(def_id);
        }
      }
    }
  }

  // Step 2: Per block, compute instruction-level liveness and build edges.
  uint32_t var_count = cfg->GetVarCount();
  for (auto &block : node->blocks_) {
    BitSet live;
    live.Resize(var_count + 256);
    block->out_.ForEach([&](uint32_t id) {
      auto [not_alloc, name] = cfg->GetName(id);
      if (not_alloc && promotable_vars.Test(id)) live.Set(id);
      else if (precolored_ids.Test(id)) live.Set(id);
    });

    auto &insts = block->instructions_;
    for (auto it = insts.rbegin(); it != insts.rend(); ++it) {
      auto *ins = it->get();
      if (ins->removed_) continue;

      // Process defs: killed going backwards (remove from live)
      for (auto def_id : ins->def_) {
        if (promotable_vars.Test(def_id)) {
          live.ClearBit(def_id);
        } else if (precolored_ids.Test(def_id)) {
          live.ClearBit(def_id);
        }
      }
      // Process uses: become live going backwards (add to live)
      for (auto use_id : ins->use_) {
        if (promotable_vars.Test(use_id)) {
          live.Set(use_id);
        } else if (precolored_ids.Test(use_id)) {
          live.Set(use_id);
        }
      }

      // Collect move pairs for coalescing
      if (auto *mv = dynamic_cast<IRMoveInstructionNode *>(ins)) {
        if (!mv->def_.empty() && !mv->use_.empty()) {
          ig.AddMovePair(*mv->use_.begin(), *mv->def_.begin());
        }
      }

      // Build edges: for each promotable def, add interference
      // edges to every other variable in liveIn.
      for (auto def_id : ins->def_) {
        auto [not_alloc, name] = cfg->GetName(def_id);
        if (!not_alloc) continue;
        if (!promotable_vars.Test(def_id)) continue;

        ig.AddNode(def_id);
        ig.IncDefCount(def_id);
        live.ForEach([&](uint32_t other) {
          if (other != def_id) {
            ig.AddNode(other);
            ig.AddEdge(def_id, other);
          }
        });
      }

      // Add use counts for promotable uses
      for (auto use_id : ins->use_) {
        if (promotable_vars.Test(use_id)) {
          ig.AddNode(use_id);
          ig.IncUseCount(use_id);
        }
      }
    }
  }

  // 3. Coalesce move-related nodes, then color
  ig.Coalesce(kNumColors);
  auto spilled = ig.Color(kNumColors, kColorPool);

  // 4. Rewrite storage for colored variables (both variable_storage_ and instruction fields)
  for (auto &block : node->blocks_) {
    for (auto &ins : block->instructions_) {
      if (ins->removed_) continue;
      for (auto def_id : ins->def_) {
        if (!ig.HasPhysReg(def_id)) continue;
        if (ig.IsPrecolored(def_id)) continue;
        auto [not_alloc, name] = cfg->GetName(def_id);
        if (!not_alloc) continue;  // skip allocated (QueryAllocated)
        if (!IsPromotableType(ins.get())) continue;  // skip complex types

        uint32_t phys_reg = ig.GetPhysReg(def_id);
        ins->storage_type_ = kRegister;
        ins->address_ = phys_reg;
        node->variable_storage_[name] = {kRegister, phys_reg};
      }
    }
  }

  // Record used s-registers and update a-reg count for caller-save.
  // a_reg_used_cnt_ was set by MemoryAllocator from param count; bump
  // it if the allocator assigned additional a-regs to regular variables
  // so SaveRegister/RestoreRegister covers them around calls.
  uint32_t max_a_reg_used = node->a_reg_used_cnt_;
  for (auto &[var_id, phys_reg] : ig.GetPhysRegs()) {
    if (ig.IsPrecolored(var_id)) continue;
    if (phys_reg >= 10 && phys_reg <= 17) {
      max_a_reg_used = std::max(max_a_reg_used, phys_reg - 10 + 1);
    } else {
      node->used_s_regs_.insert(phys_reg);
    }
  }
  node->a_reg_used_cnt_ = max_a_reg_used;
}

void RegAlloc::Visit(IRRootNode *node) {
  for (auto &func : node->functions_) {
    func->Accept(this);
  }
}
