#ifndef ENUMERATION_H
#define ENUMERATION_H

#include "lexer/lexer.h"
#include "parser/node/AST_node.h"
#include "parser/node/terminal.h"
#include "parser/node/generic.h"
#include "parser/node/struct.h"

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

#endif //ENUMERATION_H
