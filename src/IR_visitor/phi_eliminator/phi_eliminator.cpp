#include "IR_visitor/phi_eliminator/phi_eliminator.h"
#include "codegen/phi_topo.h"
#include "IR/name_allocator.h"

void ReplacePhiWithMoves(std::shared_ptr<IRRootNode> root) {
  NameAllocator name_allocator;
  for (auto &func : root->functions_) {
    auto &blocks = func->blocks_;
    uint32_t size = blocks.size();

    for (uint32_t block_id = 0; block_id < size; ++block_id) {
      auto &block = blocks[block_id];
      if (block->instructions_.empty()) {
        continue;
      }

      // Find successor block IDs
      auto last_ins = block->instructions_.back().get();
      std::vector<uint32_t> successors;
      auto br = dynamic_cast<IRBranchInstructionNode *>(last_ins);
      if (br != nullptr) {
        successors.push_back(br->true_branch_);
        successors.push_back(br->false_branch_);
      } else {
        auto j = dynamic_cast<IRJumpInstructionNode *>(last_ins);
        if (j != nullptr) {
          successors.push_back(j->destination_);
        }
      }

      for (auto succ_id : successors) {
        auto &succ = blocks[succ_id];
        if (succ->phi_.empty()) {
          continue;
        }

        PhiTopo topo;
        for (auto &phi : succ->phi_) {
          for (const auto &[val, fr] : phi->val_) {
            if (fr == block_id) {
              topo.AddEdge(val, phi.get());
            }
          }
        }

        auto order = topo.Solve();
        // Insert moves before the terminator (last instruction).
        // Use push_back directly on the deque — not AddInstruction —
        // because AddInstruction checks end_ and will drop moves.
        auto terminator = block->instructions_.back();
        block->instructions_.pop_back();

        std::string cycle_temp;
        for (auto &[from, phi] : order) {
          if (phi == nullptr) {
            cycle_temp = name_allocator.Allocate("%phi.temp");
            block->instructions_.push_back(std::make_shared<IRMoveInstructionNode>(
                cycle_temp, from, dynamic_cast<IRPhiInstructionNode *>(
                    const_cast<IRNode *>(func->variables_[from]))->type_));
          } else if (from.empty()) {
            block->instructions_.push_back(std::make_shared<IRMoveInstructionNode>(
                phi->result_, cycle_temp, phi->type_));
          } else {
            block->instructions_.push_back(std::make_shared<IRMoveInstructionNode>(
                phi->result_, from, phi->type_));
          }
        }

        block->instructions_.push_back(terminator);
      }
    }

    // Remove all phi nodes from all blocks
    for (auto &block : blocks) {
      block->phi_.clear();
    }
  }
}
