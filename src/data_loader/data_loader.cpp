#include "data_loader/data_loader.h"
#include <iostream>
#include <fstream>
#include <sstream>

auto LoadInput() -> std::string {
  char ch;
  std::string res;
  while (std::cin.get(ch)) {
    res += ch;
  }
  return res;
}

auto LoadFromFile(const std::string &s) -> std::string {
  std::ifstream file(s, std::ios::binary);
  if (!file.is_open()) {
    throw std::runtime_error("Failed to open file: " + s);
  }

  std::ostringstream ss;
  ss << file.rdbuf();
  return ss.str();
}
