#ifndef PATH_H
#define PATH_H

#include "lexer/lexer.h"
#include "parser/node/AST_node.h"
#include "parser/node/generic.h"
#include "parser/node/terminal.h"
#include "parser/node/type.h"

class SimplePathSegmentNode : public ASTNode {
public:
  SimplePathSegmentNode() = delete;
  SimplePathSegmentNode(const std::vector<Token>&, uint32_t&, const uint32_t&);
};

class PathIdentSegmentNode : public ASTNode {
public:
  PathIdentSegmentNode() = delete;
  PathIdentSegmentNode(const std::vector<Token>&, uint32_t&, const uint32_t&);
private:
  IdentifierNode *identifier_ = nullptr;
  SuperNode *super_ = nullptr;
  SelfLowerNode *self_lower_ = nullptr;
  SelfUpperNode *self_upper_ = nullptr;
  CrateValNode *crate_val_ = nullptr;
};

class TypePathFnInputsNode : public ASTNode {
public:
  TypePathFnInputsNode() = delete;
  TypePathFnInputsNode(const std::vector<Token>&, uint32_t&, const uint32_t&);
private:
  std::vector<TypeNode *> types_;
  uint32_t comma_cnt_ = 0;
};

class TypePathFnNode : public ASTNode {
public:
  TypePathFnNode() = delete;
  TypePathFnNode(const std::vector<Token>&, uint32_t&, const uint32_t&);
private:
  TypePathFnInputsNode *type_path_fn_inputs_ = nullptr;
  TypeNoBoundsNode *type_no_bounds_ = nullptr;
};

class TypePathSegmentNode : public ASTNode {
public:
  TypePathSegmentNode() = delete;
  TypePathSegmentNode(const std::vector<Token>&, uint32_t&, const uint32_t&);
private:
  PathIdentSegmentNode *path_ident_segment_ = nullptr;
  bool colon_ = false;
  GenericArgsNode *generic_args_ = nullptr;
  TypePathFnNode *type_path_fn_ = nullptr;
};

class TypePathNode : public ASTNode {
public:
  TypePathNode() = delete;
  TypePathNode(const std::vector<Token>&, uint32_t&, const uint32_t&);
private:
  uint32_t colon_cnt_ = 0;
  std::vector<TypePathSegmentNode *> type_path_segments_;
};

class QualifiedPathTypeNode : public ASTNode {
public:
  QualifiedPathTypeNode() = delete;
  QualifiedPathTypeNode(const std::vector<Token>&, uint32_t&, const uint32_t&);
private:
  TypeNode *type_ = nullptr;
  TypePathNode *type_path_ = nullptr;
};

class QualifiedPathInTypeNode : public ASTNode {
public:
  QualifiedPathInTypeNode() = delete;
  QualifiedPathInTypeNode(const std::vector<Token>&, uint32_t&, const uint32_t&);
private:
  QualifiedPathTypeNode *qualified_path_type_ = nullptr;
  std::vector<TypePathSegmentNode *> type_path_segments_;
};

#endif //PATH_H
