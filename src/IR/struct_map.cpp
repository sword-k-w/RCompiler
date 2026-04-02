#include "IR/struct_map.h"

void StructMap::Add(const std::string &name, IRStructNode *node) {
  mp_[name] = node;
}

IRStructNode *StructMap::Query(const std::string &name) {
  return mp_[name];
}
