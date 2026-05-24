#include "liveness_analysis/CFG.h"
#include "liveness_analysis/dominator_tree.h"

#include <cassert>
#include <queue>
#include <iostream>

void CFG::VariableMap::Clear() {
  id_.clear();
  allocated_id_.clear();
  name_.clear();
}

uint32_t CFG::VariableMap::Query(const std::string &name) {
  auto it = id_.find(name);
  if (it != id_.end()) {
    return it->second;
  }
  auto new_id = name_.size();
  id_[name] = new_id;
  name_.emplace_back(true, name);
  def_.emplace_back();
  use_.emplace_back();
  return new_id;
}

uint32_t CFG::VariableMap::QueryAllocated(const std::string &name) {
  auto it = allocated_id_.find(name);
  if (it != allocated_id_.end()) {
    return it->second;
  }
  auto new_id = name_.size();
  allocated_id_[name] = new_id;
  name_.emplace_back(false, name);
  def_.emplace_back();
  use_.emplace_back();
  return new_id;
}

std::pair<bool, std::string> CFG::VariableMap::GetName(uint32_t id) {
  assert(id < name_.size());
  return name_[id];
}

CFG::BlockNode::BlockNode(uint32_t id, IRBlockNode *origin) : id_(id), origin_(origin) {}

void CFG::Init(uint32_t node_cnt) {
  variable_map_.Clear();
  node_pool_.clear();

  node_pool_.resize(node_cnt);
}

void CFG::NewNode(uint32_t id, IRBlockNode *origin) {
  node_pool_[id] = std::make_shared<BlockNode>(id, origin);
}

uint32_t CFG::Query(const std::string &name) {
  return variable_map_.Query(name);
}

uint32_t CFG::QueryAllocated(const std::string &name) {
  return variable_map_.QueryAllocated(name);
}

std::pair<bool, std::string> CFG::GetName(uint32_t id) {
  return variable_map_.GetName(id);
}

void CFG::AddEdge(uint32_t u, uint32_t v) {
  node_pool_[u]->succs_.emplace_back(node_pool_[v].get());
  node_pool_[v]->preds_.emplace_back(node_pool_[u].get());
}

void CFG::AddDef(uint32_t id, IRInstructionNode *node) {
  variable_map_.def_[id].emplace_back(node);
}

void CFG::AddUse(uint32_t id, IRInstructionNode *node) {
  variable_map_.use_[id].emplace_back(node);
}

void CFG::CalcInOut() {
  auto size = node_pool_.size();
  std::vector<uint32_t> bfs_order;
  std::vector<uint32_t> vis(size, 0);
  uint32_t pos = 0;
  for (uint32_t i = 0; i < size; ++i) {
    if (node_pool_[i]->succs_.empty()) {
      vis[i] = 1;
      bfs_order.emplace_back(i);
    }
  }
  while (pos < size) {
    uint32_t u = bfs_order[pos];
    ++pos;
    for (auto &v : node_pool_[u]->preds_) {
      if (!vis[v->id_]) {
        vis[v->id_] = true;
        bfs_order.emplace_back(v->id_);
      }
    }
  }
  bool flag = true;
  while (flag) {
    flag = false;
    for (uint32_t i = 0; i < size; ++i) {
      auto u = bfs_order[i];
      auto old_out = node_pool_[u]->origin_->out_.size();
      auto old_in = node_pool_[u]->origin_->in_.size();
      for (auto &v : node_pool_[u]->succs_) {
        for (auto &x : v->origin_->in_) {
          node_pool_[u]->origin_->out_.emplace(x);
        }
      }
      for (auto &x : node_pool_[u]->origin_->out_) {
        node_pool_[u]->origin_->in_.emplace(x);
      }
      for (auto &x : node_pool_[u]->origin_->def_) {
        auto it = node_pool_[u]->origin_->in_.find(x);
        if (it != node_pool_[u]->origin_->in_.end()) {
          node_pool_[u]->origin_->in_.erase(it);
        }
      }
      for (auto &x : node_pool_[u]->origin_->use_) {
        node_pool_[u]->origin_->in_.emplace(x);
      }
      if (old_out != node_pool_[u]->origin_->out_.size()) {
        flag = true;
      }
      if (old_in != node_pool_[u]->origin_->in_.size()) {
        flag = true;
      }
    }
  }
}

void CFG::CalcDominatorTree() {
  DominatorTreeSolver dts;
  auto size = node_pool_.size();
  dts.Init(size);
  for (uint32_t i = 0; i < size; ++i) {
    for (auto &v : node_pool_[i]->succs_) {
      dts.AddEdge(0, i + 1, v->id_ + 1);
    }
    for (auto &v : node_pool_[i]->preds_) {
      dts.AddEdge(1, i + 1, v->id_ + 1);
    }
  }
  dts.Tarjan(1);
  dom_.resize(size);
  dom_child_.clear();
  dom_child_.resize(size);
  for (uint32_t i = 0; i < size; ++i) {
    dom_[i] = dts.Query(i + 1);
    --dom_[i];
    if (i > 0 && dom_[i] >= 0) {
      dom_child_[dom_[i]].emplace_back(i);
    }
  }
}

