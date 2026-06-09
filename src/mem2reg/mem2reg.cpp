#include "mem2reg/mem2reg.h"
#include "liveness_analysis/CFG.h"
#include "liveness_analysis/CFG_builder.h"

#include <iostream>
#include <queue>
#include <set>
#include <string>

void Mem2reg(std::shared_ptr<IRRootNode> root) {
  auto cfg = std::make_shared<CFG>();
  auto cfg_builder = std::make_shared<CFGBuilder>(cfg);
  for (auto &function_node : root->functions_) {
    function_node->Accept(cfg_builder.get());
    cfg->CalcDominatorTree();
    cfg->CalcFrontier();

    std::set<std::string> escaped_vars;
    std::set<uint32_t> direct_use_ids;

    for (auto &block : function_node->blocks_) {
      for (auto &instra : block->instructions_) {
        auto store_v = dynamic_cast<IRStoreVariableInstructionNode *>(instra.get());
        if (store_v != nullptr) {
          escaped_vars.insert(store_v->value_);
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
        for (auto &x : instra->use_) {
          direct_use_ids.insert(x);
        }
      }
    }

    for (auto &instruction : function_node->blocks_[0]->instructions_) {
      auto ins = instruction.get();
      auto alloca = dynamic_cast<IRAllocateInstructionNode *>(ins);
      if (alloca == nullptr) {
        continue;
      }
      std::string name = alloca->result_;
      auto id = cfg->Query(name);
      auto id_allocated = cfg->QueryAllocated(name);

      if (escaped_vars.find(name) != escaped_vars.end()) {
        continue;
      }
      if (direct_use_ids.find(id) != direct_use_ids.end()) {
        continue;
      }

      cfg->AddPhi(id_allocated, alloca->type_);
      cfg->PhiDFS(name, id_allocated);

      alloca->removed_ = true;
    }

    cfg->PhiRewriteAll();
  }
}
