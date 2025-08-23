#include "parser/node/crate.h"
#include "parser/node/item.h"
#include "common/error.h"

CrateNode::CrateNode(const std::vector<Token> &tokens, uint32_t &pos, const uint32_t &length) : ASTNode("Crate") {
  try {
    while (pos < length) {
      items_.push_back(std::make_shared<ItemNode>(tokens, pos, length));
    }
  } catch (Error &err) {
    throw err;
  }
}
