#include "mem2reg/eliminator.h"
#include "liveness_analysis/CFG.h"
#include "liveness_analysis/CFG_builder.h"

void EliminateCriticalEdge(std::shared_ptr<IRRootNode> root) {
  auto cfg = std::make_shared<CFG>();
  auto cfg_builder = std::make_shared<CFGBuilder>(cfg);
  cfg_builder->SetSkipCalcInOut(true);
  for (auto &function_node : root->functions_) {
    function_node->Accept(cfg_builder.get());
    cfg->EliminateCriticalEdge(function_node.get());
  }
}
