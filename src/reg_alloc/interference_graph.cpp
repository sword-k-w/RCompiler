#include "reg_alloc/interference_graph.h"
#include <algorithm>
#include <cassert>

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

  // Make a copy of adjacency for simplify/select
  auto adj = adjacency_;
  auto saved_adj = adjacency_;  // keep original for select phase

  // --- Simplify ---
  std::set<uint32_t> active_nodes;
  for (auto &[id, _] : adj) {
    active_nodes.insert(id);
  }

  while (!active_nodes.empty()) {
    // Find a node with degree < k that is not precolored
    bool found = false;
    for (auto it = active_nodes.begin(); it != active_nodes.end(); ) {
      uint32_t id = *it;
      if (precolored_.count(id)) {
        ++it;
        continue;
      }
      uint32_t deg = adj[id].size();
      if (deg < k) {
        stack.push_back(id);
        // Remove from adjacency
        for (auto neighbor : adj[id]) {
          adj[neighbor].erase(id);
        }
        adj.erase(id);
        it = active_nodes.erase(it);
        found = true;
        break;
      }
      ++it;
    }

    if (!found) {
      // Spill: pick non-precolored node with lowest spill cost
      uint32_t best_id = 0;
      float best_cost = -1.0f;
      for (auto id : active_nodes) {
        if (precolored_.count(id)) continue;
        float cost = SpillCost(id);
        if (best_cost < 0 || cost < best_cost) {
          best_cost = cost;
          best_id = id;
        }
      }
      if (best_id == 0) break;  // shouldn't happen

      spilled.insert(best_id);
      stack.push_back(best_id);
      // Remove from adjacency
      for (auto neighbor : adj[best_id]) {
        adj[neighbor].erase(best_id);
      }
      adj.erase(best_id);
      active_nodes.erase(best_id);
    }
  }

  // --- Select ---
  // Process in reverse order (stack is LIFO)
  for (int i = stack.size() - 1; i >= 0; --i) {
    uint32_t id = stack[i];
    bool is_spilled = spilled.count(id);

    if (precolored_.count(id)) {
      // Already has a color, nothing to do
      continue;
    }

    if (is_spilled) {
      // Keep spilled — no phys_reg_ entry
      continue;
    }

    // Find a color not used by neighbors
    std::set<uint32_t> neighbor_colors;
    for (auto neighbor : saved_adj[id]) {
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
      // No color available — spill
      spilled.insert(id);
    }
  }

  return spilled;
}
