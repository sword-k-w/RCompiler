#pragma once
#include <map>
#include <string>

#include "IR/IR_node.h"

class FunctionMap {
private:
  FunctionMap();
  std::map<std::string, IRFunctionNode *> mp_;
  std::vector<std::shared_ptr<IRFunctionNode>> builtin_func_;
  void AddBuiltinFunction(const std::string &, const std::string &, const std::vector<std::pair<std::string, std::string>> &);
public:
  FunctionMap(const FunctionMap &) = delete;
  FunctionMap &operator = (const FunctionMap &) = delete;
  static FunctionMap &Instance() {
    static FunctionMap instance;
    return instance;
  }
  void Add(const std::string &, IRFunctionNode *);
  IRFunctionNode *Query(const std::string &);
  void Accept(IRVisitorBase *);
};
