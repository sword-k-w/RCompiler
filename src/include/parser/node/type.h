#ifndef TYPE_H
#define TYPE_H

#include "lexer/lexer.h"
#include "parser/node/AST_node.h"
#include "parser/node/terminal.h"
#include "parser/node/trait.h"
#include "parser/node/path.h"

class ReferenceTypeNode : public ASTNode {
public:
  ReferenceTypeNode() = delete;
  ReferenceTypeNode(const std::vector<Token>&, uint32_t&, const uint32_t&);
private:
  bool mut_ = false;
  TypeNoBoundsNode *type_no_bounds_ = nullptr;
};

class ArrayTypeNode : public ASTNode {
public:
  ArrayTypeNode() = delete;
  ArrayTypeNode(const std::vector<Token>&, uint32_t&, const uint32_t&);
private:
  TypeNode *type_ = nullptr;
  ExpressionNode *expr_ = nullptr;
};

class SliceTypeNode : public ASTNode {
public:
  SliceTypeNode() = delete;
  SliceTypeNode(const std::vector<Token>&, uint32_t&, const uint32_t&);
private:
  TypeNode *type_ = nullptr;
};

class TypeNoBoundsNode : public ASTNode {
public:
  TypeNoBoundsNode() = delete;
  TypeNoBoundsNode(const std::vector<Token> &, uint32_t &, const uint32_t &);
private:
  TypePathNode *type_path_ = nullptr;
  ReferenceTypeNode *reference_type_ = nullptr;
  ArrayTypeNode *array_type_ = nullptr;
  SliceTypeNode *slice_type_ = nullptr;
  InferredTypeNode *inferred_type_ = nullptr;
};

#endif //TYPE_H
