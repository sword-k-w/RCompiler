#pragma once

#include "parser/class_declaration.h"
#include "lexer/lexer.h"
#include "parser/node/AST_node.h"
#include <cstdint>

class StructFieldNode : public ASTNode {
  friend class Printer;
public:
  StructFieldNode() = delete;
  StructFieldNode(const std::vector<Token> &, uint32_t &, const uint32_t &);
private:
  IdentifierNode *identifier_ = nullptr;
  TypeNode *type_ = nullptr;
};

class StructFieldsNode : public ASTNode {
  friend class Printer;
public:
  StructFieldsNode() = delete;
  StructFieldsNode(const std::vector<Token> &, uint32_t &, const uint32_t &);
private:
  std::vector<StructFieldNode *> struct_field_s_;
  uint32_t comma_cnt_ = 0;
};

class StructNode : public ASTNode {
  friend class Printer;
public:
  StructNode() = delete;
  StructNode(const std::vector<Token> &, uint32_t &, const uint32_t &);
private:
  IdentifierNode *identifier_ = nullptr;
  StructFieldsNode *struct_fields_ = nullptr;
  bool semicolon_ = false;
};
