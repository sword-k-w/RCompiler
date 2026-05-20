#include "mem2reg/mem2reg.h"
#include "liveness_analysis/CFG.h"
#include "liveness_analysis/CFG_builder.h"

#include <iostream>
#include <queue>

void Mem2reg(std::shared_ptr<IRRootNode> root) {
  auto cfg = std::make_shared<CFG>();
  auto cfg_builder = std::make_shared<CFGBuilder>(cfg);
  for (auto &function_node : root->functions_) {
    function_node->Accept(cfg_builder.get());
    cfg->CalcDominatorTree();
    cfg->CalcFrontier();
    for (auto &instruction : function_node->blocks_[0]->instructions_) {
      auto ins = instruction.get();
      auto alloca = dynamic_cast<IRAllocateInstructionNode *>(ins);
      if (alloca == nullptr) {
        continue;
      }
      std::string name = alloca->result_;
      auto id = cfg->Query(name);
      auto id_allocated = cfg->QueryAllocated(name);

      bool flag = true;
      for (auto &block: function_node->blocks_) {
        for (auto &instra: block->instructions_) {
          auto store_v = dynamic_cast<IRStoreVariableInstructionNode *>(instra.get());
          if (store_v != nullptr) {
            if (store_v->value_ == name) {
              flag = false;
              break;
            }
            continue;
          }
          auto store_c = dynamic_cast<IRStoreConstInstructionNode *>(instra.get());
          if (store_c != nullptr) {
            continue;
          }
          auto load = dynamic_cast<IRLoadInstructionNode *>(instra.get());
          if (load != nullptr) {
            continue;
          }
          if (instra->use_.find(id) != instra->use_.end()) {
            flag = false;
            break;
          }
        }
        if (!flag) {
          break;
        }
      }
      if (!flag) {
        continue;
      }
      // std::cerr << "working on " << name << '\n';

      cfg->AddPhi(id_allocated, alloca->type_);
      cfg->PhiReplace(name);

      alloca->removed_ = true;
    }
  }
}
