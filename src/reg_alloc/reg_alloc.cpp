#include "reg_alloc/reg_alloc.h"
#include "reg_alloc/interference_graph.h"
#include "liveness_analysis/CFG.h"
#include "liveness_analysis/CFG_builder.h"
#include "codegen/register.h"
#include "common/bit_set.h"
#include "IR_visitor/memory_allocator/memory_allocator.h"
#include <iostream>

// s1-s11 (callee-saved, preferred) then a0-a7 (caller-saved, fallback)
static const std::vector<uint32_t> kColorPool = {
    9, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27,  // s1-s11
    10, 11, 12, 13, 14, 15, 16, 17                 // a0-a7
};
static const uint32_t kNumColors = 19;

// Leaf functions (no calls) can safely use caller-saved registers for
// promoted variables since they're never clobbered.  We build a pool
// that puts unused a-regs (beyond parameters) and t5 before s-regs,
// eliminating s-reg save/restore for small leaf functions.
// t5 is only used for long jumps in 40000+-ins functions (CLAUDE.md
// temp-register conventions), so it's safe in leaf functions.
// t3-t4 are reserved for the constant cache; t0-t2 and t6 are temp regs.
std::vector<uint32_t> RegAlloc::LeafColorPool(const IRFunctionNode *node) {
  std::vector<uint32_t> pool;
  // t5 is safe for leaf functions (only used for long jumps in huge functions)
  pool.push_back(30);
  // a-regs beyond the parameter count are caller-saved but safe in leaf fns
  for (uint32_t r = 10 + node->a_reg_used_cnt_; r <= 17; ++r)
    pool.push_back(r);
  // s1-s11 as fallback
  for (uint32_t r : {9u, 18u, 19u, 20u, 21u, 22u, 23u, 24u, 25u, 26u, 27u})
    pool.push_back(r);
  // parameter a-regs (a0-a<n-1>) as last resort
  for (uint32_t r = 10; r < 10 + node->a_reg_used_cnt_; ++r)
    pool.push_back(r);
  return pool;
}

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
  bool is_leaf = !node->has_calls_;
  auto color_pool = is_leaf ? LeafColorPool(node) : kColorPool;
  uint32_t num_colors = color_pool.size();
  ig.Coalesce(num_colors);
  auto spilled = ig.Color(num_colors, color_pool);

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

  // 5. Record used s-registers and update a-reg count for caller-save.
  // a_reg_used_cnt_ was set by MemoryAllocator from param count; bump
  // it if the allocator assigned additional a-regs to regular variables
  // so SaveRegister/RestoreRegister covers them around calls.
  // Only callee-saved s-regs (x9=s1, x18-x27=s2-s11) need prologue
  // save/restore; caller-saved t/a-regs are safe in leaf functions.
  // Must run before address assignment so the save area is sized correctly.
  uint32_t max_a_reg_used = node->a_reg_used_cnt_;
  for (auto &[var_id, phys_reg] : ig.GetPhysRegs()) {
    if (ig.IsPrecolored(var_id)) continue;
    if (phys_reg >= 10 && phys_reg <= 17) {
      max_a_reg_used = std::max(max_a_reg_used, phys_reg - 10 + 1);
    } else if (phys_reg == 9 || (phys_reg >= 18 && phys_reg <= 27)) {
      node->used_s_regs_.insert(phys_reg);
    }
  }
  node->a_reg_used_cnt_ = max_a_reg_used;

  // 6. Assign stack addresses to kMemory variables.  MemoryAllocator
  // recorded sizes but deferred address assignment until after reg_alloc so
  // that promoted variables never occupy stack space.  Only variables still
  // in memory after promotion get addresses.
  {
    // Save area at the top of the frame: ra + a0..a(N-1), packed tightly.
    // t-regs are pure scratch (caller-saved, dead after each instruction),
    // so no space is reserved for them.  Only reserve what the function
    // actually uses — a_reg_used_cnt_ was finalized just above (line 234)
    // so it includes both parameter a-regs and any a-regs assigned to
    // promoted variables by the allocator.
    //
    // For functions with kMemory (stack-passed) parameters we keep the
    // original base=72 so the caller's outgoing-argument-area offset
    // (sp - para->address_) matches the callee's access offset
    // (callee_sp + total_stack - para->address_) under the RISC-V ABI.
    bool has_mem_param = false;
    for (auto &p : node->parameters_) {
      if (p->storage_type_ == kMemory) { has_mem_param = true; break; }
    }
    uint32_t base;
    if (!node->has_calls_) {
      base = 0;
    } else if (has_mem_param) {
      base = 72;
    } else {
      base = 8 + 8 * node->a_reg_used_cnt_;
    }
    uint32_t cur = base;

    // Build a set of names already assigned (parameters go first)
    std::unordered_set<std::string> assigned;

    // Parameters first, in declaration order
    for (auto &param : node->parameters_) {
      if (param->storage_type_ != kMemory) continue;
      auto sit = node->variable_size_.find(param->name_);
      uint32_t sz = (sit != node->variable_size_.end()) ? sit->second : 8;
      uint32_t addr = cur + sz;
      node->variable_storage_[param->name_] = {kMemory, addr};
      param->address_ = addr;
      assigned.insert(param->name_);
      cur = addr;
    }

    // Remaining kMemory variables from variable_storage_, in name order
    // for deterministic layout.
    std::vector<std::string> mem_names;
    for (auto &[name, storage] : node->variable_storage_) {
      if (storage.first == kMemory && !assigned.count(name)) {
        mem_names.push_back(name);
      }
    }
    std::sort(mem_names.begin(), mem_names.end());
    for (auto &name : mem_names) {
      auto sit = node->variable_size_.find(name);
      if (sit == node->variable_size_.end()) continue;
      uint32_t sz = sit->second;
      uint32_t addr = cur + sz;
      node->variable_storage_[name] = {kMemory, addr};
      cur = addr;
    }

    // Alloca inner data (always in memory), in block/instruction order
    for (auto &block : node->blocks_) {
      for (auto &ins : block->instructions_) {
        if (ins->removed_) continue;
        auto *alloca = dynamic_cast<IRAllocateInstructionNode *>(ins.get());
        if (!alloca) continue;
        uint32_t sz = Align8(alloca->type_->allocated_size_);
        uint32_t addr = cur + sz;
        alloca->inner_address_ = addr;
        cur = addr;
      }
    }
    node->stack_size_ = cur;

    // Update instruction address_ fields from variable_storage_.
    // Use the CFG to map def IDs -> variable names for each instruction.
    for (auto &block : node->blocks_) {
      for (auto &ins : block->instructions_) {
        if (ins->removed_) continue;
        if (ins->storage_type_ != kMemory) continue;

        // Alloca result pointers: look up by result_ name directly
        if (auto *alloca = dynamic_cast<IRAllocateInstructionNode *>(ins.get())) {
          auto it = node->variable_storage_.find(alloca->result_);
          if (it != node->variable_storage_.end()) {
            ins->address_ = it->second.second;
          }
          continue;
        }

        // Other kMemory instructions: use CFG def ID -> variable name
        if (ins->def_.empty()) continue;
        uint32_t def_id = *ins->def_.begin();
        auto [not_alloc, name] = cfg->GetName(def_id);
        if (!not_alloc) continue;
        auto it = node->variable_storage_.find(name);
        if (it != node->variable_storage_.end()) {
          ins->address_ = it->second.second;
        }
      }
    }
  }

}

void RegAlloc::Visit(IRRootNode *node) {
  for (auto &func : node->functions_) {
    func->Accept(this);
  }
}
