#pragma once

#include "parser/class_declaration.h"
#include "lexer/lexer.h"
#include "parser/node/AST_node.h"
#include <string>
#include <cstdint>

class EnumVariantsNode : public ASTNode {
  friend class Printer;
public:
  EnumVariantsNode() = delete;
  EnumVariantsNode(const std::vector<Token> &, uint32_t &, const uint32_t &);
private:
  std::vector<EnumVariantNode *> enum_variant_s_;
  uint32_t comma_cnt_ = 0;
};

class EnumerationNode : public ASTNode {
  friend class Printer;
public:
  EnumerationNode() = delete;
  EnumerationNode(const std::vector<Token> &, uint32_t &, const uint32_t &);
private:
  IdentifierNode *identifier_ = nullptr;
  EnumVariantsNode *enum_variants_ = nullptr;
};

