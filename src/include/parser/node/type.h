#ifndef TYPE_H
#define TYPE_H

#include "lexer/lexer.h"
#include "parser/node/AST_node.h"
#include "parser/node/terminal.h"
#include "parser/node/trait.h"
#include "parser/node/path.h"

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

class SliceTypeNode : public ASTNode {
  friend class Printer;
public:
  SliceTypeNode() = delete;
  SliceTypeNode(const std::vector<Token>&, uint32_t&, const uint32_t&);
private:
  TypeNode *type_ = nullptr;
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
  SliceTypeNode *slice_type_ = nullptr;
};

#endif //TYPE_H
