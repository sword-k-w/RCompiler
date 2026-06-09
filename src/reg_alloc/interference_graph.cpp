#include "reg_alloc/interference_graph.h"
#include <algorithm>
#include <cassert>
#include <cmath>
#include <iostream>
#include <queue>
#include <unordered_set>

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

  // Build initial active set and worklist for nodes with degree < k
  std::unordered_set<uint32_t> active_nodes;
  std::queue<uint32_t> worklist;
  for (auto &[id, _] : adj) {
    active_nodes.insert(id);
    if (adj[id].size() < k) {
      worklist.push(id);
    }
  }

  // Strip precolored nodes
  for (auto &[id, _] : precolored_) {
    active_nodes.erase(id);
    for (auto neighbor : adj[id]) {
      adj[neighbor].erase(id);
      adjacency_[neighbor].erase(id);
    }
    adj[id].clear();
  }

  // Rebuild worklist: stripping precolored nodes may have reduced
  // some neighbors' degrees below k.
  {
    std::queue<uint32_t> empty;
    std::swap(worklist, empty);
    for (auto nid : active_nodes) {
      if (adj[nid].size() < k) {
        worklist.push(nid);
      }
    }
  }

  // --- Simplify ---
  while (!active_nodes.empty()) {
    uint32_t id;
    if (!worklist.empty()) {
      // Pop from worklist: degree < k
      id = worklist.front();
      worklist.pop();
      if (!active_nodes.count(id)) continue;  // already removed
    } else {
      // Spill: pick any active node (scanning all nodes for optimal
      // spill cost is O(V) per iteration, which is too slow for dense
      // interference graphs). Any node works correctly.
      id = *active_nodes.begin();
      spilled.insert(id);
    }

    stack.push_back(id);
    for (auto neighbor : adj[id]) {
      adj[neighbor].erase(id);
      // If neighbor degree just dropped below k, add to worklist
      if (adj[neighbor].size() + 1 == k && adj[neighbor].size() < k) {
        worklist.push(neighbor);
      }
    }
    adj.erase(id);
    active_nodes.erase(id);
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
