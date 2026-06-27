#pragma once

#include <memory>

class IRRootNode;

class ParameterDemoter {
public:
  static void Run(std::shared_ptr<IRRootNode> root);
  static void FixupAfterRegAlloc(std::shared_ptr<IRRootNode> root);

private:
  static void RunOnFunction(class IRFunctionNode *func);
  static void RenameInIns(class IRInstructionNode *ins,
                          const std::string &from, const std::string &to);
};
