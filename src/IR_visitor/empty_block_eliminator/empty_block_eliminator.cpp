#include "IR_visitor/empty_block_eliminator/empty_block_eliminator.h"
#include <set>
#include <unordered_map>
#include <vector>

void EliminateEmptyBlocks(std::shared_ptr<IRRootNode> root) {
  for (auto &func : root->functions_) {
    auto &blocks = func->blocks_;

    bool changed;
    do {
      changed = false;

      // Build predecessor map: block_id -> list of pred block ids
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
            has_other = true;
            break;
          } else {
            has_other = true;
            break;
          }
        }

        if (has_other || !jmp) continue;

        uint32_t target = jmp->destination_;
        if (target == block->id_) continue;

        // Follow chain of empty blocks to find the final non-empty target.
        // This handles cascading correctly: A → B → C → D where B and C
        // are both empty, all predecessors of A get redirected directly to D.
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
                chain_has_other = true;
                break;
              }
            }
            if (!chain_jmp || chain_has_other) break;
            uint32_t next = chain_jmp->destination_;
            if (next == target || visited.count(next)) break;
            visited.insert(target);
            target = next;
          }
        }

        // Redirect all predecessors to bypass this block
        for (auto pred_id : preds[block->id_]) {
          auto &pred = blocks[pred_id];
          for (auto &ins : pred->instructions_) {
            if (ins->removed_) continue;
            if (auto *br = dynamic_cast<IRBranchInstructionNode *>(ins.get())) {
              if (br->true_branch_ == block->id_) br->true_branch_ = target;
              if (br->false_branch_ == block->id_) br->false_branch_ = target;
              if (br->true_branch_ == br->false_branch_) {
                br->removed_ = true;
                pred->instructions_.push_back(
                    std::make_shared<IRJumpInstructionNode>(br->true_branch_));
              }
              break;
            }
            if (auto *j = dynamic_cast<IRJumpInstructionNode *>(ins.get())) {
              if (j->destination_ == block->id_) j->destination_ = target;
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
