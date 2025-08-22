#pragma once

#include "parser/class_declaration.h"
#include "lexer/lexer.h"
#include "parser/node/AST_node.h"
#include <cstdint>

class ReferenceTypeNode : public ASTNode {
  friend class Printer;
public:
  ReferenceTypeNode() = delete;
  ReferenceTypeNode(const std::vector<Token>&, uint32_t&, const uint32_t&);
private:
  bool mut_ = false;
  TypeNoBoundsNode *type_no_bounds_ = nullptr;
};

class ArrayTypeNode : public ASTNode {
  friend class Printer;
public:
  ArrayTypeNode() = delete;
  ArrayTypeNode(const std::vector<Token>&, uint32_t&, const uint32_t&);
private:
  TypeNode *type_ = nullptr;
  ExpressionNode *expr_ = nullptr;
};

class TypeNoBoundsNode : public ASTNode {
  friend class Printer;
public:
  TypeNoBoundsNode() = delete;
  TypeNoBoundsNode(const std::vector<Token> &, uint32_t &, const uint32_t &);
private:
  TypePathNode *type_path_ = nullptr;
  ReferenceTypeNode *reference_type_ = nullptr;
  ArrayTypeNode *array_type_ = nullptr;
  UnitTypeNode *unit_type_ = nullptr;
};
