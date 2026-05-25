#include "liveness_analysis/CFG_builder.h"
#include <iostream>

CFGBuilder::CFGBuilder(std::shared_ptr<CFG> cfg) : cfg_(cfg) {}

void CFGBuilder::Merge(IRInstructionNode *node) {
  for (auto &x : node->use_) {
    if (cur_def_.find(x) == cur_def_.end()) {
      cur_use_.emplace(x);
    }
  }
  for (auto &x : node->def_) {
    cur_def_.emplace(x);
  }
}

void CFGBuilder::AddDef(IRInstructionNode *node, const std::string &name, bool allocated) {
  if (name[0] != '%') {
    return;
  }
  auto id = allocated ? cfg_->QueryAllocated(name) : cfg_->Query(name);
  node->def_.emplace(id);
  cfg_->AddDef(id, node);
}

void CFGBuilder::AddUse(IRInstructionNode *node, const std::string &name, bool allocated) {
  if (name[0] != '%') {
    return;
  }
  auto id = allocated ? cfg_->QueryAllocated(name) : cfg_->Query(name);
  node->use_.emplace(id);
  cfg_->AddUse(id, node);
}

void CFGBuilder::Visit(IRArrayNode *node) {}

void CFGBuilder::Visit(IRStructNode *node) {}

void CFGBuilder::Visit(IRArithmeticInstructionNode *node) {
  AddDef(node, node->result_, false);
  AddUse(node, node->operand1_, false);
  AddUse(node, node->operand2_, false);
  Merge(node);
}

void CFGBuilder::Visit(IRNegationInstructionNode *node) {
  AddDef(node, node->result_, false);
  AddUse(node, node->operand_, false);
  Merge(node);
}

void CFGBuilder::Visit(IRBranchInstructionNode *node) {
  AddUse(node, node->condition_, false);
  Merge(node);
  cfg_->AddEdge(cur_block_, node->true_branch_);
  cfg_->AddEdge(cur_block_, node->false_branch_);
}

void CFGBuilder::Visit(IRJumpInstructionNode *node) {
  cfg_->AddEdge(cur_block_, node->destination_);
}

void CFGBuilder::Visit(IRReturnInstructionNode *node) {
  if (!node->type_->IsEmpty()) {
    AddUse(node, node->name_, false);
    Merge(node);
  }
}

void CFGBuilder::Visit(IRAllocateInstructionNode *node) {
  AddDef(node, node->result_, false);
  cfg_->QueryAllocated(node->result_);
  Merge(node);
}

void CFGBuilder::Visit(IRLoadInstructionNode *node) {
  AddDef(node, node->result_, false);
  AddUse(node, node->pointer_, false);
  AddUse(node, node->pointer_, true);
  Merge(node);
}

void CFGBuilder::Visit(IRStoreVariableInstructionNode *node) {
  AddDef(node, node->pointer_, true);
  AddUse(node, node->pointer_, false);
  AddUse(node, node->value_, false);
  Merge(node);
}

void CFGBuilder::Visit(IRStoreConstInstructionNode *node) {
  AddDef(node, node->pointer_, true);
  AddUse(node, node->pointer_, false);
  Merge(node);
}

void CFGBuilder::Visit(IRGetElementPtrInstructionNode *node) {
  AddDef(node, node->result_, false);
  AddUse(node, node->ptrval_, false);
  cfg_->QueryAllocated(node->ptrval_);
  Merge(node);
}

void CFGBuilder::Visit(IRGetElementPtrPrimeInstructionNode *node) {
  AddDef(node, node->result_, false);
  AddUse(node, node->ptrval_, false);
  AddUse(node, node->index_, false);
  cfg_->QueryAllocated(node->ptrval_);
  Merge(node);
}

void CFGBuilder::Visit(IRCompareInstructionNode *node) {
  AddDef(node, node->result_, false);
  AddUse(node, node->operand1_, false);
  AddUse(node, node->operand2_, false);
  Merge(node);
}

void CFGBuilder::Visit(IRArgumentNode *node) {
  std::cerr << "Error! CFG builder shouldn't visit IR argument\n";
  exit(1);
}

void CFGBuilder::Visit(IRCallInstructionNode *node) {
  if (!node->result_type_->IsEmpty()) {
    AddDef(node, node->result_, false);
  }
  for (auto &argument : node->arguments_) {
    AddUse(node, argument->value_, false);
  }
  Merge(node);
}

void CFGBuilder::Visit(IRPhiInstructionNode *node) {}

void CFGBuilder::Visit(IRMoveInstructionNode *node) {
  AddDef(node, node->result_, false);
  AddUse(node, node->source_, false);
  Merge(node);
}

void CFGBuilder::Visit(IRSelectInstructionNode *node) {
  AddDef(node, node->result_, false);
  AddUse(node, node->cond_, false);
  Merge(node);
}

void CFGBuilder::Visit(IRBlockNode *node) {
  cur_block_ = node->id_;
  cur_def_.clear();
  cur_use_.clear();

  for (auto &phi : node->phi_) {
    phi->Accept(this);
  }
  for (auto &instruction : node->instructions_) {
    if (instruction->removed_) {
      continue;
    }
    instruction->Accept(this);
  }

  node->def_ = cur_def_;
  node->use_ = cur_use_;
}

void CFGBuilder::Visit(IRParameterNode *node) {
  std::cerr << "Error! CFG builder shouldn't visit IR parameter\n";
  exit(1);
}

void CFGBuilder::Visit(IRFunctionNode *node) {
  uint32_t size = node->blocks_.size();
  cfg_->Init(size);
  for (uint32_t i = 0; i < size; ++i) {
    cfg_->NewNode(i, node->blocks_[i].get());
  }
  for (auto &para : node->parameters_) {
    cfg_->Query(para->name_);
  }
  for (auto &block : node->blocks_) {
    block->Accept(this);
  }
  cfg_->CalcInOut();
  // for (auto &block : node->blocks_) {
  //   std::cerr << block->id_ << '\n';
  //   std::cerr << "def: ";
  //   for (auto &x : block->def_) {
  //     auto [a, b] = cfg_->GetName(x);
  //     if (!a) {
  //       std::cerr << b << " ";
  //     }
  //   }
  //   std::cerr << '\n';
  //   std::cerr << "use: ";
  //   for (auto &x : block->use_) {
  //     auto [a, b] = cfg_->GetName(x);
  //     if (!a) {
  //       std::cerr << b << " ";
  //     }
  //   }
  //   std::cerr << '\n';
  //   std::cerr << "in: ";
  //   for (auto &x : block->in_) {
  //     auto [a, b] = cfg_->GetName(x);
  //     if (!a) {
  //       std::cerr << b << " ";
  //     }
  //   }
  //   std::cerr << '\n';
  //   std::cerr << "out: ";
  //   for (auto &x : block->out_) {
  //     auto [a, b] = cfg_->GetName(x);
  //     if (!a) {
  //       std::cerr << b << " ";
  //     }
  //   }
  //   std::cerr << '\n';
  // }
}

void CFGBuilder::Visit(IRRootNode *node) {
  std::cerr << "Error! CFG builder shouldn't visit IR root\n";
  exit(1);
}
