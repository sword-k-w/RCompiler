#pragma once

#include <cstdint>
#include <map>
#include <set>
#include <vector>

class InterferenceGraph {
public:
  InterferenceGraph() = default;

  void AddNode(uint32_t id);
  void AddEdge(uint32_t u, uint32_t v);
  void SetPrecolored(uint32_t id, uint32_t phys_reg);
  bool IsPrecolored(uint32_t id) const;
  uint32_t GetPrecoloredReg(uint32_t id) const;

  void IncDefCount(uint32_t id);
  void IncUseCount(uint32_t id);

  // Returns the set of spilled variable IDs.
  // k = number of allocatable registers, colors = list of phys reg IDs.
  std::set<uint32_t> Color(uint32_t k, const std::vector<uint32_t> &colors);

  uint32_t GetPhysReg(uint32_t id) const;
  bool HasPhysReg(uint32_t id) const;
  const std::map<uint32_t, uint32_t> &GetPhysRegs() const { return phys_reg_; }

private:
  std::map<uint32_t, std::set<uint32_t>> adjacency_;
  std::map<uint32_t, uint32_t> precolored_;
  std::map<uint32_t, uint32_t> phys_reg_;
  std::map<uint32_t, uint32_t> def_count_;
  std::map<uint32_t, uint32_t> use_count_;

  uint32_t Degree(uint32_t id) const;
  float SpillCost(uint32_t id) const;
  void RemoveNode(uint32_t id);
};
