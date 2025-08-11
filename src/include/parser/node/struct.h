#ifndef STRUCT_H
#define STRUCT_H

#include "lexer/lexer.h"
#include "parser/node/AST_node.h"
#include "parser/node/terminal.h"
#include "parser/node/generic.h"
#include "parser/node/type.h"

class StructFieldNode : public ASTNode {
public:
  StructFieldNode() = delete;
  StructFieldNode(const std::vector<Token> &, uint32_t &, const uint32_t &);
private:
  IdentifierNode *identifier_ = nullptr;
  TypeNode *type_ = nullptr;
};

class StructFieldsNode : public ASTNode {
public:
  StructFieldsNode() = delete;
  StructFieldsNode(const std::vector<Token> &, uint32_t &, const uint32_t &);
private:
  std::vector<StructFieldNode *> struct_field_s_;
  uint32_t comma_cnt_ = 0;
};

class TupleFieldNode : public ASTNode {
public:
  TupleFieldNode() = delete;
  TupleFieldNode(const std::vector<Token> &, uint32_t &, const uint32_t &);
private:
  TypeNode *type_ = nullptr;
};

class TupleFieldsNode : public ASTNode {
public:
  TupleFieldsNode() = delete;
  TupleFieldsNode(const std::vector<Token> &, uint32_t &, const uint32_t &);
private:
  std::vector<TupleFieldNode *> tuple_field_s_;
  uint32_t comma_cnt_ = 0;
};

class StructNode : public ASTNode {
public:
  StructNode() = delete;
  StructNode(const std::vector<Token> &, uint32_t &, const uint32_t &);
private:
  IdentifierNode *identifier_ = nullptr;
  GenericParamsNode *generic_params_ = nullptr;
  WhereClauseNode *where_clause_ = nullptr;
  StructFieldsNode *struct_fields_ = nullptr;
  TupleFieldsNode *tuple_fields_ = nullptr;
  bool semicolon_ = false;
};

#endif //STRUCT_H
