#ifndef TRAIT_H
#define TRAIT_H

#include "lexer/lexer.h"
#include "parser/node/AST_node.h"
#include "parser/node/terminal.h"
#include "parser/node/generic.h"
#include "parser/node/item.h"

class TraitBoundNode : public ASTNode {
public:
  TraitBoundNode() = delete;
  TraitBoundNode(const std::vector<Token> &, uint32_t &, const uint32_t &);
private:
  bool parenthesis_ = false;
  bool question_ = false;
  TypePathNode *type_path_ = nullptr;
};

class UseBoundGenericArgNode : public ASTNode {
public:
  UseBoundGenericArgNode() = delete;
  UseBoundGenericArgNode(const std::vector<Token> &, uint32_t &, const uint32_t &);
private:
  IdentifierNode *identifier_ = nullptr;
  SelfUpperNode *self_upper_ = nullptr;
};

class UseBoundGenericArgsNode : public ASTNode {
public:
  UseBoundGenericArgsNode() = delete;
  UseBoundGenericArgsNode(const std::vector<Token> &, uint32_t &, const uint32_t &);
private:
  std::vector<UseBoundGenericArgNode *> use_bound_generic_arg_s_;
  uint32_t comma_cnt_ = 0;
};

class UseBoundNode : public ASTNode {
public:
  UseBoundNode() = delete;
  UseBoundNode(const std::vector<Token> &, uint32_t &, const uint32_t &);
  UseBoundGenericArgsNode *use_bound_generic_args_ = nullptr;
};

class TypeParamBoundNode : public ASTNode {
public:
  TypeParamBoundNode() = delete;
  TypeParamBoundNode(const std::vector<Token> &, uint32_t &, const uint32_t &);
private:
  TraitBoundNode *trait_bound_ = nullptr;
  UseBoundNode *use_bound_ = nullptr;
};

class TypeParamBoundsNode : public ASTNode {
public:
  TypeParamBoundsNode() = delete;
  TypeParamBoundsNode(const std::vector<Token> &, uint32_t &, const uint32_t &);
private:
  std::vector<TypeParamBoundNode *> type_param_bound_s_;
  uint32_t plus_cnt_ = 0;
};

class TraitNode : public ASTNode {
public:
  TraitNode() = delete;
  TraitNode(const std::vector<Token> &, uint32_t &, const uint32_t &);
private:
  IdentifierNode *identifier_ = nullptr;
  bool colon_ = false;
  TypeParamBoundsNode *type_param_bounds_ = nullptr;
  std::vector<AsscociatedItemNode *> asscociated_items_;
};

#endif //TRAIT_H
