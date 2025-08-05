#include "data_loader/data_loader.h"
#include <iostream>

auto LoadInput() -> std::string {
  char ch;
  std::string res;
  while (std::cin.get(ch)) {
    res += ch;
  }
  return res;
}