#pragma once

#include "parser/class_declaration.h"
#include "lexer/lexer.h"
#include "parser/node/AST_node.h"
#include <cstdint>
#include <memory>

class LiteralPatternNode : public ASTNode {
  friend class Printer;
  friend class FirstChecker;
public:
  LiteralPatternNode() = delete;
  LiteralPatternNode(const std::vector<Token> &, uint32_t &, const uint32_t &);
  void Accept(VisitorBase *) override;
private:
  bool hyphen_ = false;
  std::shared_ptr<LiteralExpressionNode> literal_expr_;
};

class IdentifierPatternNode : public ASTNode {
  friend class Printer;
  friend class FirstChecker;
public:
  IdentifierPatternNode() = delete;
  IdentifierPatternNode(const std::vector<Token> &, uint32_t &, const uint32_t &);
  void Accept(VisitorBase *) override;
private:
  bool ref_ = false;
  bool mut_ = false;
  std::shared_ptr<IdentifierNode> identifier_;
  std::shared_ptr<PatternNoTopAltNode> pattern_no_top_alt_;
};

class ReferencePatternNode : public ASTNode {
  friend class Printer;
  friend class FirstChecker;
public:
  ReferencePatternNode() = delete;
  ReferencePatternNode(const std::vector<Token> &, uint32_t &, const uint32_t &);
  void Accept(VisitorBase *) override;
private:
  bool single_ = false;
  bool mut_ = false;
  std::shared_ptr<PatternWithoutRangeNode> pattern_without_range_;
};

class TupleStructItemsNode : public ASTNode {
  friend class Printer;
  friend class FirstChecker;
public:
  TupleStructItemsNode() = delete;
  TupleStructItemsNode(const std::vector<Token> &, uint32_t &, const uint32_t &);
  void Accept(VisitorBase *) override;
private:
  std::vector<std::shared_ptr<PatternNode>> patterns_;
  uint32_t comma_cnt_ = 0;
};

class TupleStructPatternNode : public ASTNode {
  friend class Printer;
  friend class FirstChecker;
public:
  TupleStructPatternNode() = delete;
  TupleStructPatternNode(const std::vector<Token> &, uint32_t &, const uint32_t &);
  void Accept(VisitorBase *) override;
private:
  std::shared_ptr<PathInExpressionNode> path_in_expr_;
  std::shared_ptr<TupleStructItemsNode> tuple_struct_items_;
};

class PatternWithoutRangeNode : public ASTNode {
  friend class Printer;
  friend class FirstChecker;
public:
  PatternWithoutRangeNode() = delete;
  PatternWithoutRangeNode(const std::vector<Token> &, uint32_t &, const uint32_t &);
  void Accept(VisitorBase *) override;
private:
  std::shared_ptr<LiteralPatternNode> literal_pattern_;
  std::shared_ptr<IdentifierPatternNode> identifier_pattern_;
  std::shared_ptr<WildcardPatternNode> wildcard_pattern_;
  std::shared_ptr<ReferencePatternNode> reference_pattern_;
  std::shared_ptr<TupleStructPatternNode> tuple_struct_pattern_;
  std::shared_ptr<PathPatternNode> path_pattern_;
};
