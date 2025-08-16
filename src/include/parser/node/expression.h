#ifndef EXPRESSION_H
#define EXPRESSION_H

#include <string>
#include "parser/node/AST_node.h"
#include "parser/node/terminal.h"
#include "parser/node/statement.h"

class LiteralExpressionNode : public ASTNode {
public:
  LiteralExpressionNode() = delete;
  LiteralExpressionNode(const std::vector<Token> &, uint32_t &, const uint32_t &);
private:
  CharLiteralNode *char_literal_ = nullptr;
  StringLiteralNode *string_literal_ = nullptr;
  RawStringLiteralNode *raw_string_literal_ = nullptr;
  CStringLiteralNode *c_string_literal_ = nullptr;
  RawCStringLiteralNode *raw_c_string_literal_ = nullptr;
  IntegerLiteralNode *integer_literal_ = nullptr;
  FloatLiteralNode *float_literal_ = nullptr;
  TrueNode *true_ = nullptr;
  FalseNode *false_ = nullptr;
};

class BorrowExpressionNode : public ASTNode {

private:
};

class NegationExpressionNode : public ASTNode {

private:
};

class ArithmeticOrLogicalExpressionNode : public ASTNode {

};

class ComparisonExpressionNode : public ASTNode {

};

class LazyBooleanExpressionNode : public ASTNode {

};

class AssignmentExpressionNode : public ASTNode {

};

class CompoundAssignmentExpressionNode : public ASTNode {

};

class OperatorExpressionNode : public ASTNode {

private:
  BorrowExpressionNode *borrow_expr_ = nullptr;
  NegationExpressionNode *negation_expr_ = nullptr;
  ArithmeticOrLogicalExpressionNode *arithmetic_or_logical_expr_ = nullptr;
  ComparisonExpressionNode *comparison_expr_ = nullptr;
  LazyBooleanExpressionNode *lazy_boolean_expr_ = nullptr;
  AssignmentExpressionNode *assignment_expr_ = nullptr;
  CompoundAssignmentExpressionNode *compound_assignment_expr_ = nullptr;
};

class GroupedExpressionNode : public ASTNode {

private:
};

class ArrayElementsNode : public ASTNode {
public:
  ArrayElementsNode() = delete;
  ArrayElementsNode(const std::vector<Token> &, uint32_t &, const uint32_t &);
private:
  std::vector<ExpressionNode *> exprs_;
  bool semicolon_ = false;
};

class ArrayExpressionNode : public ASTNode {
public:
  ArrayExpressionNode() = delete;
  ArrayExpressionNode(const std::vector<Token> &, uint32_t &, const uint32_t &);
private:
  ArrayElementsNode *array_elements_ = nullptr;
};

class IndexExpressionNode : public ASTNode {

private:
};

class PathInExpressionNode : public ASTNode {
public:
  PathInExpressionNode() = delete;
  PathInExpressionNode(const std::vector<Token> &, uint32_t &, const uint32_t &);
private:
  std::vector<PathExprSegmentNode *> path_expr_segments_;
};

class StructExprFieldNode : public ASTNode {
public:
  StructExprFieldNode() = delete;
  StructExprFieldNode(const std::vector<Token> &, uint32_t &, const uint32_t &);
private:
  IdentifierNode *identifier_ = nullptr;
  ExpressionNode *expr_ = nullptr;
};

class StructExprFieldsNode : public ASTNode {
public:
  StructExprFieldsNode() = delete;
  StructExprFieldsNode(const std::vector<Token> &, uint32_t &, const uint32_t &);
private:
  std::vector<StructExprFieldNode *> struct_expr_field_s_;
  uint32_t comma_cnt = 0;
};

class StructExpressionNode : public ASTNode {
public:
  StructExpressionNode() = delete;
  StructExpressionNode(const std::vector<Token> &, uint32_t &, const uint32_t &);
private:
  PathInExpressionNode *path_in_expr = nullptr;
  StructExprFieldsNode *struct_expr_fields_ = nullptr;
};

class CallExpressionNode : public ASTNode {

private:
};

class MethodCallExpressionNode : public ASTNode {

private:
};

class FieldExpressionNode : public ASTNode {

private:
};

class BreakExpressionNode : public ASTNode {
public:
  BreakExpressionNode() = delete;
  BreakExpressionNode(const std::vector<Token> &, uint32_t &, const uint32_t &);
private:
  ExpressionNode *expr_ = nullptr;
};

class ReturnExpressionNode : public ASTNode {
public:
  ReturnExpressionNode() = delete;
  ReturnExpressionNode(const std::vector<Token> &, uint32_t &, const uint32_t &);
private:
  ExpressionNode *expr_ = nullptr;
};

