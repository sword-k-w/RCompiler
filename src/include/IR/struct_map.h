#pragma once
#include <map>
#include <string>

#include "IR/IR_node.h"

class StructMap {
private:
  StructMap() = default;
  std::map<std::string, IRStructNode *> mp_;
public:
  StructMap(const StructMap &) = delete;
  StructMap &operator = (const StructMap &) = delete;
  static StructMap &Instance() {
    static StructMap instance;
    return instance;
  }
  void Add(const std::string &, IRStructNode *);
  IRStructNode *Query(const std::string &);
  void Print();
};
