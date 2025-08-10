#include "parser/error.h"
#include <iostream>

void Error(const std::string_view &info) {
  std::cerr << "Error!\n" << info << '\n';
  node_pool.Clear();
  exit(-1);
}