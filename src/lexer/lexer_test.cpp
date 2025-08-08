#include "lexer/lexer.h"
#include "data_loader/data_loader.h"

int main() {
  freopen("data.txt", "r", stdin);
  Lexer a(LoadInput());
  auto res = a.Run();
  for (auto &x : res) {
    x.Print(std::cerr);
    std::cerr << '\n';
  }
  return 0;
}