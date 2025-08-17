#ifndef EXPRESSION_H
#define EXPRESSION_H

#include <string>
#include "parser/node/AST_node.h"
#include "parser/node/terminal.h"
#include "parser/node/statement.h"
#include <map>
#include <set>

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

class ExpressionWithoutBlockNode : public ASTNode {
public:
  ExpressionWithoutBlockNode() = delete;
  ExpressionWithoutBlockNode(const std::vector<Token> &, uint32_t &, const uint32_t &);
private:
  ExpressionNode *expr_ = nullptr;
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

class MatchArmGuardNode : public ASTNode {
public:
  MatchArmGuardNode() = delete;
  MatchArmGuardNode(const std::vector<Token> &, uint32_t &, const uint32_t &);
private:
  ExpressionNode *expr_;
};

class MatchArmNode : public ASTNode {
public:
  MatchArmNode() = delete;
  MatchArmNode(const std::vector<Token> &, uint32_t &, const uint32_t &);
private:
  PatternNode *pattern_ = nullptr;
  MatchArmGuardNode *match_arm_guard_ = nullptr;
};

class MatchArmsNode : public ASTNode {
public:
  MatchArmsNode() = delete;
  MatchArmsNode(const std::vector<Token> &, uint32_t &, const uint32_t &);
private:
  std::vector<MatchArmNode *> match_arm_s_;
  std::vector<ExpressionNode *> expr_;
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
public:
  ExpressionWithBlockNode() = delete;
  ExpressionWithBlockNode(const std::vector<Token> &, uint32_t &, const uint32_t &);
private:
  BlockExpressionNode *block_expr_ = nullptr;
  ConstBlockExpressionNode *const_block_expr_ = nullptr;
  LoopExpressionNode *loop_expr_ = nullptr;
  IfExpressionNode *if_expr_ = nullptr;
  MatchExpressionNode *match_expr_ = nullptr;
};

class CallParamsNode : public ASTNode {
public:
  CallParamsNode() = delete;
  CallParamsNode(const std::vector<Token> &, uint32_t &, const uint32_t &);
private:
  std::vector<ExpressionNode *> exprs_;
  uint32_t comma_cnt_ = 0;
};

enum ExpressionType {
  kLiteralExpr, kPathExpr, kArrayExpr, kStructExpr, kContinueExpr, kUnderscoreExpr, kBorrowExpr,
  kDereferenceExpr, kNegationExpr, kArithmeticOrLogicExpr, kComparisonExpr, kLazyBooleanExpr,
  kTypeCastExpr, kAssignmentExpr, kCompoundAssignmentExpr, kGroupedExpr, kIndexExpr, kCallExpr,
  kMethodCallExpr, kFieldExpr, kBreakExpr, kReturnExpr, kBlockExpr
};

class ExpressionNode : public ASTNode {
public:
  ExpressionNode() = delete;
  ExpressionNode(const ExpressionNode &) = default;
  ExpressionNode(const std::vector<Token> &, uint32_t &, const uint32_t &, const uint32_t & = 0);
  ExpressionType Type() const;
private:
  ExpressionType type_;
  std::string op_;
  bool mut_ = false;
  ExpressionNode *expr1_ = nullptr;
  ExpressionNode *expr2_ = nullptr;
  TypeNoBoundsNode *type_no_bounds_ = nullptr;
  CallParamsNode *call_params_ = nullptr;
  PathExprSegmentNode *path_expr_segment_ = nullptr;
  IdentifierNode *identifier_ = nullptr;
  LiteralExpressionNode *literal_expr_ = nullptr;
  PathExpressionNode *path_expr_ = nullptr;
  ArrayExpressionNode *array_expr_ = nullptr;
  StructExpressionNode *struct_expr_ = nullptr;
  ContinueExpressionNode *continue_expr_ = nullptr;
  UnderscoreExpressionNode *underscore_expr_ = nullptr;
  ExpressionWithBlockNode *expr_with_block_ = nullptr;
};

// true means binary, false means unary
std::map<std::pair<std::string, bool>, std::pair<uint32_t, uint32_t>> binding_power = {
  {{".", true}, {500000, 500001}},
  {{"(", true}, {200000, 200001}},
  {{"[", true}, {200000, 200001}},
  {{"-", false}, {0, 100000}},
  {{"!", false}, {0, 100000}},
  {{"*", false}, {0, 100000}},
  {{"borrow", false}, {0, 100000}},
  {{"as", true}, {800000, 800001}},
  {{"*", true}, {500000, 500001}},
  {{"/", true}, {500000, 500001}},
  {{"%", true}, {500000, 500001}},
  {{"+", true}, {200000, 200001}},
  {{"-", true}, {200000, 200001}},
  {{"<<", true}, {100000, 100001}},
  {{">>", true}, {100000, 100001}},
  {{"&", true}, {50000, 50001}},
  {{"^", true}, {20000, 20001}},
  {{"|", true}, {10000, 10001}},
  {{"==", true}, {5000, 5000}},
  {{"!=", true}, {5000, 5000}},
  {{"<", true}, {5000, 5000}},
  {{">", true}, {5000, 5000}},
  {{"<=", true}, {5000, 5000}},
  {{">=", true}, {5000, 5000}},
  {{"&&", true}, {2000, 2001}},
  {{"||", true}, {1000, 1001}},
  {{"=", true}, {501, 500}},
  {{"+=", true}, {501, 500}},
  {{"-=", true}, {501, 500}},
  {{"*=", true}, {501, 500}},
  {{"/=", true}, {501, 500}},
  {{"%=", true}, {501, 500}},
  {{"&=", true}, {501, 500}},
  {{"|=", true}, {501, 500}},
  {{"^=", true}, {501, 500}},
  {{"<<=", true}, {501, 500}},
  {{">>=", true}, {501, 500}},
  {{"return", false}, {0, 200}},
  {{"break", false}, {0, 200}}
};

std::map<std::string, ExpressionType> infix_type = {
  {".", kFieldExpr}, // uncertain
  {"(", kCallExpr},
  {"[", kIndexExpr},
  {"as", kTypeCastExpr},
  {"*", kArithmeticOrLogicExpr},
  {"/", kArithmeticOrLogicExpr},
  {"%", kArithmeticOrLogicExpr},
  {"+", kArithmeticOrLogicExpr},
  {"-", kArithmeticOrLogicExpr},
  {"<<", kArithmeticOrLogicExpr},
  {">>", kArithmeticOrLogicExpr},
  {"&", kArithmeticOrLogicExpr},
  {"|", kArithmeticOrLogicExpr},
  {"^", kArithmeticOrLogicExpr},
  {"==", kComparisonExpr},
  {"!=", kComparisonExpr},
  {">", kComparisonExpr},
  {"<", kComparisonExpr},
  {">=", kComparisonExpr},
  {"<=", kComparisonExpr},
  {"||", kLazyBooleanExpr},
  {"&&", kLazyBooleanExpr},
  {"=", kAssignmentExpr},
  {"+=", kCompoundAssignmentExpr},
  {"-=", kCompoundAssignmentExpr},
  {"*=", kCompoundAssignmentExpr},
  {"/=", kCompoundAssignmentExpr},
  {"%=", kCompoundAssignmentExpr},
  {"|=", kCompoundAssignmentExpr},
  {"^=", kCompoundAssignmentExpr},
  {"&=", kCompoundAssignmentExpr},
  {"<<=", kCompoundAssignmentExpr},
  {">>=", kCompoundAssignmentExpr},
  {"return", kReturnExpr},
  {"break", kBreakExpr},
};



#endif //EXPRESSION_H
