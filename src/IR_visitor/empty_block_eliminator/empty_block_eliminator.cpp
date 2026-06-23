#include "IR_visitor/empty_block_eliminator/empty_block_eliminator.h"
#include <set>
#include <unordered_map>
#include <vector>

void EliminateEmptyBlocks(std::shared_ptr<IRRootNode> root) {
  for (auto &func : root->functions_) {
    auto &blocks = func->blocks_;

    // --- Pass 1: Eliminate phi-merge blocks (br-only) ---
    // A merge block has only a Branch "br %phi -> true:X, false:Y".
    // Its predecessors define %phi via "move %phi <- %tmp" and jump here.
    // We thread the Branch into each predecessor, testing %tmp directly.
    bool changed;
    do {
      changed = false;

      std::unordered_map<uint32_t, std::vector<uint32_t>> preds;
      for (auto &block : blocks) {
        for (auto &ins : block->instructions_) {
          if (ins->removed_) continue;
          if (auto *br = dynamic_cast<IRBranchInstructionNode *>(ins.get())) {
            preds[br->true_branch_].push_back(block->id_);
            preds[br->false_branch_].push_back(block->id_);
            break;
          }
          if (auto *j = dynamic_cast<IRJumpInstructionNode *>(ins.get())) {
            preds[j->destination_].push_back(block->id_);
            break;
          }
        }
      }

      for (auto &block : blocks) {
        if (block->id_ == 0) continue;

        IRBranchInstructionNode *br = nullptr;
        bool has_other = false;
        for (auto &ins : block->instructions_) {
          if (ins->removed_) continue;
          if (auto *b = dynamic_cast<IRBranchInstructionNode *>(ins.get())) {
            br = b;
          } else {
            has_other = true;
            break;
          }
        }
        if (has_other || !br) continue;

        bool any = false;
        for (auto pred_id : preds[block->id_]) {
          auto &pred = blocks[pred_id];

          IRJumpInstructionNode *pred_jmp = nullptr;
          for (auto &ins : pred->instructions_) {
            if (ins->removed_) continue;
            if (auto *j = dynamic_cast<IRJumpInstructionNode *>(ins.get())) {
              if (j->destination_ == block->id_) { pred_jmp = j; break; }
            }
          }
          if (!pred_jmp) continue;

          IRMoveInstructionNode *def_mv = nullptr;
          for (auto &ins : pred->instructions_) {
            if (ins->removed_) continue;
            if (auto *mv = dynamic_cast<IRMoveInstructionNode *>(ins.get())) {
              if (mv->result_ == br->condition_) { def_mv = mv; break; }
            }
          }
          if (!def_mv) continue;

          // Verify the Move's result has no other uses (only br uses it).
          bool has_other_uses = false;
          for (auto &ob : blocks) {
            for (auto &oi : ob->instructions_) {
              if (oi->removed_) continue;
              if (auto *b = dynamic_cast<IRBranchInstructionNode *>(oi.get())) {
                if (b != br && b->condition_ == def_mv->result_)
                  { has_other_uses = true; goto done_check; }
              }
              if (auto *m = dynamic_cast<IRMoveInstructionNode *>(oi.get())) {
                if (m->source_ == def_mv->result_)
                  { has_other_uses = true; goto done_check; }
              }
            }
          }
          done_check:
          if (has_other_uses) continue;

          // Thread: replace pred's Jump with Branch on the Move's source
          pred_jmp->removed_ = true;
          pred->instructions_.push_back(
              std::make_shared<IRBranchInstructionNode>(
                  def_mv->source_, br->true_branch_, br->false_branch_));
          def_mv->removed_ = true;
          any = true;
        }

        if (any) { br->removed_ = true; changed = true; }
      }
    } while (changed);

    // --- Pass 2: Single-move jump threading ---
    // Eliminate blocks containing only a Move + Jump.  The Move is
    // duplicated into each predecessor and the jump is redirected.
    // This cleans up pass-through blocks left by ReplacePhiWithMoves
    // and the function inliner's block splitting.
    // Safety: the Move's result must have no uses outside the target
    // block, otherwise duplicating the definition would create
    // conflicting definitions visible in other blocks.
    do {
      changed = false;

      std::unordered_map<uint32_t, std::vector<uint32_t>> preds;
      for (auto &block : blocks) {
        for (auto &ins : block->instructions_) {
          if (ins->removed_) continue;
          if (auto *br = dynamic_cast<IRBranchInstructionNode *>(ins.get())) {
            preds[br->true_branch_].push_back(block->id_);
            preds[br->false_branch_].push_back(block->id_);
            break;
          }
          if (auto *j = dynamic_cast<IRJumpInstructionNode *>(ins.get())) {
            preds[j->destination_].push_back(block->id_);
            break;
          }
        }
      }

      for (auto &block : blocks) {
        if (block->id_ == 0) continue;

        // Look for pattern: exactly one Move + one Jump
        IRMoveInstructionNode *mv = nullptr;
        IRJumpInstructionNode *jmp = nullptr;
        bool has_other = false;
        for (auto &ins : block->instructions_) {
          if (ins->removed_) continue;
          if (auto *m = dynamic_cast<IRMoveInstructionNode *>(ins.get())) {
            if (mv) { has_other = true; break; }
            mv = m;
          } else if (auto *j = dynamic_cast<IRJumpInstructionNode *>(ins.get())) {
            if (jmp) { has_other = true; break; }
            jmp = j;
          } else {
            has_other = true; break;
          }
        }
        if (has_other || !mv || !jmp) continue;

        uint32_t target = jmp->destination_;
        if (target == block->id_) continue;

        // Verify the Move's result has no uses outside the target block
        bool has_other_uses = false;
        for (auto &ob : blocks) {
          if (ob->id_ == block->id_) continue;
          for (auto &oi : ob->instructions_) {
            if (oi->removed_) continue;
            if (auto *arith = dynamic_cast<IRArithmeticInstructionNode *>(oi.get())) {
              if (arith->operand1_ == mv->result_ || arith->operand2_ == mv->result_)
                { if (ob->id_ != target) { has_other_uses = true; goto use_done; } }
            } else if (auto *neg = dynamic_cast<IRNegationInstructionNode *>(oi.get())) {
              if (neg->operand_ == mv->result_)
                { if (ob->id_ != target) { has_other_uses = true; goto use_done; } }
            } else if (auto *comp = dynamic_cast<IRCompareInstructionNode *>(oi.get())) {
              if (comp->operand1_ == mv->result_ || comp->operand2_ == mv->result_)
                { if (ob->id_ != target) { has_other_uses = true; goto use_done; } }
            } else if (auto *br = dynamic_cast<IRBranchInstructionNode *>(oi.get())) {
              if (br->condition_ == mv->result_)
                { if (ob->id_ != target) { has_other_uses = true; goto use_done; } }
            } else if (auto *ret = dynamic_cast<IRReturnInstructionNode *>(oi.get())) {
              if (ret->name_ == mv->result_)
                { if (ob->id_ != target) { has_other_uses = true; goto use_done; } }
            } else if (auto *sele = dynamic_cast<IRSelectInstructionNode *>(oi.get())) {
              if (sele->cond_ == mv->result_)
                { if (ob->id_ != target) { has_other_uses = true; goto use_done; } }
            } else if (auto *call2 = dynamic_cast<IRCallInstructionNode *>(oi.get())) {
              for (auto &arg : call2->arguments_) {
                if (arg->value_ == mv->result_)
                  { if (ob->id_ != target) { has_other_uses = true; goto use_done; } }
              }
            } else if (auto *store = dynamic_cast<IRStoreVariableInstructionNode *>(oi.get())) {
              if (store->value_ == mv->result_ || store->pointer_ == mv->result_)
                { if (ob->id_ != target) { has_other_uses = true; goto use_done; } }
            } else if (auto *stc = dynamic_cast<IRStoreConstInstructionNode *>(oi.get())) {
              if (stc->pointer_ == mv->result_)
                { if (ob->id_ != target) { has_other_uses = true; goto use_done; } }
            } else if (auto *load = dynamic_cast<IRLoadInstructionNode *>(oi.get())) {
              if (load->pointer_ == mv->result_)
                { if (ob->id_ != target) { has_other_uses = true; goto use_done; } }
            } else if (auto *mv2 = dynamic_cast<IRMoveInstructionNode *>(oi.get())) {
              if (mv2->source_ == mv->result_)
                { if (ob->id_ != target) { has_other_uses = true; goto use_done; } }
            } else if (auto *gep = dynamic_cast<IRGetElementPtrInstructionNode *>(oi.get())) {
              if (gep->ptrval_ == mv->result_)
                { if (ob->id_ != target) { has_other_uses = true; goto use_done; } }
            } else if (auto *gepp = dynamic_cast<IRGetElementPtrPrimeInstructionNode *>(oi.get())) {
              if (gepp->ptrval_ == mv->result_ || gepp->index_ == mv->result_)
                { if (ob->id_ != target) { has_other_uses = true; goto use_done; } }
            }
          }
        }
        use_done:
        if (has_other_uses) continue;

        // Verify the Move's result has exactly one definition (this Move).
        // Names produced by ReplacePhiWithMoves and FunctionInliner are
        // singly-defined; threading a multiply-defined name risks creating
        // multiple reaching definitions visible in the same use.
        bool has_other_def = false;
        for (auto &ob : blocks) {
          if (ob->id_ == block->id_) continue;
          for (auto &oi : ob->instructions_) {
            if (oi->removed_) continue;
            // Check results/defs in various instruction types
            if (auto *arith = dynamic_cast<IRArithmeticInstructionNode *>(oi.get())) {
              if (arith->result_ == mv->result_) { has_other_def = true; goto def_done; }
            } else if (auto *neg = dynamic_cast<IRNegationInstructionNode *>(oi.get())) {
              if (neg->result_ == mv->result_) { has_other_def = true; goto def_done; }
            } else if (auto *comp = dynamic_cast<IRCompareInstructionNode *>(oi.get())) {
              if (comp->result_ == mv->result_) { has_other_def = true; goto def_done; }
            } else if (auto *sele = dynamic_cast<IRSelectInstructionNode *>(oi.get())) {
              if (sele->result_ == mv->result_) { has_other_def = true; goto def_done; }
            } else if (auto *call2 = dynamic_cast<IRCallInstructionNode *>(oi.get())) {
              if (call2->result_ == mv->result_) { has_other_def = true; goto def_done; }
            } else if (auto *load = dynamic_cast<IRLoadInstructionNode *>(oi.get())) {
              if (load->result_ == mv->result_) { has_other_def = true; goto def_done; }
            } else if (auto *mv2 = dynamic_cast<IRMoveInstructionNode *>(oi.get())) {
              if (mv2->result_ == mv->result_) { has_other_def = true; goto def_done; }
            } else if (auto *gep = dynamic_cast<IRGetElementPtrInstructionNode *>(oi.get())) {
              if (gep->result_ == mv->result_) { has_other_def = true; goto def_done; }
            } else if (auto *gepp = dynamic_cast<IRGetElementPtrPrimeInstructionNode *>(oi.get())) {
              if (gepp->result_ == mv->result_) { has_other_def = true; goto def_done; }
            }
          }
        }
        def_done:
        if (has_other_def) continue;

        // Safe to thread: duplicate Move into each predecessor, redirect jump
        for (auto pred_id : preds[block->id_]) {
          auto &pred = blocks[pred_id];
          pred->instructions_.insert(
              pred->instructions_.end() - 1,
              std::make_shared<IRMoveInstructionNode>(
                  mv->result_, mv->source_, mv->type_));

          for (auto &ins : pred->instructions_) {
            if (ins->removed_) continue;
            if (auto *pj = dynamic_cast<IRJumpInstructionNode *>(ins.get())) {
              if (pj->destination_ == block->id_) pj->destination_ = target;
              break;
            }
            if (auto *pbr = dynamic_cast<IRBranchInstructionNode *>(ins.get())) {
              if (pbr->true_branch_ == block->id_) pbr->true_branch_ = target;
              if (pbr->false_branch_ == block->id_) pbr->false_branch_ = target;
              if (pbr->true_branch_ == pbr->false_branch_) {
                pbr->removed_ = true;
                pred->instructions_.push_back(
                    std::make_shared<IRJumpInstructionNode>(pbr->true_branch_));
              }
              break;
            }
          }
        }

        mv->removed_ = true;
        jmp->removed_ = true;
        changed = true;
      }
    } while (changed);

    // --- Pass 3: Jump-only block elimination (chain-following) ---
    do {
      changed = false;

      std::unordered_map<uint32_t, std::vector<uint32_t>> preds;
      for (auto &block : blocks) {
        for (auto &ins : block->instructions_) {
          if (ins->removed_) continue;
          if (auto *br = dynamic_cast<IRBranchInstructionNode *>(ins.get())) {
            preds[br->true_branch_].push_back(block->id_);
            preds[br->false_branch_].push_back(block->id_);
            break;
          }
          if (auto *j = dynamic_cast<IRJumpInstructionNode *>(ins.get())) {
            preds[j->destination_].push_back(block->id_);
            break;
          }
        }
      }

      for (auto &block : blocks) {
        if (block->id_ == 0) continue;

        IRJumpInstructionNode *jmp = nullptr;
        bool has_other = false;
        for (auto &ins : block->instructions_) {
          if (ins->removed_) continue;
          if (auto *j = dynamic_cast<IRJumpInstructionNode *>(ins.get())) {
            jmp = j;
          } else if (dynamic_cast<IRBranchInstructionNode *>(ins.get())) {
            has_other = true; break;
          } else {
            has_other = true; break;
          }
        }
        if (has_other || !jmp) continue;

        uint32_t target = jmp->destination_;
        if (target == block->id_) continue;

        {
          std::set<uint32_t> visited;
          visited.insert(block->id_);
          while (true) {
            auto &tgt = blocks[target];
            IRJumpInstructionNode *chain_jmp = nullptr;
            bool chain_has_other = false;
            for (auto &ins : tgt->instructions_) {
              if (ins->removed_) continue;
              if (auto *j = dynamic_cast<IRJumpInstructionNode *>(ins.get())) {
                chain_jmp = j;
              } else {
                chain_has_other = true; break;
              }
            }
            if (!chain_jmp || chain_has_other) break;
            uint32_t next = chain_jmp->destination_;
            if (next == target || visited.count(next)) break;
            visited.insert(target);
            target = next;
          }
        }

        for (auto pred_id : preds[block->id_]) {
          auto &pred = blocks[pred_id];
          for (auto &ins : pred->instructions_) {
            if (ins->removed_) continue;
            if (auto *pbr = dynamic_cast<IRBranchInstructionNode *>(ins.get())) {
              if (pbr->true_branch_ == block->id_) pbr->true_branch_ = target;
              if (pbr->false_branch_ == block->id_) pbr->false_branch_ = target;
              if (pbr->true_branch_ == pbr->false_branch_) {
                pbr->removed_ = true;
                pred->instructions_.push_back(
                    std::make_shared<IRJumpInstructionNode>(pbr->true_branch_));
              }
              break;
            }
            if (auto *pj = dynamic_cast<IRJumpInstructionNode *>(ins.get())) {
              if (pj->destination_ == block->id_) pj->destination_ = target;
              break;
            }
          }
        }

        jmp->removed_ = true;
        changed = true;
      }
    } while (changed);

    // --- Pass 4: Redirect jumps around completely empty blocks ---
    // After the previous passes, some blocks may have all instructions removed
    // (e.g. threaded Move+Jump pairs, dead blocks from SCCP).  These empty
    // blocks break the DFS fall-through chain in the assembly generator
    // because they have no live terminator to follow.  For each empty block,
    // we follow the chain of removed jumps to find the first live target,
    // then redirect all incoming jumps/branches directly there.
    {
      // Build predecessor map
      std::unordered_map<uint32_t, std::vector<uint32_t>> preds;
      for (auto &block : blocks) {
        for (auto &ins : block->instructions_) {
          if (ins->removed_) continue;
          if (auto *br = dynamic_cast<IRBranchInstructionNode *>(ins.get())) {
            preds[br->true_branch_].push_back(block->id_);
            preds[br->false_branch_].push_back(block->id_);
            break;
          }
          if (auto *j = dynamic_cast<IRJumpInstructionNode *>(ins.get())) {
            preds[j->destination_].push_back(block->id_);
            break;
          }
        }
      }

      // Helper: check if a block is completely empty (all ins removed)
      auto is_empty = [&](uint32_t id) -> bool {
        for (auto &ins : blocks[id]->instructions_)
          if (!ins->removed_) return false;
        return true;
      };

      // Helper: follow chain of removed jumps to find the first live target.
      // The chain is formed by the last removed jump in each empty block.
      auto follow_chain = [&](uint32_t id) -> uint32_t {
        std::set<uint32_t> visited;
        uint32_t cur = id;
        while (visited.insert(cur).second) {
          if (!is_empty(cur)) return cur;  // reached a live block
          uint32_t next = UINT32_MAX;
          for (auto &ins : blocks[cur]->instructions_) {
            if (auto *j = dynamic_cast<IRJumpInstructionNode *>(ins.get()))
              next = j->destination_;
          }
          if (next == UINT32_MAX || next == cur) break;
          cur = next;
        }
        return cur;  // chain ends; return whatever we landed on
      };

      for (auto &block : blocks) {
        if (block->id_ == 0) continue;
        if (!is_empty(block->id_)) continue;

        uint32_t target = follow_chain(block->id_);
        if (target == block->id_) continue;  // no valid target

        for (auto pred_id : preds[block->id_]) {
          auto &pred = blocks[pred_id];
          for (auto &ins : pred->instructions_) {
            if (ins->removed_) continue;
            if (auto *pj = dynamic_cast<IRJumpInstructionNode *>(ins.get())) {
              if (pj->destination_ == block->id_) pj->destination_ = target;
              break;
            }
            if (auto *pbr = dynamic_cast<IRBranchInstructionNode *>(ins.get())) {
              if (pbr->true_branch_ == block->id_) pbr->true_branch_ = target;
              if (pbr->false_branch_ == block->id_) pbr->false_branch_ = target;
              if (pbr->true_branch_ == pbr->false_branch_) {
                pbr->removed_ = true;
                pred->instructions_.push_back(
                    std::make_shared<IRJumpInstructionNode>(pbr->true_branch_));
              }
              break;
            }
          }
        }
      }
    }
  }
}
