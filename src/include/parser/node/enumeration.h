#ifndef ENUMERATION_H
#define ENUMERATION_H

#include "lexer/lexer.h"
#include "parser/node/AST_node.h"
#include "parser/node/terminal.h"
#include "parser/node/generic.h"
#include "parser/node/struct.h"

class EnumVariantStructNode : public ASTNode {
public:
  EnumVariantStructNode() = delete;
  EnumVariantStructNode(const std::vector<Token> &, uint32_t &, const uint32_t &);
private:
  StructFieldsNode *struct_fields_ = nullptr;
};

class EnumVariantDiscriminantNode : public ASTNode {
public:
  EnumVariantDiscriminantNode() = delete;
  EnumVariantDiscriminantNode(const std::vector<Token> &, uint32_t &, const uint32_t &);
private:
  ExpressionNode *expr_ = nullptr;
};

class EnumVariantNode : public ASTNode {
public:
  EnumVariantNode() = delete;
  EnumVariantNode(const std::vector<Token> &, uint32_t &, const uint32_t &);
private:
  IdentifierNode *identifier_ = nullptr;
  EnumVariantStructNode *enum_variant_struct_ = nullptr;
  EnumVariantDiscriminantNode *enum_variant_discriminant_ = nullptr;
};

class EnumVariantsNode : public ASTNode {
public:
  EnumVariantsNode() = delete;
  EnumVariantsNode(const std::vector<Token> &, uint32_t &, const uint32_t &);
private:
  std::vector<EnumVariantNode *> enum_variant_s_;
  uint32_t comma_cnt_ = 0;
};

class EnumerationNode : public ASTNode {
public:
  EnumerationNode() = delete;
  EnumerationNode(const std::vector<Token> &, uint32_t &, const uint32_t &);
private:
  IdentifierNode *identifier_ = nullptr;
  GenericParamsNode *generic_params_ = nullptr;
  WhereClauseNode *where_clause_ = nullptr;
  EnumVariantsNode *enum_variants_ = nullptr;
};

#endif //ENUMERATION_H
