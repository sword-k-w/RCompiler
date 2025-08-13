#ifndef PATTERN_H
#define PATTERN_H

#include "terminal.h"
#include "lexer/lexer.h"
#include "parser/node/AST_node.h"
#include "parser/node/terminal.h"
#include "parser/node/expression.h"

class LiteralPatternNode : public ASTNode {
public:
  LiteralPatternNode() = delete;
  LiteralPatternNode(const std::vector<Token> &, uint32_t &, const uint32_t &);
private:
  bool hyphen_ = false;
  LiteralExpressionNode *literal_expr_ = nullptr;
};

class IdentifierPatternNode : public ASTNode {
public:
  IdentifierPatternNode() = delete;
  IdentifierPatternNode(const std::vector<Token> &, uint32_t &, const uint32_t &);
private:
  bool ref_ = false;
  bool mut_ = false;
  IdentifierNode *identifier_ = nullptr;
  PatternNoTopAltNode *pattern_no_top_alt_ = nullptr;
};

using WildcardPatternNode = InferredTypeNode;

class ReferencePatternNode : public ASTNode {
public:
  ReferencePatternNode() = delete;
  ReferencePatternNode(const std::vector<Token> &, uint32_t &, const uint32_t &);
private:
  bool single_ = false;
  bool mut_ = false;
  PatternWithoutRangeNode *pattern_without_range_ = nullptr;
};

class StructPatternFieldNode : public ASTNode {
public:
  StructPatternFieldNode();
  StructPatternFieldNode(const std::vector<Token> &, uint32_t &, const uint32_t &);
private:
  IdentifierNode *identifier_ = nullptr;
  bool colon_ = false;
  PatternNode *pattern_ = nullptr;
  bool ref_ = false;
  bool mut_ = false;
};

class StructPatternFieldsNode : public ASTNode {
public:
  StructPatternFieldsNode();
  StructPatternFieldsNode(const std::vector<Token> &, uint32_t &, const uint32_t &);
private:
  std::vector<StructPatternFieldNode *> struct_pattern_field_s_;
};

using StructPatternEtCeteraNode = RestPatternNode;

class StructPatternElementsNode : public ASTNode {
public:
  StructPatternElementsNode() = delete;
  StructPatternElementsNode(const std::vector<Token> &, uint32_t &, const uint32_t &);
private:
  StructPatternFieldsNode *struct_pattern_fields_ = nullptr;
  bool comma_ = false;
  StructPatternEtCeteraNode *struct_pattern_et_cetera_ = nullptr;
};

class StructPatternNode : public ASTNode {
public:
  StructPatternNode() = delete;
  StructPatternNode(const std::vector<Token> &, uint32_t &, const uint32_t &);
private:
  PathInExpressionNode *path_in_expr_ = nullptr;
  StructPatternElementsNode *struct_pattern_elements_ = nullptr;
};

class TuplePatternItemsNode : public ASTNode {
public:
  TuplePatternItemsNode() = delete;
  TuplePatternItemsNode(const std::vector<Token> &, uint32_t &, const uint32_t &);
private:
  std::vector<PatternNode *> patterns_;
  RestPatternNode *rest_pattern_ = nullptr;
  uint32_t comma_cnt_ = 0;
};

class TuplePatternNode : public ASTNode {
public:
  TuplePatternNode() = delete;
  TuplePatternNode(const std::vector<Token> &, uint32_t &, const uint32_t &);
private:
  TuplePatternItemsNode *tuple_pattern_items_ = nullptr;
};

class GroupedPatternNode : public ASTNode {
public:
  GroupedPatternNode() = delete;
  GroupedPatternNode(const std::vector<Token> &, uint32_t &, const uint32_t &);
private:
  PatternNode *pattern_ = nullptr;
};

class SlicePatternItemsNode : public ASTNode {
public:
  SlicePatternItemsNode() = delete;
  SlicePatternItemsNode(const std::vector<Token> &, uint32_t &, const uint32_t &);
private:
  std::vector<PatternNode *> patterns_;
  uint32_t comma_cnt_ = 0;
};

class SlicePatternNode : public ASTNode {
public:
  SlicePatternNode() = delete;
  SlicePatternNode(const std::vector<Token> &, uint32_t &, const uint32_t &);
private:
  SlicePatternItemsNode *slice_pattern_items_ = nullptr;
};

using PathPatternNode = PathExpressionNode;

class PatternWithoutRangeNode : public ASTNode {
public:
  PatternWithoutRangeNode() = delete;
  PatternWithoutRangeNode(const std::vector<Token> &, uint32_t &, const uint32_t &);
private:
  LiteralPatternNode *literal_pattern_ = nullptr;
  IdentifierPatternNode *identifier_pattern_ = nullptr;
  WildcardPatternNode *wildcard_pattern_ = nullptr;
  RestPatternNode *rest_pattern_ = nullptr;
  ReferencePatternNode *reference_pattern_ = nullptr;
  StructPatternNode *struct_pattern_ = nullptr;
  TuplePatternNode *tuple_pattern_ = nullptr;
  GroupedPatternNode *grouped_pattern_ = nullptr;
  SlicePatternNode *slice_pattern_ = nullptr;
  PathPatternNode *path_pattern_ = nullptr;
};

class RangePatternNode : public ASTNode {
public:
  RangePatternNode() = delete;
  RangePatternNode(const std::vector<Token> &, uint32_t &, const uint32_t &);
};

class PatternNoTopAltNode : public ASTNode {
public:
  PatternNoTopAltNode() = delete;
  PatternNoTopAltNode(const std::vector<Token> &, uint32_t &, const uint32_t &);
private:
  PatternWithoutRangeNode *pattern_without_range_ = nullptr;
  RangePatternNode *range_pattern_ = nullptr;
};

class PatternNode : public ASTNode {
public:
  PatternNode() = delete;
  PatternNode(const std::vector<Token> &, uint32_t &, const uint32_t &);
private:
  std::vector<PatternNoTopAltNode *> pattern_no_top_alts_;
  uint32_t or_cnt_ = 0;
};

#endif //PATTERN_H
