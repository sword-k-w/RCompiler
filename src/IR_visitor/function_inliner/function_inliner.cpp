#include "IR_visitor/function_inliner/function_inliner.h"

#include "IR/function_map.h"
#include "IR/name_allocator.h"

#include <set>
#include <unordered_map>
#include <memory>

static const uint32_t kMaxInlineInstructions = 50;

// ─── helpers ───────────────────────────────────────────────────────────

uint32_t FunctionInliner::CountInstructions(IRFunctionNode *callee) {
  uint32_t count = 0;
  for (auto &block : callee->blocks_) {
    for (auto &ins : block->instructions_) {
      if (!ins->removed_) ++count;
    }
  }
  return count;
}

bool FunctionInliner::HasRecursiveCall(IRFunctionNode *func) {
  for (auto &block : func->blocks_) {
    for (auto &ins : block->instructions_) {
      if (ins->removed_) continue;
      if (auto *call = dynamic_cast<IRCallInstructionNode *>(ins.get())) {
        if (call->function_name_ == func->name_) return true;
      }
    }
  }
  return false;
}

// ─── instruction cloning ───────────────────────────────────────────────

std::shared_ptr<IRInstructionNode> FunctionInliner::CloneInstruction(
    std::shared_ptr<IRInstructionNode> ins,
    const std::unordered_map<std::string, std::string>       &name_remap,
    const std::unordered_map<uint32_t, uint32_t>             &block_id_remap) {

  auto Rename = [&](const std::string &s) -> std::string {
    if (s.empty() || s[0] != '%') return s;
    auto it = name_remap.find(s);
    if (it != name_remap.end()) return it->second;
    return s; // caller-scope variable, leave as-is
  };

  auto RemapBlock = [&](uint32_t id) -> uint32_t {
    auto it = block_id_remap.find(id);
    if (it != block_id_remap.end()) return it->second;
    return id;
  };

  // ── arithmetic ──
  if (auto *x = dynamic_cast<IRArithmeticInstructionNode*>(ins.get())) {
    return std::make_shared<IRArithmeticInstructionNode>(
        Rename(x->result_), x->op_, x->type_,
        Rename(x->operand1_), Rename(x->operand2_), x->is_unsigned_);
  }
  // ── negation ──
  if (auto *x = dynamic_cast<IRNegationInstructionNode*>(ins.get())) {
    return std::make_shared<IRNegationInstructionNode>(
        Rename(x->result_), x->is_minus_, x->type_, Rename(x->operand_));
  }
  // ── branch ──
  if (auto *x = dynamic_cast<IRBranchInstructionNode*>(ins.get())) {
    return std::make_shared<IRBranchInstructionNode>(
        Rename(x->condition_),
        RemapBlock(x->true_branch_),
        RemapBlock(x->false_branch_));
  }
  // ── jump ──
  if (auto *x = dynamic_cast<IRJumpInstructionNode*>(ins.get())) {
    return std::make_shared<IRJumpInstructionNode>(
        RemapBlock(x->destination_));
  }
  // ── return ──
  if (auto *x = dynamic_cast<IRReturnInstructionNode*>(ins.get())) {
    if (x->type_->IsEmpty()) {
      return std::make_shared<IRReturnInstructionNode>();
    } else {
      return std::make_shared<IRReturnInstructionNode>(x->type_, Rename(x->name_));
    }
  }
  // ── alloca ──
  if (auto *x = dynamic_cast<IRAllocateInstructionNode*>(ins.get())) {
    return std::make_shared<IRAllocateInstructionNode>(
        Rename(x->result_), x->type_);
  }
  // ── load ──
  if (auto *x = dynamic_cast<IRLoadInstructionNode*>(ins.get())) {
    return std::make_shared<IRLoadInstructionNode>(
        Rename(x->result_), x->type_, Rename(x->pointer_));
  }
  // ── store variable ──
  if (auto *x = dynamic_cast<IRStoreVariableInstructionNode*>(ins.get())) {
    return std::make_shared<IRStoreVariableInstructionNode>(
        x->type_, Rename(x->value_), Rename(x->pointer_));
  }
  // ── store const ──
  if (auto *x = dynamic_cast<IRStoreConstInstructionNode*>(ins.get())) {
    return std::make_shared<IRStoreConstInstructionNode>(
        x->type_, x->value_, Rename(x->pointer_));
  }
  // ── GEP (constant index) ──
  if (auto *x = dynamic_cast<IRGetElementPtrInstructionNode*>(ins.get())) {
    return std::make_shared<IRGetElementPtrInstructionNode>(
        Rename(x->result_), x->type_, Rename(x->ptrval_), x->index_);
  }
  // ── GEP' (variable index) ──
  if (auto *x = dynamic_cast<IRGetElementPtrPrimeInstructionNode*>(ins.get())) {
    return std::make_shared<IRGetElementPtrPrimeInstructionNode>(
        Rename(x->result_), x->type_, Rename(x->ptrval_), Rename(x->index_));
  }
  // ── compare ──
  if (auto *x = dynamic_cast<IRCompareInstructionNode*>(ins.get())) {
    return std::make_shared<IRCompareInstructionNode>(
        Rename(x->result_), x->op_, x->type_,
        Rename(x->operand1_), Rename(x->operand2_));
  }
  // ── call ──
  if (auto *x = dynamic_cast<IRCallInstructionNode*>(ins.get())) {
    auto cloned = std::make_shared<IRCallInstructionNode>(
        Rename(x->result_), x->result_type_, x->function_name_);
    for (auto &arg : x->arguments_) {
      cloned->AddArgument(std::make_shared<IRArgumentNode>(
          arg->type_, Rename(arg->value_)));
    }
    return cloned;
  }
  // ── phi ──
  if (auto *x = dynamic_cast<IRPhiInstructionNode*>(ins.get())) {
    auto cloned = std::make_shared<IRPhiInstructionNode>(
        Rename(x->result_), x->type_);
    for (auto &[val, block_id] : x->val_) {
      cloned->val_.emplace_back(Rename(val), RemapBlock(block_id));
    }
    return cloned;
  }
  // ── move ──
  if (auto *x = dynamic_cast<IRMoveInstructionNode*>(ins.get())) {
    return std::make_shared<IRMoveInstructionNode>(
        Rename(x->result_), Rename(x->source_), x->type_);
  }
  // ── select ──
  if (auto *x = dynamic_cast<IRSelectInstructionNode*>(ins.get())) {
    return std::make_shared<IRSelectInstructionNode>(
        Rename(x->result_), Rename(x->cond_));
  }

  return nullptr; // should not reach here
}

