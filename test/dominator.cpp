#include "liveness_analysis/dominator_tree.h"
#include <iostream>

int main() {
  DominatorTreeSolver dts;
  dts.Init(6);
  auto add = [&] (int u, int v) {
    dts.AddEdge(0, u, v);
    dts.AddEdge(1, v, u);
  };
  add(1, 2);
  add(1, 3);
  add(2, 4);
  add(2, 6);
  add(5, 3);
  dts.Tarjan(1);
  for (int i = 1; i <= 6; ++i) {
    std::cout << dts.Query(i) << '\n';
  }
  return 0;
}