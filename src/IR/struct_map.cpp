#include "IR/struct_map.h"
#include <iostream>

void StructMap::Add(const std::string &name, IRStructNode *node) {
  mp_[name] = node;
}

IRStructNode *StructMap::Query(const std::string &name) {
  return mp_[name];
}

void StructMap::Clear() {
  mp_.clear();
}

void StructMap::Print() {
  std::cerr << "[structs in StructMap]\n";
  for (auto [name, node] : mp_) {
    std::cerr << name << '\n';
  }
}

