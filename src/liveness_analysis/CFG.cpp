#include "liveness_analysis/CFG.h"
#include "liveness_analysis/dominator_tree.h"

#include <cassert>
#include <functional>
#include <queue>
#include <iostream>

void CFG::VariableMap::Clear() {
  id_.clear();
  allocated_id_.clear();
  name_.clear();
  def_.clear();
  use_.clear();
  def_blocks_.clear();
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
  def_blocks_.emplace_back();
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
  def_blocks_.emplace_back();
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

void CFG::AddDefBlock(uint32_t id, uint32_t block_id) {
  variable_map_.def_blocks_[id].emplace(block_id);
}

void CFG::AddUse(uint32_t id, IRInstructionNode *node) {
  variable_map_.use_[id].emplace_back(node);
}

void CFG::CalcInOut() {
  auto size = node_pool_.size();
  if (size == 0) {
    return;
  }

  uint32_t var_cnt = variable_map_.name_.size();

  // Resize and clear all in/out bitsets
  for (uint32_t i = 0; i < size; ++i) {
    node_pool_[i]->origin_->in_.Resize(var_cnt);
    node_pool_[i]->origin_->out_.Resize(var_cnt);
  }

  // Compute postorder via DFS from entry block.
  std::vector<uint32_t> postorder;
  postorder.reserve(size);
  std::vector<bool> visited(size, false);
  std::function<void(uint32_t)> dfs = [&](uint32_t u) {
    visited[u] = true;
    for (auto &v : node_pool_[u]->succs_) {
      if (!visited[v->id_]) {
        dfs(v->id_);
      }
    }
    postorder.push_back(u);
  };
  dfs(0);

  std::queue<uint32_t> worklist;
  std::vector<bool> in_worklist(size, false);

  BitSet new_out, new_in;
  new_out.Resize(var_cnt);
  new_in.Resize(var_cnt);

  auto process = [&](uint32_t u) {
    auto block = node_pool_[u]->origin_;

    // new_out = U IN[succ]
    new_out.Clear();
    for (auto &v : node_pool_[u]->succs_) {
      new_out.Union(v->origin_->in_);
    }

    // new_in = new_out, then clear def bits, set use bits
    new_in.Copy(new_out);
    for (auto &x : block->def_) {
      new_in.ClearBit(x);
    }
    for (auto &x : block->use_) {
      new_in.Set(x);
    }

    if (new_out != block->out_ || new_in != block->in_) {
      block->out_ = new_out;
      block->in_ = new_in;

      for (auto &pred : node_pool_[u]->preds_) {
        if (!in_worklist[pred->id_]) {
          worklist.push(pred->id_);
          in_worklist[pred->id_] = true;
        }
      }
    }
  };

  // First pass: postorder
  for (uint32_t u : postorder) {
    process(u);
  }

  // Second pass: worklist for residual from back edges
  while (!worklist.empty()) {
    uint32_t u = worklist.front();
    worklist.pop();
    in_worklist[u] = false;
    process(u);
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
  visited_block.clear();
  auto var_name = variable_map_.GetName(id_allocated).second;
  std::queue<uint32_t> q;
  auto add = [&] (uint32_t u) {
    for (auto &v : frontier_[u]) {
      if (visited_block.find(v) == visited_block.end()
        && node_pool_[v]->origin_->in_.Test(id_allocated)) {
        q.emplace(v);
        visited_block.emplace(v);
      }
    }
  };
  auto &def_blocks = variable_map_.def_blocks_[id_allocated];
  for (auto block_id : def_blocks) {
    add(block_id);
  }
  while (!q.empty()) {
    auto u = q.front();
    q.pop();
    auto phi = std::make_shared<IRPhiInstructionNode>(name_allocator_.Allocate("%phi."), type);
    node_pool_[u]->origin_->AddPhi(phi);
    phi_to_var_[phi->result_] = var_name;
    add(u);
  }
}

void CFG::ReplaceValue(std::string &name) {
  auto it = replace_map_.find(name);
  if (it == replace_map_.end()) return;
  // Walk chained replacements until reaching a key not in the map.
  // Chains form because a DFS may push an unresolved load result from
  // a variable processed later; transitive closure fixes this.
  while (true) {
    name = it->second;
    it = replace_map_.find(name);
    if (it == replace_map_.end()) break;
  }
}

void CFG::BuildInsIndex(IRFunctionNode *) {
  auto size = node_pool_.size();
  block_ins_index_.clear();
  block_ins_index_.resize(size);
  for (uint32_t i = 0; i < size; ++i) {
    auto &index = block_ins_index_[i];
    auto block = node_pool_[i]->origin_;
    for (auto &instruction : block->instructions_) {
      auto ins = instruction.get();
      if (auto *load = dynamic_cast<IRLoadInstructionNode *>(ins)) {
        index[load->pointer_].push_back(ins);
      } else if (auto *store_v = dynamic_cast<IRStoreVariableInstructionNode *>(ins)) {
        index[store_v->pointer_].push_back(ins);
      } else if (auto *store_c = dynamic_cast<IRStoreConstInstructionNode *>(ins)) {
        index[store_c->pointer_].push_back(ins);
      }
    }
  }
}

void CFG::DFS(const std::string &name, uint32_t u, uint32_t id_allocated) {
  auto block = node_pool_[u]->origin_;
  uint32_t push_cnt = 0;
  if (visited_block.find(u) != visited_block.end()) {
    current_val_.emplace(block->phi_.back()->result_);
    ++push_cnt;
  }

  auto &index = block_ins_index_[u];
  auto it = index.find(name);
  if (it != index.end()) {
    for (auto ins : it->second) {
      if (auto *load = dynamic_cast<IRLoadInstructionNode *>(ins)) {
        load->removed_ = true;
        replace_map_[load->result_] = current_val_.top();
      } else if (auto *store_v = dynamic_cast<IRStoreVariableInstructionNode *>(ins)) {
        ReplaceValue(store_v->value_);
        store_v->removed_ = true;
        current_val_.push(store_v->value_);
        ++push_cnt;
      } else if (auto *store_c = dynamic_cast<IRStoreConstInstructionNode *>(ins)) {
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
    DFS(name, v, id_allocated);
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
  DFS(name, 0, 0);
  PhiRewriteAll();
}
void CFG::PhiDFS(const std::string &name, uint32_t id_allocated) {
  while (!current_val_.empty()) {
    current_val_.pop();
  }
  DFS(name, 0, id_allocated);
}

void CFG::BatchedDFSRecursive(uint32_t u,
    std::map<std::string, std::stack<std::string>> &stacks,
    const std::set<std::string> &promoted) {
  auto block = node_pool_[u]->origin_;
  std::map<std::string, uint32_t> local_pushes;

  auto do_push = [&](const std::string &var, const std::string &val) {
    stacks[var].push(val);
    ++local_pushes[var];
  };

  // Process phi nodes
  for (auto &phi : block->phi_) {
    auto it = phi_to_var_.find(phi->result_);
    if (it != phi_to_var_.end()) {
      do_push(it->second, phi->result_);
    }
  }

  // Process instructions via index — only for promoted variables
  auto &index = block_ins_index_[u];
  for (auto &[pointer_name, ins_list] : index) {
    if (!promoted.count(pointer_name)) continue;
    for (auto ins : ins_list) {
      if (auto *load = dynamic_cast<IRLoadInstructionNode *>(ins)) {
        auto &stack = stacks[pointer_name];
        if (!stack.empty()) {
          load->removed_ = true;
          replace_map_[load->result_] = stack.top();
        }
      } else if (auto *store_v = dynamic_cast<IRStoreVariableInstructionNode *>(ins)) {
        ReplaceValue(store_v->value_);
        store_v->removed_ = true;
        do_push(pointer_name, store_v->value_);
      } else if (auto *store_c = dynamic_cast<IRStoreConstInstructionNode *>(ins)) {
        store_c->removed_ = true;
        do_push(pointer_name, std::to_string(store_c->value_));
      }
    }
  }

  // Add current values to successor phis
  for (auto &v : node_pool_[u]->succs_) {
    for (auto &phi : v->origin_->phi_) {
      auto var_it = phi_to_var_.find(phi->result_);
      if (var_it != phi_to_var_.end()) {
        auto &stack = stacks[var_it->second];
        if (!stack.empty()) {
          phi->val_.emplace_back(stack.top(), u);
        }
      }
    }
  }

  // Recurse into children
  for (auto &v : dom_child_[u]) {
    BatchedDFSRecursive(v, stacks, promoted);
  }

  // Pop locally pushed values
  for (auto &[var, cnt] : local_pushes) {
    auto &stack = stacks[var];
    while (cnt--) {
      stack.pop();
    }
  }
}

void CFG::BatchedPhiDFS(const std::set<std::string> &promoted) {
  replace_map_.clear();
  std::map<std::string, std::stack<std::string>> stacks;
  BatchedDFSRecursive(0, stacks, promoted);
}

void CFG::PhiRewriteAll() {
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
        ReplaceValue(store_v->value_);
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
        continue;
      }
      auto move = dynamic_cast<IRMoveInstructionNode *>(ins);
      if (move != nullptr) {
        ReplaceValue(move->source_);
        // Note: result_ is defined by this instruction, so it is not
        // rewritten — only the source operand is replaced.
        continue;
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
