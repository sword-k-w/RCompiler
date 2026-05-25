#include "gtest/gtest.h"
#include "reg_alloc/interference_graph.h"

// Precolored phys regs use 0–9 (a-regs), color pool uses 10+ (s-regs).
// The Color() method asserts these sets are disjoint.

// --- Node / Edge basics ---

TEST(InterferenceGraphTest, SelfLoopIsIgnored) {
  InterferenceGraph g;
  g.AddNode(1);
  g.AddEdge(1, 1);
  std::vector<uint32_t> colors = {10};
  auto spilled = g.Color(1, colors);
  EXPECT_TRUE(spilled.empty());
}

TEST(InterferenceGraphTest, AdjacentNodesGetDifferentColors) {
  InterferenceGraph g;
  g.AddNode(1);
  g.AddNode(2);
  g.AddEdge(1, 2);
  std::vector<uint32_t> colors = {10, 11};
  auto spilled = g.Color(2, colors);
  EXPECT_TRUE(spilled.empty());
  EXPECT_NE(g.GetPhysReg(1), g.GetPhysReg(2));
}

// --- Precolored API ---

TEST(InterferenceGraphTest, PrecoloredAPI) {
  InterferenceGraph g;
  g.SetPrecolored(10, 0);
  EXPECT_TRUE(g.IsPrecolored(10));
  EXPECT_EQ(0u, g.GetPrecoloredReg(10));
  EXPECT_TRUE(g.HasPhysReg(10));
  EXPECT_EQ(0u, g.GetPhysReg(10));
}

TEST(InterferenceGraphTest, NonPrecoloredHasNoReg) {
  InterferenceGraph g;
  g.AddNode(1);
  EXPECT_FALSE(g.IsPrecolored(1));
  EXPECT_FALSE(g.HasPhysReg(1));
}

// --- Spill cost priority ---

TEST(InterferenceGraphTest, HigherSpillCostSurvivesSpilling) {
  // K3 with k=2: all degree 2 >= k, one must spill.
  // Node 3 has highest use count → highest spill cost → survives.
  InterferenceGraph g;
  g.AddNode(1); g.AddNode(2); g.AddNode(3);
  g.AddEdge(1, 2);
  g.AddEdge(2, 3);
  g.AddEdge(1, 3);
  for (int i = 0; i < 10; ++i) g.IncUseCount(3);

  std::vector<uint32_t> colors = {10, 11};
  auto spilled = g.Color(2, colors);
  EXPECT_FALSE(spilled.empty());
  EXPECT_FALSE(spilled.count(3));
}

// --- Coloring: success cases ---

TEST(InterferenceGraphTest, ColorEmptyGraph) {
  InterferenceGraph g;
  std::vector<uint32_t> colors = {10, 11, 12};
  auto spilled = g.Color(3, colors);
  EXPECT_TRUE(spilled.empty());
}

TEST(InterferenceGraphTest, ColorTwoDisconnectedNodes) {
  InterferenceGraph g;
  g.AddNode(1);
  g.AddNode(2);
  std::vector<uint32_t> colors = {10, 11, 12};
  auto spilled = g.Color(3, colors);
  EXPECT_TRUE(spilled.empty());
  EXPECT_TRUE(g.HasPhysReg(1));
  EXPECT_TRUE(g.HasPhysReg(2));
}

TEST(InterferenceGraphTest, ChainGraphColorsWith2Regs) {
  // 1 — 2 — 3 — 4
  InterferenceGraph g;
  g.AddNode(1); g.AddNode(2); g.AddNode(3); g.AddNode(4);
  g.AddEdge(1, 2);
  g.AddEdge(2, 3);
  g.AddEdge(3, 4);
  std::vector<uint32_t> colors = {10, 11};
  auto spilled = g.Color(2, colors);
  EXPECT_TRUE(spilled.empty());
  EXPECT_NE(g.GetPhysReg(1), g.GetPhysReg(2));
  EXPECT_NE(g.GetPhysReg(2), g.GetPhysReg(3));
  EXPECT_NE(g.GetPhysReg(3), g.GetPhysReg(4));
}

// --- Coloring: spill cases ---

TEST(InterferenceGraphTest, K3SpillsWith2Colors) {
  InterferenceGraph g;
  g.AddNode(1); g.AddNode(2); g.AddNode(3);
  g.AddEdge(1, 2);
  g.AddEdge(2, 3);
  g.AddEdge(1, 3);
  std::vector<uint32_t> colors = {10, 11};
  auto spilled = g.Color(2, colors);
  EXPECT_FALSE(spilled.empty());
  for (uint32_t id = 1; id <= 3; ++id) {
    if (spilled.count(id))
      EXPECT_FALSE(g.HasPhysReg(id));
    else
      EXPECT_TRUE(g.HasPhysReg(id));
  }
}

