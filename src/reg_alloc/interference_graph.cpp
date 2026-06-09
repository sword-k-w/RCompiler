#include "reg_alloc/interference_graph.h"
#include <algorithm>
#include <cassert>
#include <iostream>

void InterferenceGraph::AddNode(uint32_t id) {
  adjacency_[id];  // ensure entry exists
}

void InterferenceGraph::AddEdge(uint32_t u, uint32_t v) {
  if (u == v) return;
  adjacency_[u].insert(v);
  adjacency_[v].insert(u);
}

void InterferenceGraph::SetPrecolored(uint32_t id, uint32_t phys_reg) {
  precolored_[id] = phys_reg;
  phys_reg_[id] = phys_reg;
  AddNode(id);
}

bool InterferenceGraph::IsPrecolored(uint32_t id) const {
  return precolored_.count(id);
}

uint32_t InterferenceGraph::GetPrecoloredReg(uint32_t id) const {
  return precolored_.at(id);
}

void InterferenceGraph::IncDefCount(uint32_t id) {
  ++def_count_[id];
}

void InterferenceGraph::IncUseCount(uint32_t id) {
  ++use_count_[id];
}

uint32_t InterferenceGraph::Degree(uint32_t id) const {
  auto it = adjacency_.find(id);
  if (it == adjacency_.end()) return 0;
  return it->second.size();
}

float InterferenceGraph::SpillCost(uint32_t id) const {
  uint32_t uses = use_count_.count(id) ? use_count_.at(id) : 0;
  uint32_t defs = def_count_.count(id) ? def_count_.at(id) : 0;
  uint32_t deg = Degree(id);
  if (deg == 0) return 0.0f;
  return static_cast<float>(uses + defs) / deg;
}

void InterferenceGraph::RemoveNode(uint32_t id) {
  for (auto neighbor : adjacency_[id]) {
    adjacency_[neighbor].erase(id);
  }
  adjacency_.erase(id);
}

uint32_t InterferenceGraph::GetPhysReg(uint32_t id) const {
  return phys_reg_.at(id);
}

bool InterferenceGraph::HasPhysReg(uint32_t id) const {
  return phys_reg_.count(id);
}

std::set<uint32_t> InterferenceGraph::Color(uint32_t k, const std::vector<uint32_t> &colors) {
  std::set<uint32_t> spilled;
  std::vector<uint32_t> stack;

  auto adj = adjacency_;

  // --- Simplify ---
  std::set<uint32_t> active_nodes;
  for (auto &[id, _] : adj) {
    active_nodes.insert(id);
  }

  // Precolored nodes use a-registers, regular nodes use s-registers —
  // disjoint sets, so edges to precolored nodes don't constrain coloring.
  for (auto &[id, _] : precolored_) {
    active_nodes.erase(id);
    for (auto neighbor : adj[id]) {
      adj[neighbor].erase(id);
      adjacency_[neighbor].erase(id);
    }
    adj[id].clear();
  }

  // Verify precolored phys regs and color pool are disjoint.
  std::set<uint32_t> precolored_regs;
  for (auto &[_, reg] : precolored_) {
    precolored_regs.insert(reg);
  }
  std::set<uint32_t> color_pool(colors.begin(), colors.end());
  std::vector<uint32_t> overlap;
  std::set_intersection(precolored_regs.begin(), precolored_regs.end(),
                        color_pool.begin(), color_pool.end(),
                        std::back_inserter(overlap));
  assert(overlap.empty() && "precolored regs and color pool must be disjoint");

  while (!active_nodes.empty()) {
    // Find a node with degree < k
    bool found = false;
    for (auto it = active_nodes.begin(); it != active_nodes.end(); ++it) {
      uint32_t id = *it;
      if (adj[id].size() < k) {
        stack.push_back(id);
        for (auto neighbor : adj[id]) {
          adj[neighbor].erase(id);
        }
        adj.erase(id);
        active_nodes.erase(it);
        found = true;
        break;
      }
    }

    if (!found) {
      // Spill: pick node with lowest spill cost
      uint32_t best_id = *active_nodes.begin();
      float best_cost = SpillCost(best_id);
      for (auto id : active_nodes) {
        float cost = SpillCost(id);
        if (cost < best_cost) {
          best_cost = cost;
          best_id = id;
        }
      }

      spilled.insert(best_id);
      stack.push_back(best_id);
      for (auto neighbor : adj[best_id]) {
        adj[neighbor].erase(best_id);
      }
      adj.erase(best_id);
      active_nodes.erase(best_id);
    }
  }

  // --- Select ---
  for (int i = stack.size() - 1; i >= 0; --i) {
    uint32_t id = stack[i];
    if (spilled.count(id)) continue;

    std::set<uint32_t> neighbor_colors;
    for (auto neighbor : adjacency_[id]) {
      if (phys_reg_.count(neighbor)) {
        neighbor_colors.insert(phys_reg_[neighbor]);
      }
    }

    bool assigned = false;
    for (auto color : colors) {
      if (!neighbor_colors.count(color)) {
        phys_reg_[id] = color;
        assigned = true;
        break;
      }
    }

    if (!assigned) {
      spilled.insert(id);
    }
  }

  return spilled;
}
