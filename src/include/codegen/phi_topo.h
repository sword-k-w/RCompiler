#pragma once
#include <string>
#include <map>
#include <optional>
#include "IR/IR_node.h"

class PhiTopo {
  using ResultType = std::vector<std::pair<std::string, IRPhiInstructionNode *>>;
public:
  void AddEdge(const std::string &, IRPhiInstructionNode *);
  ResultType Solve(); // string == "" or ptr == nullptr means need t reg to transfer (the dependence relationship forms a cycle)
  void Clear();
private:
  ResultType edges_; // (from, to(phi))
  std::map<std::string, uint32_t> id_;
  std::vector<std::string> name_;
  std::vector<std::optional<std::pair<uint32_t, uint32_t>>> g_;
  std::vector<uint32_t> deg_;
};