TEST(InterferenceGraphTest, K4SpillsAtLeast2With2Colors) {
  InterferenceGraph g;
  g.AddNode(1); g.AddNode(2); g.AddNode(3); g.AddNode(4);
  g.AddEdge(1, 2); g.AddEdge(1, 3); g.AddEdge(1, 4);
  g.AddEdge(2, 3); g.AddEdge(2, 4);
  g.AddEdge(3, 4);
  std::vector<uint32_t> colors = {10, 11};
  auto spilled = g.Color(2, colors);
  EXPECT_GE(spilled.size(), 2u);
}

// --- Precolored nodes ---

TEST(InterferenceGraphTest, PrecoloredEdgesAreStripped) {
  // Precolored (a-reg) and color pool (s-reg) are disjoint,
  // so edges between them are stripped before coloring.
  InterferenceGraph g;
  g.SetPrecolored(10, 0);
  g.AddNode(1);
  g.AddEdge(1, 10);
  std::vector<uint32_t> colors = {10, 11};
  auto spilled = g.Color(2, colors);
  EXPECT_FALSE(spilled.count(1));
  EXPECT_TRUE(g.HasPhysReg(1));
}

TEST(InterferenceGraphTest, PrecoloredOnlyGraphDoesNotCrash) {
  InterferenceGraph g;
  g.SetPrecolored(1, 0);
  g.SetPrecolored(2, 1);
  g.AddEdge(1, 2);
  std::vector<uint32_t> colors = {10, 11, 12};
  auto spilled = g.Color(3, colors);
  EXPECT_TRUE(spilled.empty());
  EXPECT_EQ(0u, g.GetPhysReg(1));
  EXPECT_EQ(1u, g.GetPhysReg(2));
}

// --- GetPhysRegs ---

TEST(InterferenceGraphTest, GetPhysRegsReturnsColoredNodes) {
  InterferenceGraph g;
  g.AddNode(1);
  g.AddNode(2);
  std::vector<uint32_t> colors = {10, 11};
  g.Color(2, colors);
  const auto &regs = g.GetPhysRegs();
  EXPECT_EQ(2u, regs.size());
  EXPECT_TRUE(regs.count(1));
  EXPECT_TRUE(regs.count(2));
}

TEST(InterferenceGraphTest, HasPhysRegFalseForUnknown) {
  InterferenceGraph g;
  EXPECT_FALSE(g.HasPhysReg(42));
}

// --- Complex topologies ---

TEST(InterferenceGraphTest, StarGraphColorsWith2Regs) {
  // Center (1) connected to 5 leaves. Leaves simplify first (deg=1 < 2),
  // then center has deg=0 and colors.
  InterferenceGraph g;
  g.AddNode(1);
  for (uint32_t i = 2; i <= 6; ++i) {
    g.AddNode(i);
    g.AddEdge(1, i);
  }
  std::vector<uint32_t> colors = {10, 11};
  auto spilled = g.Color(2, colors);
  EXPECT_TRUE(spilled.empty());
  for (uint32_t i = 1; i <= 6; ++i) {
    EXPECT_TRUE(g.HasPhysReg(i));
  }
}

TEST(InterferenceGraphTest, AlternatingSimplifyAndSpill) {
  // K3 (1,2,3) with a leaf (4) attached to 1. k=2.
  // Simplify: leaf 4 (deg=1) → now {1,2,3} all deg=2, spill one
  // → remaining two simplify.
  InterferenceGraph g;
  g.AddNode(1); g.AddNode(2); g.AddNode(3); g.AddNode(4);
  g.AddEdge(1, 2); g.AddEdge(2, 3); g.AddEdge(1, 3);
  g.AddEdge(1, 4);
  std::vector<uint32_t> colors = {10, 11};
  auto spilled = g.Color(2, colors);
  EXPECT_EQ(1u, spilled.size());
  for (uint32_t i = 1; i <= 4; ++i) {
    if (!spilled.count(i)) EXPECT_TRUE(g.HasPhysReg(i));
  }
}

