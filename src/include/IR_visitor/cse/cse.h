#pragma once
#include <cstdint>
#include <memory>
#include <set>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

class IRRootNode;
class IRInstructionNode;
class IRFunctionNode;
class IRArrayNode;
class IRBlockNode;

/// Local value numbering for pure address computations (GEP / GEPP).
/// Eliminates redundant pointer arithmetic so that a single base pointer
/// computed via GEPP(base, idx) is reused for all field accesses on the
/// same element, and nested chains like a[i].x[j].y / a[i].x[j].z share
/// the common prefix.
class CSE {
public:
  static void Run(std::shared_ptr<IRRootNode> root);

private:
  using MoveDefMap = std::unordered_map<uint32_t, std::unordered_set<std::string>>;

  static std::string TypeEncode(const std::shared_ptr<IRArrayNode> &type);
  static bool ReplaceOperands(
      IRInstructionNode *ins,
      const std::unordered_map<std::string, std::string> &subst);
  static std::pair<std::string, std::string> TryMakeKey(
      IRInstructionNode *ins,
      uint32_t cur_block_id,
      size_t cur_ins_idx,
      const MoveDefMap &move_defs,
      const std::unordered_set<std::string> &all_move_names,
      const std::unordered_map<uint32_t, uint32_t> &ir_to_pos,
      const std::vector<int32_t> &idom,
      const std::vector<std::shared_ptr<IRBlockNode>> &blocks);
  static void ApplySubstitutions(
      std::shared_ptr<IRFunctionNode> func,
      const std::unordered_map<std::string, std::string> &subst);

  static MoveDefMap BuildMoveDefs(std::shared_ptr<IRFunctionNode> func,
                                  std::unordered_set<std::string> &all_move_names);

  static std::string EncodeOperand(
      const std::string &name,
      uint32_t cur_block_id,
      size_t cur_ins_idx,
      const MoveDefMap &move_defs,
      const std::unordered_set<std::string> &all_move_names,
      const std::unordered_map<uint32_t, uint32_t> &ir_to_pos,
      const std::vector<int32_t> &idom,
      const std::vector<std::shared_ptr<IRBlockNode>> &blocks);

};
