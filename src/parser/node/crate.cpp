#include "parser/node/crate.h"
#include "common/error.h"
#include "parser/node_pool.h"

CrateNode::CrateNode(const std::vector<Token> &tokens, uint32_t &pos, const uint32_t &length) : ASTNode("Crate") {
  try {
    while (pos < length) {
      items_.push_back(node_pool.Make<ItemNode>(tokens, pos, length));
    }
  } catch (Error &err) {
    throw err;
  }
}
