#pragma once

#include "parser/class_declaration.h"
#include "lexer/lexer.h"
#include "parser/node/AST_node.h"
#include <cstdint>
#include <memory>

class CrateNode : public ASTNode {
  friend class Printer;
public:
  CrateNode() = delete;
  CrateNode(const std::vector<Token> &, uint32_t &, const uint32_t &);
private:
  std::vector<std::shared_ptr<ItemNode>> items_;
};
