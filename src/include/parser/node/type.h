#ifndef TYPE_H
#define TYPE_H

#include "lexer/lexer.h"
#include "parser/node/AST_node.h"
#include "parser/node/terminal.h"
#include "parser/node/trait.h"
#include "parser/node/path.h"

class ParenthesizedTypeNode : public ASTNode {
public:
  ParenthesizedTypeNode() = delete;
  ParenthesizedTypeNode(const std::vector<Token> &, uint32_t &, const uint32_t &);
private:
  TypeNode *type_;
};

class ImplTraitTypeOneBoundNode : public ASTNode {
public:
  ImplTraitTypeOneBoundNode() = delete;
  ImplTraitTypeOneBoundNode(const std::vector<Token>&, uint32_t&, const uint32_t&);
private:
  TraitBoundNode *trait_bound_ = nullptr;
};

class TraitObjectTypeOneBoundNode : public ASTNode {
public:
  TraitObjectTypeOneBoundNode() = delete;
  TraitObjectTypeOneBoundNode(const std::vector<Token>&, uint32_t&, const uint32_t&);
private:
  TraitBoundNode *trait_bound_ = nullptr;
};

class TupleTypeNode : public ASTNode {
public:
  TupleTypeNode() = delete;
  TupleTypeNode(const std::vector<Token>&, uint32_t&, const uint32_t&);
private:
  std::vector<TypeNode *> types_;
  uint32_t comma_cnt_ = 0;
};

class RawPointerTypeNode : public ASTNode {
public:
  RawPointerTypeNode() = delete;
  RawPointerTypeNode(const std::vector<Token>&, uint32_t&, const uint32_t&);
private:
  bool mut_ = false;
  TypeNoBoundsNode *type_no_bounds_ = nullptr;
};

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

class QualifiedPathInTypeNode : public ASTNode {
public:
  QualifiedPathInTypeNode() = delete;
  QualifiedPathInTypeNode(const std::vector<Token>&, uint32_t&, const uint32_t&);
private:
};

class BareFunctionTypeNode : public ASTNode {
public:
  BareFunctionTypeNode() = delete;
  BareFunctionTypeNode(const std::vector<Token>&, uint32_t&, const uint32_t&);
private:
};

class TypeNoBoundsNode : public ASTNode {
public:
  TypeNoBoundsNode() = delete;
  TypeNoBoundsNode(const std::vector<Token> &, uint32_t &, const uint32_t &);
private:
  ParenthesizedTypeNode *parenthesized_type_ = nullptr;
  ImplTraitTypeOneBoundNode *impl_trait_type_one_bound_ = nullptr;
  TraitObjectTypeOneBoundNode *trait_object_type_one_bound_ = nullptr;
  TypePathNode *type_path_ = nullptr;
  TupleTypeNode *tuple_type_ = nullptr;
  NeverTypeNode *never_type_ = nullptr;
  RawPointerTypeNode *raw_pointer_type_ = nullptr;
  ReferenceTypeNode *reference_type_ = nullptr;
  ArrayTypeNode *array_type_ = nullptr;
  SliceTypeNode *slice_type_ = nullptr;
  InferredTypeNode *inferred_type_ = nullptr;
  QualifiedPathInTypeNode *qualified_path_in_type_ = nullptr;
  BareFunctionTypeNode *bare_function_type_ = nullptr;
};

class ImplTraitTypeNode : public ASTNode {
public:
  ImplTraitTypeNode() = delete;
  ImplTraitTypeNode(const std::vector<Token> &, uint32_t &, const uint32_t &);
private:
  TypeParamBoundsNode *type_param_bounds_ = nullptr;
};

class TraitObjectTypeNode : public ASTNode {
public:
  TraitObjectTypeNode() = delete;
  TraitObjectTypeNode(const std::vector<Token> &, uint32_t &, const uint32_t &);
private:
  TypeParamBoundsNode *type_param_bounds_ = nullptr;
};

class TypeNode : public ASTNode {
public:
  TypeNode() = delete;
  TypeNode(const std::vector<Token> &, uint32_t &, const uint32_t &);
private:
  TypeNoBoundsNode *type_no_bounds_ = nullptr;
  ImplTraitTypeNode *impl_trait_type_ = nullptr;
  TraitObjectTypeNode *trait_object_type_ = nullptr;
};

#endif //TYPE_H
