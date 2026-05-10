#include "liveness_analysis/dominator_tree.h"
#include <iostream>

int main() {
  DominatorTreeSolver dts;
  dts.Init(4);
  auto add = [&] (int u, int v) {
    dts.AddEdge(0, u, v);
    dts.AddEdge(1, v, u);
  };
  add(1, 3);
  add(3, 2);
  add(2, 3);
  add(3, 4);
  dts.Tarjan(1);
  for (int i = 1; i <= 4; ++i) {
    std::cout << dts.Query(i) << '\n';
  }
  return 0;
}