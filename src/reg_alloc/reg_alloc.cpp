#include "reg_alloc/reg_alloc.h"
#include "reg_alloc/interference_graph.h"
#include "liveness_analysis/CFG.h"
#include "liveness_analysis/CFG_builder.h"
#include "codegen/register.h"

static const std::vector<uint32_t> kColorPool = {9, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27};
static const uint32_t kNumColors = 11;

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
    block->in_.clear();
    block->out_.clear();
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

  // Track which def IDs are promotable (used to filter uses below)
  std::set<uint32_t> promotable_defs;

  // Walk instructions in reverse per block to build edges.
  // Only track non-allocated variables with promotable types.
  for (auto &block : node->blocks_) {
    std::set<uint32_t> live;
    for (auto id : block->out_) {
      auto [not_alloc, name] = cfg->GetName(id);
      if (not_alloc) live.insert(id);  // keep normal (Query) vars
    }

    auto &insts = block->instructions_;
    for (auto it = insts.rbegin(); it != insts.rend(); ++it) {
      auto *ins = it->get();
      if (ins->removed_) continue;

      for (auto def_id : ins->def_) {
        auto [not_alloc, name] = cfg->GetName(def_id);
        if (!not_alloc) continue;  // skip allocated (QueryAllocated)
        if (!IsPromotableType(ins)) continue;  // skip complex types

        promotable_defs.insert(def_id);
        ig.AddNode(def_id);
        ig.IncDefCount(def_id);

        for (auto live_id : live) {
          if (live_id != def_id) {
            ig.AddNode(live_id);
            ig.AddEdge(def_id, live_id);
          }
        }
        live.erase(def_id);
      }

      for (auto use_id : ins->use_) {
        auto [not_alloc, name] = cfg->GetName(use_id);
        if (!not_alloc) continue;  // skip allocated
        if (!promotable_defs.count(use_id)) continue;  // only track uses of promotable defs

        ig.AddNode(use_id);
        ig.IncUseCount(use_id);
        live.insert(use_id);
      }
    }
  }

  // 3. Color
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

  // Record used s-registers
  for (auto &[var_id, phys_reg] : ig.GetPhysRegs()) {
    if (!ig.IsPrecolored(var_id)) {
      node->used_s_regs_.insert(phys_reg);
    }
  }
}

void RegAlloc::Visit(IRRootNode *node) {
  for (auto &func : node->functions_) {
    func->Accept(this);
  }
}
