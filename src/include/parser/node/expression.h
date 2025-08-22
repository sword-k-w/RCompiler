#pragma once

#include "parser/class_declaration.h"
#include "lexer/lexer.h"
#include "parser/node/AST_node.h"
#include <string>
#include <cstdint>
#include <map>
#include <set>

class LiteralExpressionNode : public ASTNode {
  friend class Printer;
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
  TrueNode *true_ = nullptr;
  FalseNode *false_ = nullptr;
};

class ArrayElementsNode : public ASTNode {
  friend class Printer;
public:
  ArrayElementsNode() = delete;
  ArrayElementsNode(const std::vector<Token> &, uint32_t &, const uint32_t &);
private:
  std::vector<ExpressionNode *> exprs_;
  uint32_t comma_cnt_ = 0;
  bool semicolon_ = false;
};

class ArrayExpressionNode : public ASTNode {
  friend class Printer;
public:
  ArrayExpressionNode() = delete;
  ArrayExpressionNode(const std::vector<Token> &, uint32_t &, const uint32_t &);
private:
  ArrayElementsNode *array_elements_ = nullptr;
};

class PathInExpressionNode : public ASTNode {
  friend class Printer;
public:
  PathInExpressionNode() = delete;
  PathInExpressionNode(const std::vector<Token> &, uint32_t &, const uint32_t &);
private:
  std::vector<PathExprSegmentNode *> path_expr_segments_;
};

class StructExprFieldNode : public ASTNode {
  friend class Printer;
public:
  StructExprFieldNode() = delete;
  StructExprFieldNode(const std::vector<Token> &, uint32_t &, const uint32_t &);
private:
  IdentifierNode *identifier_ = nullptr;
  ExpressionNode *expr_ = nullptr;
};

class StructExprFieldsNode : public ASTNode {
  friend class Printer;
public:
  StructExprFieldsNode() = delete;
  StructExprFieldsNode(const std::vector<Token> &, uint32_t &, const uint32_t &);
private:
  std::vector<StructExprFieldNode *> struct_expr_field_s_;
  uint32_t comma_cnt_ = 0;
};

class StructExpressionNode : public ASTNode {
  friend class Printer;
public:
  StructExpressionNode() = delete;
  StructExpressionNode(const std::vector<Token> &, uint32_t &, const uint32_t &);
private:
  PathInExpressionNode *path_in_expr_ = nullptr;
  StructExprFieldsNode *struct_expr_fields_ = nullptr;
};

class ExpressionWithoutBlockNode : public ASTNode {
  friend class Printer;
public:
  ExpressionWithoutBlockNode() = delete;
  ExpressionWithoutBlockNode(const std::vector<Token> &, uint32_t &, const uint32_t &);
private:
  ExpressionNode *expr_ = nullptr;
};

class BlockExpressionNode : public ASTNode {
  friend class Printer;
public:
  BlockExpressionNode() = delete;
  BlockExpressionNode(const std::vector<Token> &, uint32_t &, const uint32_t &);
private:
  StatementsNode *statements_ = nullptr;
};

class ConstBlockExpressionNode : public ASTNode {
  friend class Printer;
public:
  ConstBlockExpressionNode() = delete;
  ConstBlockExpressionNode(const std::vector<Token> &, uint32_t &, const uint32_t &);
private:
  BlockExpressionNode *block_expr_ = nullptr;
};

class InfiniteLoopExpressionNode : public ASTNode {
  friend class Printer;
public:
  InfiniteLoopExpressionNode() = delete;
  InfiniteLoopExpressionNode(const std::vector<Token> &, uint32_t &, const uint32_t &);
private:
  BlockExpressionNode *block_expr_ = nullptr;
};

class ConditionsNode : public ASTNode {
  friend class Printer;
public:
  ConditionsNode() = delete;
  ConditionsNode(const std::vector<Token> &, uint32_t &, const uint32_t &);
private:
  ExpressionNode *expr_ = nullptr;
};

class PredicateLoopExpressionNode : public ASTNode {
  friend class Printer;
public:
  PredicateLoopExpressionNode() = delete;
  PredicateLoopExpressionNode(const std::vector<Token> &, uint32_t &, const uint32_t &);
private:
  ConditionsNode *conditions_ = nullptr;
  BlockExpressionNode *block_expr_ = nullptr;
};

class LoopExpressionNode : public ASTNode {
  friend class Printer;
public:
  LoopExpressionNode() = delete;
  LoopExpressionNode(const std::vector<Token> &, uint32_t &, const uint32_t &);
private:
  InfiniteLoopExpressionNode *infinite_loop_expr_ = nullptr;
  PredicateLoopExpressionNode *predicate_loop_expr_ = nullptr;
};

class IfExpressionNode : public ASTNode {
  friend class Printer;
public:
  IfExpressionNode() = delete;
  IfExpressionNode(const std::vector<Token> &, uint32_t &, const uint32_t &);
private:
  ConditionsNode *conditions_ = nullptr;
  BlockExpressionNode *block_expr1_ = nullptr;
  BlockExpressionNode *block_expr2_ = nullptr;
  IfExpressionNode *if_expr_ = nullptr;
};

class ExpressionWithBlockNode : public ASTNode {
  friend class Printer;
public:
  ExpressionWithBlockNode() = delete;
  ExpressionWithBlockNode(const std::vector<Token> &, uint32_t &, const uint32_t &);
private:
  BlockExpressionNode *block_expr_ = nullptr;
  ConstBlockExpressionNode *const_block_expr_ = nullptr;
  LoopExpressionNode *loop_expr_ = nullptr;
  IfExpressionNode *if_expr_ = nullptr;
};

class CallParamsNode : public ASTNode {
  friend class Printer;
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
  kMethodCallExpr, kFieldExpr, kBreakExpr, kReturnExpr, kExprWithBlock
};

void Print(ExpressionType, std::ostream &);

class ExpressionNode : public ASTNode {
  friend class Printer;
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
inline std::map<std::pair<std::string, bool>, std::pair<uint32_t, uint32_t>> binding_power = {
  {{".", true}, {5000000, 5000001}},
  {{"(", true}, {2000000, 2000001}},
  {{"[", true}, {2000000, 2000001}},
  {{"-", false}, {0, 1000000}},
  {{"!", false}, {0, 1000000}},
  {{"*", false}, {0, 1000000}},
  {{"&", false}, {0, 1000000}},
  {{"&&", false}, {0, 1000000}},
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
  {{"break", false}, {0, 200}},
  {{"(", false}, {0, 0}},
};

inline std::map<std::string, ExpressionType> infix_type = {
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
