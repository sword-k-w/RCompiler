#pragma once

#include <cstdint>
#include <unordered_map>
#include <unordered_set>
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
  uint32_t Degree(uint32_t id) const;
  // Record a move pair for coalescing: src and dst should ideally share
  // a physical register to eliminate the move instruction.
  void AddMovePair(uint32_t src_id, uint32_t dst_id);
  void Coalesce(uint32_t k);
  const std::unordered_map<uint32_t, uint32_t> &GetPhysRegs() const { return phys_reg_; }

private:
  std::unordered_map<uint32_t, std::unordered_set<uint32_t>> adjacency_;
  std::unordered_map<uint32_t, uint32_t> precolored_;
  std::unordered_map<uint32_t, uint32_t> phys_reg_;
  std::unordered_map<uint32_t, uint32_t> def_count_;
  std::unordered_map<uint32_t, uint32_t> use_count_;
  std::vector<std::pair<uint32_t, uint32_t>> move_pairs_;
  std::unordered_map<uint32_t, uint32_t> coalesced_;

  float SpillCost(uint32_t id) const;
  void RemoveNode(uint32_t id);
};
