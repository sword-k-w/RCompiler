#include "parser/node/crate.h"

CrateNode::CrateNode(const std::vector<Token> &tokens, uint32_t &pos, const uint32_t &length) : ASTNode("Crate") {
  while (pos < length) {
    items_.push_back(node_pool.Make<ItemNode>(tokens, pos, length));
  }
}
