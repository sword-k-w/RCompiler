#pragma once
#include <cstdint>
#include <memory>
#include <set>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

class IRRootNode;
class IRInstructionNode;
class IRFunctionNode;
class IRArrayNode;

/// Local value numbering for pure address computations (GEP / GEPP).
/// Eliminates redundant pointer arithmetic so that a single base pointer
/// computed via GEPP(base, idx) is reused for all field accesses on the
/// same element, and nested chains like a[i].x[j].y / a[i].x[j].z share
/// the common prefix.
class CSE {
public:
  static void Run(std::shared_ptr<IRRootNode> root);

private:
  static std::string TypeEncode(const std::shared_ptr<IRArrayNode> &type);
  static bool ReplaceOperands(
      IRInstructionNode *ins,
      const std::unordered_map<std::string, std::string> &subst);
  static std::pair<std::string, std::string> TryMakeKey(IRInstructionNode *ins);
  static void ApplySubstitutions(
      std::shared_ptr<IRFunctionNode> func,
      const std::unordered_map<std::string, std::string> &subst);

};
