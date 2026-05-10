#pragma once

#include <cstdint>
#include <vector>

class DominatorTreeSolver {
private:
  uint32_t n;
  uint32_t tim;
  uint32_t edge_cnt;
  std::vector<uint32_t> dfn;
  std::vector<uint32_t> pos;
  std::vector<uint32_t> fa;
  std::vector<uint32_t> mn;
  std::vector<uint32_t> idm;
  std::vector<uint32_t> sdm;
  std::vector<uint32_t> pa;

  struct Edge {
    int v, nxt;
  };
  std::vector<Edge> e;
  std::vector<uint32_t> head[3];
  void DFS(uint32_t);
  uint32_t Find(uint32_t);
public:
  DominatorTreeSolver() = default;
  void Init(uint32_t);
  void AddEdge(uint32_t, uint32_t, uint32_t);
  void Tarjan(uint32_t);
  uint32_t Query(uint32_t);
};