class ExpressionWithoutBlockNode : public ASTNode {
private:
  LiteralExpressionNode *literal_expr_ = nullptr;
  PathExpressionNode *path_expr_ = nullptr;
  OperatorExpressionNode *operator_expr_ = nullptr;
  GroupedExpressionNode *grouped_expr_ = nullptr;
  ArrayExpressionNode *array_expr_ = nullptr;
  IndexExpressionNode *index_expr_ = nullptr;
  StructExpressionNode *struct_expr_ = nullptr;
  CallExpressionNode *call_expr_ = nullptr;
  MethodCallExpressionNode *method_call_expr_ = nullptr;
  FieldExpressionNode *field_expr_ = nullptr;
  ContinueExpressionNode *continue_expr_ = nullptr;
  BreakExpressionNode *break_expr_ = nullptr;
  ReturnExpressionNode *return_expr_ = nullptr;
  UnderscoreExpressionNode *underscore_expr_ = nullptr;
};

class BlockExpressionNode : public ASTNode {
public:
  BlockExpressionNode() = delete;
  BlockExpressionNode(const std::vector<Token> &, uint32_t &, const uint32_t &);
private:
  StatementsNode *statements_s_ = nullptr;
};

class ConstBlockExpressionNode : public ASTNode {
public:
  ConstBlockExpressionNode() = delete;
  ConstBlockExpressionNode(const std::vector<Token> &, uint32_t &, const uint32_t &);
private:
  BlockExpressionNode *block_expr_ = nullptr;
};

class InfiniteLoopExpressionNode : public ASTNode {
public:
  InfiniteLoopExpressionNode() = delete;
  InfiniteLoopExpressionNode(const std::vector<Token> &, uint32_t &, const uint32_t &);
private:
  BlockExpressionNode *block_expr_ = nullptr;
};

class ConditionsNode : public ASTNode {
public:
  ConditionsNode() = delete;
  ConditionsNode(const std::vector<Token> &, uint32_t &, const uint32_t &);
private:
};

class PredicateLoopExpressionNode : public ASTNode {
public:
  PredicateLoopExpressionNode() = delete;
  PredicateLoopExpressionNode(const std::vector<Token> &, uint32_t &, const uint32_t &);
private:
  ConditionsNode *conditions_ = nullptr;
  BlockExpressionNode *block_expr = nullptr;
};

class LoopExpressionNode : public ASTNode {
public:
  LoopExpressionNode() = delete;
  LoopExpressionNode(const std::vector<Token> &, uint32_t &, const uint32_t &);
private:
  InfiniteLoopExpressionNode *infinite_loop_expr_ = nullptr;
  PredicateLoopExpressionNode *predicate_loop_expr_ = nullptr;
};

class IfExpressionNode : public ASTNode {
public:
  IfExpressionNode() = delete;
  IfExpressionNode(const std::vector<Token> &, uint32_t &, const uint32_t &);
private:
  ConditionsNode *conditions_ = nullptr;
  BlockExpressionNode *block_expr1_ = nullptr;
  BlockExpressionNode *block_expr2_ = nullptr;
  IfExpressionNode *if_expr_ = nullptr;
};

class ScrutineeNode : public ASTNode {
public:
  ScrutineeNode() = delete;
  ScrutineeNode(const std::vector<Token> &, uint32_t &, const uint32_t &);
private:
  ExpressionNode *expr_ = nullptr;
};

class MatchArmsNode : public ASTNode {
public:
  MatchArmsNode() = delete;
  MatchArmsNode(const std::vector<Token> &, uint32_t &, const uint32_t &);
private:
};

class MatchExpressionNode : public ASTNode {
public:
  MatchExpressionNode() = delete;
  MatchExpressionNode(const std::vector<Token> &, uint32_t &, const uint32_t &);
private:
  ScrutineeNode *scrutinee_ = nullptr;
  MatchArmsNode *match_arms_ = nullptr;
};

class ExpressionWithBlockNode : public ASTNode {

private:
  BlockExpressionNode *block_expr_ = nullptr;
  ConstBlockExpressionNode *const_block_expr_ = nullptr;
  LoopExpressionNode *loop_expr_ = nullptr;
  IfExpressionNode *if_expr_ = nullptr;
  MatchExpressionNode *match_expr_ = nullptr;
};

class ExpressionNode : public ASTNode {
public:

private:
  ExpressionWithoutBlockNode *expr_wo_block_ = nullptr;
  ExpressionWithBlockNode *expr_w_block_ = nullptr;
};
#endif //EXPRESSION_H
