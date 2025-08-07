#include "lexer/lexer.h"

int main() {
  Lexer a("hello world r###\"114514  rjrao\"###cr####\"\"####");
  auto res = a.Run();
  for (auto &x : res) {
    x.Print(std::cerr);
    std::cerr << '\n';
  }
  return 0;
}