#include "parser/node/expression.h"
#include "parser/node/statement.h"
#include "parser/node/function.h"
#include "parser/node/struct.h"
#include "parser/node/crate.h"
#include "parser/node/path.h"
#include "parser/node/pattern.h"
#include "lexer/lexer.h"
#include "data_loader/data_loader.h"

int main() {

  freopen("data.txt", "r", stdin);
  Lexer a(LoadInput());
  auto res = a.Run();
  try {
    uint32_t pos = 0;
    CrateNode *crate = node_pool.Make<CrateNode>(res, pos, res.size());
  } catch (Error &err) {
    std::cerr << err.Info() << '\n';
  } catch (...) {
    std::cerr << "unexpected error!";
  }
  return 0;
}