void CFG::CalcFrontier() {
  auto size = node_pool_.size();
  frontier_.clear();
  frontier_.resize(size);
  for (uint32_t i = 1; i < size; ++i) {
    auto u = node_pool_[i];
    std::set<uint32_t> cand_;
    for (auto &v : u->preds_) {
      int32_t x = v->id_;
      while (x > 0) {
        cand_.emplace(x);
        x = dom_[x];
      }
    }
    int32_t x = dom_[u->id_];
    while (x > 0) {
      auto it = cand_.find(x);
      if (it != cand_.end()) {
        cand_.erase(it);
      }
      x = dom_[x];
    }
    for (auto &v : cand_) {
      frontier_[v].emplace(u->id_);
    }
  }
  // for (uint32_t i = 0; i < size; ++i) {
  //   std::cerr << i << ":\n";
  //   for (auto &u : frontier_[i]) {
  //     std::cerr << u << ", ";
  //   }
  //   std::cerr << '\n';
  // }
}

void CFG::AddPhi(uint32_t id_allocated, std::shared_ptr<IRArrayNode> type) {
  auto size = node_pool_.size();
  visited_block.clear();
  std::queue<uint32_t> q;
  auto add = [&] (uint32_t u) {
    for (auto &v : frontier_[u]) {
      // std::cerr << u << " " << v << '\n';
      if (visited_block.find(v) == visited_block.end()
        && node_pool_[v]->origin_->in_.find(id_allocated) != node_pool_[v]->origin_->in_.end()) {
        q.emplace(v);
        visited_block.emplace(v);
      }
    }
  };
  for (uint32_t i = 0; i < size; ++i) {
    auto u = node_pool_[i];
    if (u->origin_->def_.find(id_allocated) != u->origin_->def_.end()) {
      add(u->id_);
    }
  }
  while (!q.empty()) {
    auto u = q.front();
    q.pop();
    node_pool_[u]->origin_->AddPhi(std::make_shared<IRPhiInstructionNode>(name_allocator_.Allocate("%phi."), type));
    add(u);
  }
}

void CFG::ReplaceValue(std::string &name) {
  if (replace_map_.find(name) != replace_map_.end()) {
    name = replace_map_[name];
  }
}

void CFG::DFS(const std::string &name, uint32_t u) {
  auto block = node_pool_[u]->origin_;
  uint32_t push_cnt = 0;
  if (visited_block.find(u) != visited_block.end()) {
    current_val_.emplace(block->phi_.back()->result_);
    ++push_cnt;
  }
  for (auto &instruction : block->instructions_) {
    auto ins = instruction.get();
    auto load = dynamic_cast<IRLoadInstructionNode *>(ins);
    if (load != nullptr) {
      if (load->pointer_ == name) {
        load->removed_ = true;
        replace_map_[load->result_] = current_val_.top();
      }
      continue;
    }
    auto store_v = dynamic_cast<IRStoreVariableInstructionNode *>(ins);
    if (store_v != nullptr) {
      ReplaceValue(store_v->value_);
      if (store_v->pointer_ == name) {
        store_v->removed_ = true;
        current_val_.push(store_v->value_);
        ++push_cnt;
      }
      continue;
    }
    auto store_c = dynamic_cast<IRStoreConstInstructionNode *>(ins);
    if (store_c != nullptr) {
      if (store_c->pointer_ == name) {
        store_c->removed_ = true;
        current_val_.push(std::to_string(store_c->value_));
        ++push_cnt;
      }
    }
  }
  for (auto &v : node_pool_[u]->succs_) {
    if (visited_block.find(v->id_) != visited_block.end()) {
      v->origin_->phi_.back()->val_.emplace_back(current_val_.top(), u);
    }
  }
  for (auto &v : dom_child_[u]) {
    DFS(name, v);
  }
  while (push_cnt--) {
    current_val_.pop();
  }
}

