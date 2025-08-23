#include "parser/node/AST_node.h"
#include "common/error.h"
#include <iostream>

ASTNode::ASTNode(const std::string_view &name) : name_(std::make_shared<std::string>(name)) {}

void ASTNode::CheckLength(const uint32_t &pos, const uint32_t &length) const {
  if (pos >= length) {
    throw Error(std::string("try parsing") + *name_ + std::string("Node but get keyword instead of identifier"));
  }
}

