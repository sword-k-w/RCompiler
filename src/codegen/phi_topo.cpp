#include "codegen/phi_topo.h"
#include <queue>
#include <iostream>

void PhiTopo::AddEdge(const std::string &from, IRPhiInstructionNode *to) {
  edges_.emplace_back(from, to);
  if (!id_.count(from)) {
    auto new_id = id_.size();
    id_[from] = new_id;
    name_.emplace_back(from);
  }
  if (!id_.count(to->result_)) {
    auto new_id = id_.size();
    id_[to->result_] = new_id;
    name_.emplace_back(to->result_);
  }
}

PhiTopo::ResultType PhiTopo::Solve() {
  ResultType res;
  uint32_t m = edges_.size();
  uint32_t n = id_.size();
  g_.resize(n);
  deg_.assign(n, 0);
  for (uint32_t i = 0; i < m; ++i) {
    uint32_t u = id_[edges_[i].first];
    uint32_t v = id_[edges_[i].second->result_];
    g_[v] = std::make_pair(u, i);
    ++deg_[u];
  }
  std::queue<uint32_t> q;
  for (uint32_t i = 0; i < n; ++i) {
    if (!deg_[i]) {
      q.emplace(i);
    }
  }
  while (!q.empty()) {
    uint32_t u = q.front();
    q.pop();
    if (!g_[u].has_value()) continue;
    auto [v, origin] = g_[u].value();
    res.emplace_back(edges_[origin]);
    if (!--deg_[v]) {
      q.emplace(v);
    }
  }
  for (uint32_t u = 0; u < n; ++u) {
    if (deg_[u]) {
      uint32_t start = u;
      --deg_[u];
      if (!g_[u].has_value()) continue;
      auto [v_first, origin_first] = g_[u].value();
      if (v_first == u) continue; // self-loop, no-op

      res.emplace_back(edges_[origin_first].first, nullptr); // save first source to temp
      auto *tmp_phi = edges_[origin_first].second;

      u = v_first;
      while (u != start) {
        --deg_[u];
        if (!g_[u].has_value()) {
          std::cerr << "Error! broken cycle!\n";
          exit(-1);
        }
        auto [v, origin] = g_[u].value();
        res.emplace_back(edges_[origin]);
        u = v;
      }

      res.emplace_back("", tmp_phi); // temp -> first dest
    }
  }
  return res;
}

void PhiTopo::Clear() {
  edges_.clear();
  id_.clear();
  name_.clear();
  g_.clear();
}

