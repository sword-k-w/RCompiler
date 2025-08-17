#include "parser/error.h"
#include <iostream>

Error::Error(const std::string &info) : info_(info) {}

std::string Error::Info() const {
  return info_;
}


// void Error(const std::string_view &info) {
//   std::cerr << "Error!\n" << info << '\n';
//   node_pool.Clear();
//   exit(-1);
// }