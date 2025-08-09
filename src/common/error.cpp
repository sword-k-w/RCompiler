#include "common/error.h"
#include <iostream>

void Error(const std::string_view &info) {
  std::cerr << "Error!\n" << info << '\n';
  exit(-1);
}