#pragma once
#include <map>
#include <string>

#include "IR/IR_node.h"

class FunctionMap {
private:
  FunctionMap() = default;
  std::map<std::string, IRFunctionNode *> mp_;
public:
  FunctionMap(const FunctionMap &) = delete;
  FunctionMap &operator = (const FunctionMap &) = delete;
  static FunctionMap &Instance() {
    static FunctionMap instance;
    return instance;
  }
  void Add(const std::string &, IRFunctionNode *);
  IRFunctionNode *Query(const std::string &);
};
