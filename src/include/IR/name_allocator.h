#pragma once
#include <string>
#include <map>
#include <cstdint>

class NameAllocator {
public:
  NameAllocator() = default;
  std::string Allocate(const std::string &);
private:
  std::map<std::string, int32_t> repeat_cnt_;
};