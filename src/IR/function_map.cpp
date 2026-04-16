#include "IR/function_map.h"

void FunctionMap::Add(const std::string &name, IRFunctionNode *node) {
  mp_[name] = node;
}

IRFunctionNode *FunctionMap::Query(const std::string &name) {
  return mp_[name];
}
