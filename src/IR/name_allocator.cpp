#include "IR/name_allocator.h"

std::string NameAllocator::Allocate(const std::string &raw_name) {
  int32_t cnt = ++repeat_cnt_[raw_name];
  return raw_name + "." + std::to_string(cnt);
}
