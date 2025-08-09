#include "parser/crate.h"

CrateNode::CrateNode(const std::vector<Token> &tokens, uint32_t &pos, const uint32_t &length) {
  while (pos < length) {
    items_.push_back(new ItemNode(tokens, pos, length));
  }
}
