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

    // --- Pass 2: Original empty-block (jump-only) elimination ---
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
  }
}
