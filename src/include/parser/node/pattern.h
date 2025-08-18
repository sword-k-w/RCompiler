#ifndef PATTERN_H
#define PATTERN_H

#include "terminal.h"
#include "lexer/lexer.h"
#include "parser/node/AST_node.h"
#include "parser/node/terminal.h"
#include "parser/node/expression.h"

class LiteralPatternNode : public ASTNode {
  friend class Printer;
public:
  LiteralPatternNode() = delete;
  LiteralPatternNode(const std::vector<Token> &, uint32_t &, const uint32_t &);
private:
  bool hyphen_ = false;
  LiteralExpressionNode *literal_expr_ = nullptr;
};

class IdentifierPatternNode : public ASTNode {
  friend class Printer;
public:
  IdentifierPatternNode() = delete;
  IdentifierPatternNode(const std::vector<Token> &, uint32_t &, const uint32_t &);
private:
  bool ref_ = false;
  bool mut_ = false;
  IdentifierNode *identifier_ = nullptr;
  PatternNoTopAltNode *pattern_no_top_alt_ = nullptr;
};

class ReferencePatternNode : public ASTNode {
  friend class Printer;
public:
  ReferencePatternNode() = delete;
  ReferencePatternNode(const std::vector<Token> &, uint32_t &, const uint32_t &);
private:
  bool single_ = false;
  bool mut_ = false;
  PatternWithoutRangeNode *pattern_without_range_ = nullptr;
};

class TupleStructItemsNode : public ASTNode {
  friend class Printer;
public:
  TupleStructItemsNode() = delete;
  TupleStructItemsNode(const std::vector<Token> &, uint32_t &, const uint32_t &);
private:
  std::vector<PatternNode *> patterns_;
  uint32_t comma_cnt_ = 0;
};

class TupleStructPatternNode : public ASTNode {
  friend class Printer;
public:
  TupleStructPatternNode() = delete;
  TupleStructPatternNode(const std::vector<Token> &, uint32_t &, const uint32_t &);
private:
  PathInExpressionNode *path_in_expr_ = nullptr;
  TupleStructItemsNode *tuple_struct_items_ = nullptr;
};

class PatternWithoutRangeNode : public ASTNode {
  friend class Printer;
public:
  PatternWithoutRangeNode() = delete;
  PatternWithoutRangeNode(const std::vector<Token> &, uint32_t &, const uint32_t &);
private:
  LiteralPatternNode *literal_pattern_ = nullptr;
  IdentifierPatternNode *identifier_pattern_ = nullptr;
  WildcardPatternNode *wildcard_pattern_ = nullptr;
  ReferencePatternNode *reference_pattern_ = nullptr;
  TupleStructPatternNode *tuple_struct_pattern_ = nullptr;
  PathPatternNode *path_pattern_ = nullptr;
};
#endif //PATTERN_H