void CFG::PhiReplace(const std::string &name) {
  while (!current_val_.empty()) {
    current_val_.pop();
  }
  replace_map_.clear();
  DFS(name, 0);
  auto size = node_pool_.size();
  for (uint32_t i = 0; i < size; ++i) {
    auto block = node_pool_[i]->origin_;
    for (auto &instruction : block->instructions_) {
      if (instruction->removed_) {
        continue;
      }
      auto ins = instruction.get();
      auto arith = dynamic_cast<IRArithmeticInstructionNode *>(ins);
      if (arith != nullptr) {
        ReplaceValue(arith->operand1_);
        ReplaceValue(arith->operand2_);
        continue;
      }
      auto neg = dynamic_cast<IRNegationInstructionNode *>(ins);
      if (neg != nullptr) {
        ReplaceValue(neg->operand_);
        continue;
      }
      auto br = dynamic_cast<IRBranchInstructionNode *>(ins);
      if (br != nullptr) {
        ReplaceValue(br->condition_);
        continue;
      }
      auto ret = dynamic_cast<IRReturnInstructionNode *>(ins);
      if (ret != nullptr) {
        if (!ret->type_->IsEmpty()) {
          ReplaceValue(ret->name_);
        }
        continue;
      }
      auto load = dynamic_cast<IRLoadInstructionNode *>(ins);
      if (load != nullptr) {
        ReplaceValue(load->pointer_);
        continue;
      }
      auto store_v = dynamic_cast<IRStoreVariableInstructionNode *>(ins);
      if (store_v != nullptr) {
        ReplaceValue(store_v->pointer_);
        if (replace_map_.find(store_v->value_) != replace_map_.end()) {
          std::cerr << "Unexpected! Store value should be replaced.\n";
          exit(-1);
        }
        continue;
      }
      auto store_c = dynamic_cast<IRStoreConstInstructionNode *>(ins);
      if (store_c != nullptr) {
        ReplaceValue(store_c->pointer_);
        continue;
      }
      auto gete = dynamic_cast<IRGetElementPtrInstructionNode *>(ins);
      if (gete != nullptr) {
        ReplaceValue(gete->ptrval_);
        continue;
      }
      auto getep = dynamic_cast<IRGetElementPtrPrimeInstructionNode *>(ins);
      if (getep != nullptr) {
        ReplaceValue(getep->ptrval_);
        ReplaceValue(getep->index_);
        continue;
      }
      auto comp = dynamic_cast<IRCompareInstructionNode *>(ins);
      if (comp != nullptr) {
        ReplaceValue(comp->operand1_);
        ReplaceValue(comp->operand2_);
        continue;
      }
      auto call = dynamic_cast<IRCallInstructionNode *>(ins);
      if (call != nullptr) {
        for (auto &argument : call->arguments_) {
          ReplaceValue(argument->value_);
        }
        continue;
      }
      auto sele = dynamic_cast<IRSelectInstructionNode *>(ins);
      if (sele != nullptr) {
        ReplaceValue(sele->cond_);
      }
    }
    for (auto &phi : block->phi_) {
      for (auto &[value, _] : phi->val_) {
        ReplaceValue(value);
      }
    }
  }
}

void CFG::EliminateCriticalEdge(IRFunctionNode *func) {
  auto size = node_pool_.size();
  auto new_id = size;
  for (uint32_t i = 0; i < size; ++i) {
    if (node_pool_[i]->succs_.size() > 1) {
      auto suc_size = node_pool_[i]->succs_.size();
      for (uint32_t j = 0; j < suc_size; ++j) {
        auto v = node_pool_[i]->succs_[j];
        if (v->preds_.size() > 1) {
          auto id = new_id++;
          auto new_IRblock = std::make_shared<IRBlockNode>(id);
          func->AddBlock(new_IRblock);

          auto new_block = std::make_shared<BlockNode>(id, new_IRblock.get());
          node_pool_.emplace_back(new_block);
          new_block->preds_.emplace_back(node_pool_[i].get());
          new_block->succs_.emplace_back(v);

          node_pool_[i]->succs_[j] = new_block.get();
          bool flag = false;
          for (auto &pre : v->preds_) {
            if (pre->id_ == node_pool_[i]->id_) {
              pre = new_block.get();
              flag = true;
              break;
            }
          }
          assert(flag);

          auto las_ins = node_pool_[i]->origin_->instructions_.back().get();
          auto br = dynamic_cast<IRBranchInstructionNode *>(las_ins);
          assert(br != nullptr);
          if (br->true_branch_ == v->id_) {
            br->true_branch_ = id;
          } else {
            assert(br->false_branch_ == v->id_);
            br->false_branch_ = id;
          }

          for (auto &phi : v->origin_->phi_) {
            for (auto &[val, fr] : phi->val_) {
              if (fr == node_pool_[i]->id_) {
                auto transfer = name_allocator_.Allocate("%transfer.");
                auto move_ins = std::make_shared<IRPhiInstructionNode>(transfer, phi->type_);
                move_ins->val_.emplace_back(val, fr);
                new_IRblock->AddPhi(move_ins);
                val = transfer;
                fr = id;
              }
            }
          }
          new_IRblock->AddInstruction(std::make_shared<IRJumpInstructionNode>(v->id_));
        }
      }
    }
  }
}