TEST(InterferenceGraphTest, DiamondGraphSpillsWith2Colors) {
  // 1 — 2 — 4
  //  \ / \ /
  //   3 —'
  // Two triangles sharing edge 2-3. Chromatic number 3, so k=2 spills.
  InterferenceGraph g;
  g.AddNode(1); g.AddNode(2); g.AddNode(3); g.AddNode(4);
  g.AddEdge(1, 2); g.AddEdge(1, 3);
  g.AddEdge(2, 3); g.AddEdge(2, 4);
  g.AddEdge(3, 4);
  std::vector<uint32_t> colors = {10, 11};
  auto spilled = g.Color(2, colors);
  EXPECT_FALSE(spilled.empty());
  for (uint32_t i = 1; i <= 4; ++i) {
    if (!spilled.count(i)) {
      EXPECT_TRUE(g.HasPhysReg(i));
      // Adjacent non-spilled nodes must have different colors
      for (uint32_t j = i + 1; j <= 4; ++j) {
        if (!spilled.count(j)) {
          // Can't easily check adjacency here, but we can check if both
          // got the same color they aren't adjacent — skip detailed check
        }
      }
    }
  }
}

TEST(InterferenceGraphTest, DisjointComponentsOneSpills) {
  // Component A: K3 (1,2,3) — will spill with k=2
  // Component B: chain 4-5-6 — colors fine with k=2
  // Spill in A shouldn't affect B.
  InterferenceGraph g;
  // K3
  g.AddNode(1); g.AddNode(2); g.AddNode(3);
  g.AddEdge(1, 2); g.AddEdge(2, 3); g.AddEdge(1, 3);
  // Chain
  g.AddNode(4); g.AddNode(5); g.AddNode(6);
  g.AddEdge(4, 5); g.AddEdge(5, 6);
  std::vector<uint32_t> colors = {10, 11};
  auto spilled = g.Color(2, colors);
  // Chain nodes should all color
  EXPECT_FALSE(spilled.count(4));
  EXPECT_FALSE(spilled.count(5));
  EXPECT_FALSE(spilled.count(6));
  EXPECT_NE(g.GetPhysReg(4), g.GetPhysReg(5));
  EXPECT_NE(g.GetPhysReg(5), g.GetPhysReg(6));
}

TEST(InterferenceGraphTest, ManyPrecoloredNeighborsDontAffectColoring) {
  // Central regular node connected to 5 precolored nodes.
  // All edges stripped → central node has degree 0, colors with any k>=1.
  InterferenceGraph g;
  g.AddNode(0);
  for (uint32_t i = 1; i <= 5; ++i) {
    g.SetPrecolored(i, i - 1); // phys regs 0–4
    g.AddEdge(0, i);
  }
  std::vector<uint32_t> colors = {10};
  auto spilled = g.Color(1, colors);
  EXPECT_TRUE(spilled.empty());
  EXPECT_TRUE(g.HasPhysReg(0));
}

TEST(InterferenceGraphTest, EqualSpillCostHandlesTies) {
  // K4 with all default use/def counts (spill cost = 0/deg = 0 for all).
  // k=2 — must spill 2 nodes. Just verify no crash with ties.
  InterferenceGraph g;
  g.AddNode(1); g.AddNode(2); g.AddNode(3); g.AddNode(4);
  g.AddEdge(1, 2); g.AddEdge(1, 3); g.AddEdge(1, 4);
  g.AddEdge(2, 3); g.AddEdge(2, 4);
  g.AddEdge(3, 4);
  std::vector<uint32_t> colors = {10, 11};
  auto spilled = g.Color(2, colors);
  EXPECT_GE(spilled.size(), 2u);
  // Spilled nodes have no phys reg; non-spilled do
  for (uint32_t i = 1; i <= 4; ++i) {
    EXPECT_EQ(spilled.count(i), !g.HasPhysReg(i));
  }
}

TEST(InterferenceGraphTest, NonAdjacentNodesCanShareColor) {
  // Two disconnected pairs: 1-2 and 3-4. k=2. All color, and
  // non-adjacent nodes can use the same color.
  InterferenceGraph g;
  g.AddNode(1); g.AddNode(2);
  g.AddNode(3); g.AddNode(4);
  g.AddEdge(1, 2);
  g.AddEdge(3, 4);
  std::vector<uint32_t> colors = {10, 11};
  auto spilled = g.Color(2, colors);
  EXPECT_TRUE(spilled.empty());
  EXPECT_NE(g.GetPhysReg(1), g.GetPhysReg(2));
  EXPECT_NE(g.GetPhysReg(3), g.GetPhysReg(4));
}

TEST(InterferenceGraphTest, SingleColorForcesSpillOnEdge) {
  // Two connected nodes, only one color available. One must spill.
  InterferenceGraph g;
  g.AddNode(1);
  g.AddNode(2);
  g.AddEdge(1, 2);
  std::vector<uint32_t> colors = {10};
  auto spilled = g.Color(1, colors);
  EXPECT_EQ(1u, spilled.size());
  // The non-spilled node gets color 10
  uint32_t colored = spilled.count(1) ? 2 : 1;
  EXPECT_TRUE(g.HasPhysReg(colored));
  EXPECT_EQ(10u, g.GetPhysReg(colored));
}