// ─── core inlining ─────────────────────────────────────────────────────

void FunctionInliner::InlineCall(
    std::shared_ptr<IRFunctionNode>     caller,
    std::shared_ptr<IRBlockNode>        block,
    size_t                              call_idx,
    IRFunctionNode                     *callee,
    std::shared_ptr<IRCallInstructionNode> call,
    NameAllocator                      &name_allocator) {

  std::unordered_map<std::string, std::string> name_remap;
  std::unordered_map<uint32_t, uint32_t>       block_id_remap;

  // =====================================================================
  // Phase 1 — pre-scan callee to build the variable rename map
  // =====================================================================

  // Map callee parameter names directly to the caller's argument values.
  // This avoids creating intermediate move instructions.
  for (size_t i = 0; i < callee->parameters_.size(); ++i) {
    name_remap[callee->parameters_[i]->name_] = call->arguments_[i]->value_;
  }

  for (auto &cb : callee->blocks_) {
    for (auto &ins : cb->instructions_) {
      if (ins->removed_) continue;

      if (auto *x = dynamic_cast<IRArithmeticInstructionNode*>(ins.get())) {
        if (!x->result_.empty() && x->result_[0] == '%')
          name_remap[x->result_] = name_allocator.Allocate("%inline.");
      } else if (auto *x = dynamic_cast<IRNegationInstructionNode*>(ins.get())) {
        if (!x->result_.empty() && x->result_[0] == '%')
          name_remap[x->result_] = name_allocator.Allocate("%inline.");
      } else if (auto *x = dynamic_cast<IRAllocateInstructionNode*>(ins.get())) {
        if (!x->result_.empty() && x->result_[0] == '%')
          name_remap[x->result_] = name_allocator.Allocate("%inline.");
      } else if (auto *x = dynamic_cast<IRLoadInstructionNode*>(ins.get())) {
        if (!x->result_.empty() && x->result_[0] == '%')
          name_remap[x->result_] = name_allocator.Allocate("%inline.");
      } else if (auto *x = dynamic_cast<IRGetElementPtrInstructionNode*>(ins.get())) {
        if (!x->result_.empty() && x->result_[0] == '%')
          name_remap[x->result_] = name_allocator.Allocate("%inline.");
      } else if (auto *x = dynamic_cast<IRGetElementPtrPrimeInstructionNode*>(ins.get())) {
        if (!x->result_.empty() && x->result_[0] == '%')
          name_remap[x->result_] = name_allocator.Allocate("%inline.");
      } else if (auto *x = dynamic_cast<IRCompareInstructionNode*>(ins.get())) {
        if (!x->result_.empty() && x->result_[0] == '%')
          name_remap[x->result_] = name_allocator.Allocate("%inline.");
      } else if (auto *x = dynamic_cast<IRCallInstructionNode*>(ins.get())) {
        if (!x->result_.empty() && x->result_[0] == '%')
          name_remap[x->result_] = name_allocator.Allocate("%inline.");
      } else if (auto *x = dynamic_cast<IRPhiInstructionNode*>(ins.get())) {
        if (!x->result_.empty() && x->result_[0] == '%')
          name_remap[x->result_] = name_allocator.Allocate("%inline.");
      } else if (auto *x = dynamic_cast<IRMoveInstructionNode*>(ins.get())) {
        if (!x->result_.empty() && x->result_[0] == '%')
          name_remap[x->result_] = name_allocator.Allocate("%inline.");
      } else if (auto *x = dynamic_cast<IRSelectInstructionNode*>(ins.get())) {
        if (!x->result_.empty() && x->result_[0] == '%')
          name_remap[x->result_] = name_allocator.Allocate("%inline.");
      }
      // also catch return instruction values (for the result value move)
      if (auto *x = dynamic_cast<IRReturnInstructionNode*>(ins.get())) {
        if (!x->name_.empty() && x->name_[0] == '%') {
          // only add if not already mapped (return names are already in map
          // since they were produced by an earlier instruction)
          if (name_remap.find(x->name_) == name_remap.end()) {
            name_remap[x->name_] = name_allocator.Allocate("%inline.ret.");
          }
        }
      }
    }
  }

  // =====================================================================
  // Phase 2 — clone every callee block
  // =====================================================================

  std::vector<std::shared_ptr<IRBlockNode>> cloned_blocks;
  uint32_t base_id = static_cast<uint32_t>(caller->blocks_.size());

  // First, pre-build the block_id_remap for ALL callee blocks BEFORE cloning.
  // This ensures cross-block references (jumps, branches, phis) are correctly
  // remapped even when the target block hasn't been cloned yet.
  for (size_t i = 0; i < callee->blocks_.size(); ++i) {
    uint32_t new_id = base_id + static_cast<uint32_t>(i);
    block_id_remap[callee->blocks_[i]->id_] = new_id;
  }

  // Mem2reg requires all alloca instructions to be in the entry block (block 0).
  // When cloning the callee's blocks we therefore redirect every alloca into the
  // caller's first block instead of the cloned block.
  // Important: the entry block is the block with ID 0, which may not be at
  // caller->blocks_[0] (blocks are not necessarily ordered by ID).
  std::shared_ptr<IRBlockNode> entry = nullptr;
  for (auto &b : caller->blocks_) {
    if (b->GetID() == 0) { entry = b; break; }
  }
  if (entry == nullptr) {
    // Should never happen; fallback to index 0
    entry = caller->blocks_[0];
  }

  uint32_t pushed_allocas = 0;

  for (size_t i = 0; i < callee->blocks_.size(); ++i) {
    uint32_t new_id = base_id + static_cast<uint32_t>(i);
    auto cloned = std::make_shared<IRBlockNode>(new_id);

    for (auto &ins : callee->blocks_[i]->instructions_) {
      if (ins->removed_) continue;
      auto cloned_ins = CloneInstruction(ins, name_remap, block_id_remap);
      if (!cloned_ins) continue;

      if (dynamic_cast<IRAllocateInstructionNode *>(cloned_ins.get())) {
        // Redirect alloca to the caller's entry block (match AddInstruction's
        // emplace_front convention).
        entry->instructions_.push_front(cloned_ins);
        ++pushed_allocas;
      } else {
        cloned->instructions_.push_back(cloned_ins);
        if (dynamic_cast<IRBranchInstructionNode*>(cloned_ins.get()) ||
            dynamic_cast<IRJumpInstructionNode*>(cloned_ins.get()) ||
            dynamic_cast<IRReturnInstructionNode*>(cloned_ins.get())) {
          cloned->end_ = true;
        }
      }
    }

    // clone phis
    for (auto &phi : callee->blocks_[i]->phi_) {
      auto cloned_phi = CloneInstruction(phi, name_remap, block_id_remap);
      if (cloned_phi) {
        cloned->AddPhi(std::dynamic_pointer_cast<IRPhiInstructionNode>(cloned_phi));
      }
    }

    cloned_blocks.push_back(cloned);
    caller->AddBlock(cloned);
  }

  // =====================================================================
  // Phase 3 — split the caller's block at the call site
  // =====================================================================

  // If we pushed allocas to the same block that contains the call, the
  // call's index has shifted right by |pushed_allocas|.
  if (block.get() == entry.get()) {
    call_idx += pushed_allocas;
  }

  auto continuation = std::make_shared<IRBlockNode>(
      static_cast<uint32_t>(caller->blocks_.size()));

  // move everything after the call into the continuation block
  auto after_it = block->instructions_.begin() +
                  static_cast<std::ptrdiff_t>(call_idx + 1);
  continuation->instructions_.insert(
      continuation->instructions_.end(),
      std::make_move_iterator(after_it),
      std::make_move_iterator(block->instructions_.end()));

  // preserve end_ flag from the original terminator
  continuation->end_ = block->end_;

  // erase the call and everything after it
  block->instructions_.erase(
      block->instructions_.begin() + static_cast<std::ptrdiff_t>(call_idx),
      block->instructions_.end());

  // add jump to inlined entry
  uint32_t entry_id = cloned_blocks.empty()
      ? continuation->GetID()
      : cloned_blocks[0]->GetID();
  block->instructions_.push_back(
      std::make_shared<IRJumpInstructionNode>(entry_id));
  block->end_ = true;

  // if continuation is empty (shouldn't happen), add a fallback jump
  if (continuation->instructions_.empty()) {
    continuation->instructions_.push_back(
        std::make_shared<IRJumpInstructionNode>(continuation->GetID()));
    continuation->end_ = true;
  }

  caller->AddBlock(continuation);

  // =====================================================================
  // Phase 4 — rewrite returns to jumps targeting the continuation block
  // =====================================================================

  uint32_t cont_id = continuation->GetID();

  for (auto &cb : cloned_blocks) {
    // iterate by index so we can modify while walking
    for (size_t i = 0; i < cb->instructions_.size(); ++i) {
      auto ret = dynamic_cast<IRReturnInstructionNode*>(
          cb->instructions_[i].get());
      if (!ret || ret->removed_) continue;

      // non-void return: copy ret value to call result
      if (!call->result_.empty() && !ret->name_.empty()) {
        cb->instructions_.push_back(
            std::make_shared<IRMoveInstructionNode>(
                call->result_, ret->name_, ret->type_));
      }

      // replace return with jump
      cb->instructions_.push_back(
          std::make_shared<IRJumpInstructionNode>(cont_id));

      ret->removed_ = true;
      cb->end_ = true;
    }
  }

  // mark the original call as removed
  call->removed_ = true;
}

