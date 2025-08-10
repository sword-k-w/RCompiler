#include "parser/node/AST_node.h"

ASTNode::ASTNode(const std::string_view &name) : name_(name) {}

void ASTNode::CheckLength(const uint32_t &pos, const uint32_t &length) const {
  if (pos >= length) {
    Error(std::string("try parsing") + name_ + std::string("Node but get keyword instead of identifier"));
  }
}

