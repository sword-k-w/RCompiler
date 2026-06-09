#pragma once

#include <vector>
#include <string>
#include <map>
#include <memory>
#include <stack>
#include "IR/IR_node.h"
#include "IR/name_allocator.h"
class CFG {
private:
  struct VariableMap {
    std::map<std::string, uint32_t> id_;
    std::map<std::string, uint32_t> allocated_id_;
    std::vector<std::pair<bool, std::string>> name_; // false means allocated
    std::vector<std::vector<IRInstructionNode *>> def_;
    std::vector<std::vector<IRInstructionNode *>> use_;
    std::vector<std::set<uint32_t>> def_blocks_;
    void Clear();
    uint32_t Query(const std::string &);
    uint32_t QueryAllocated(const std::string &);
    std::pair<bool, std::string> GetName(uint32_t);
  };

  VariableMap variable_map_;

  struct BlockNode {
    uint32_t id_;
    std::vector<BlockNode *> succs_;
    std::vector<BlockNode *> preds_;

    IRBlockNode *origin_;
    BlockNode() = delete;
    BlockNode(uint32_t, IRBlockNode *);
  };
  std::vector<std::shared_ptr<BlockNode>> node_pool_;
  std::vector<int32_t> dom_;
  std::vector<std::vector<uint32_t>> dom_child_;
  std::vector<std::set<uint32_t>> frontier_;

  // Per-block index: pointer_name -> instructions that reference it (in block order)
  std::vector<std::map<std::string, std::vector<IRInstructionNode *>>> block_ins_index_;

  NameAllocator name_allocator_;

  std::set<uint32_t> visited_block;
  std::stack<std::string> current_val_;
  std::map<std::string, std::string> replace_map_;
  std::map<std::string, std::string> phi_to_var_;  // phi result -> alloca name

  void ReplaceValue(std::string &);
  void DFS(const std::string &, uint32_t, uint32_t);
  void BatchedDFSRecursive(uint32_t, std::map<std::string, std::stack<std::string>> &,
                           const std::set<std::string> &);
public:
  CFG() = default;
  void Init(uint32_t);
  void NewNode(uint32_t, IRBlockNode *);
  uint32_t Query(const std::string &);
  uint32_t QueryAllocated(const std::string &);
  std::pair<bool, std::string> GetName(uint32_t);
  void AddEdge(uint32_t, uint32_t);
  void AddDef(uint32_t, IRInstructionNode *);
  void AddDefBlock(uint32_t, uint32_t);
  void AddUse(uint32_t, IRInstructionNode *);
  void CalcInOut();

  void CalcDominatorTree();
  void CalcFrontier();
  void AddPhi(uint32_t, std::shared_ptr<IRArrayNode>);
  void BuildInsIndex(IRFunctionNode *);
  void PhiReplace(const std::string &);
  void PhiDFS(const std::string &, uint32_t);
  void BatchedPhiDFS(const std::set<std::string> &);
  void PhiRewriteAll();
  void EliminateCriticalEdge(IRFunctionNode *);
};