// ─── function-level scan ───────────────────────────────────────────────

bool FunctionInliner::InlineCallsInFunction(
    std::shared_ptr<IRFunctionNode> func,
    std::shared_ptr<IRRootNode>     root,
    uint32_t                        max_insn,
    NameAllocator                  &name_allocator,
    std::set<IRFunctionNode *>     &inlined_callees) {

  bool changed = false;
  bool local_changed;

  do {
    local_changed = false;
    for (size_t block_idx = 0;
         block_idx < func->blocks_.size();
         ++block_idx) {
      auto &block = func->blocks_[block_idx];

      for (size_t inst_idx = 0;
           inst_idx < block->instructions_.size();
           ++inst_idx) {
        auto &ins = block->instructions_[inst_idx];
        if (ins->removed_) continue;

        auto call = std::dynamic_pointer_cast<IRCallInstructionNode>(ins);
        if (!call) continue;

        // lookup callee
        IRFunctionNode *callee = FunctionMap::Instance().Query(
            call->function_name_);
        if (!callee || callee->IsBuiltin()) continue;

        // no self-recursion (the call site we are looking at is inside
        // the same function we're trying to inline into)
        if (callee == func.get()) continue;

        // don't inline the same callee into this caller more than once;
        // stops both direct recursion (f → f) and mutual recursion
        // (f → g → f) from growing without bound
        if (!inlined_callees.insert(callee).second) continue;

        // heuristic: don't inline huge functions
        if (CountInstructions(callee) > max_insn) continue;

        // sanity: arg count must match param count
        if (call->arguments_.size() != callee->parameters_.size()) continue;

        // skip callees with no body and non-void return
        // (result would be undefined)
        if (callee->blocks_.empty() && !call->result_.empty()) continue;

        InlineCall(func, block, inst_idx, callee, call, name_allocator);
        local_changed = true;
        changed = true;
        break;  // restart block scan
      }
      if (local_changed) break;  // restart function scan
    }
  } while (local_changed);

  return changed;
}

// ─── public entry point ────────────────────────────────────────────────

void FunctionInliner::Run(std::shared_ptr<IRRootNode> root) {
  // Per-function allocators: persist across outer-loop iterations so that
  // when a function is revisited after other inlines introduce new calls,
  // the name counter continues from where it left off — avoiding duplicate
  // %inline.* names that would collide.
  std::unordered_map<IRFunctionNode *, NameAllocator> allocators;
  // Per-function already-inlined sets: persist across outer-loop iterations
  // so that recursive / mutually-recursive callees don't cause unbounded
  // expansion when the function is revisited.
  std::unordered_map<IRFunctionNode *, std::set<IRFunctionNode *>> inline_sets;

  bool changed;
  do {
    changed = false;
    for (auto &func : root->functions_) {
      if (func->IsBuiltin()) continue;
      if (InlineCallsInFunction(func, root, kMaxInlineInstructions,
                                allocators[func.get()],
                                inline_sets[func.get()])) {
        changed = true;
      }
    }
  } while (changed);
}
