// https://oi-wiki.org/graph/dominator-tree/

#include "liveness_analysis/dominator_tree.h"

void DominatorTreeSolver::Init(uint32_t _n) {
  n = _n;
  tim = 0;
  edge_cnt = 0;

  dfn.resize(n + 1);
  pos.resize(n + 1);
  fa.resize(n + 1);
  mn.resize(n + 1);
  idm.resize(n + 1);
  sdm.resize(n + 1);
  pa.resize(n + 1);
  head[0].resize(n + 1);
  head[1].resize(n + 1);
  head[2].resize(n + 1);
  e.clear();
  e.emplace_back(0, 0);

  for (uint32_t i = 1; i <= n; ++i) {
    dfn[i] = 0;
    pos[i] = 0;
    fa[i] = 0;
    mn[i] = i;
    sdm[i] = i;
    pa[i] = i;
    head[0][i] = 0;
    head[1][i] = 0;
    head[2][i] = 0;
  }
}

void DominatorTreeSolver::AddEdge(uint32_t o, uint32_t u, uint32_t v) {
  e.emplace_back(v, head[o][u]);
  head[o][u] = ++edge_cnt;
}

void DominatorTreeSolver::DFS(uint32_t u) {
  dfn[u] = ++tim;
  pos[tim] = u;
  for (uint32_t i = head[0][u]; i; i = e[i].nxt) {
    uint32_t v = e[i].v;
    if (!dfn[v]) {
      DFS(v);
      fa[v] = u;
    }
  }
}

uint32_t DominatorTreeSolver::Find(uint32_t x) {
  if (pa[x] == x) {
    return x;
  }
  int tmp = pa[x];
  pa[x] = Find(pa[x]);
  if (dfn[sdm[mn[tmp]]] < dfn[sdm[mn[x]]]) {
    mn[x] = mn[tmp];
  }
  return pa[x];
}

void DominatorTreeSolver::Tarjan(uint32_t s) {
  DFS(s);
  for (uint32_t i = tim; i >= 2; --i) {
    uint32_t u = pos[i];
    uint32_t res = n + 1;
    for (uint32_t j = head[1][u]; j; j = e[j].nxt) {
      uint32_t v = e[j].v;
      if (!dfn[v]) {
        continue;
      }
      Find(v);
      if (dfn[v] < dfn[u]) {
        res = std::min(res, dfn[v]);
      } else {
        res = std::min(res, dfn[sdm[mn[v]]]);
      }
    }
    sdm[u] = pos[res];
    pa[u] = fa[u];
    AddEdge(2, sdm[u], u);
    u = fa[u];
    for (uint32_t j = head[2][u]; j; j = e[j].nxt) {
      uint32_t v = e[j].v;
      Find(v);
      if (sdm[mn[v]] == u) {
        idm[v] = u;
      } else {
        idm[v] = mn[v];
      }
    }
    head[2][u] = 0;
  }
  for (uint32_t i = 2; i <= tim; ++i) {
    uint32_t u = pos[i];
    if (idm[u] != sdm[u]) {
      idm[u] = idm[idm[u]];
    }
  }
}

uint32_t DominatorTreeSolver::Query(uint32_t u) {
  return idm[u];
}
