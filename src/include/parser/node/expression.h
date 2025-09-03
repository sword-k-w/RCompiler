#pragma once

#include "parser/class_declaration.h"
#include "lexer/lexer.h"
#include "parser/node/AST_node.h"
#include <string>
#include <cstdint>
#include <map>
#include <memory>

class LiteralExpressionNode : public ASTNode {
  friend class Printer;
  friend class FirstChecker;
  friend class ThirdChecker;
public:
  LiteralExpressionNode() = delete;
  LiteralExpressionNode(const std::vector<Token> &, uint32_t &, const uint32_t &);
  void Accept(VisitorBase *) override;
private:
  std::shared_ptr<CharLiteralNode> char_literal_;
  std::shared_ptr<StringLiteralNode> string_literal_;
  std::shared_ptr<RawStringLiteralNode> raw_string_literal_;
  std::shared_ptr<CStringLiteralNode> c_string_literal_;
  std::shared_ptr<RawCStringLiteralNode> raw_c_string_literal_;
  std::shared_ptr<IntegerLiteralNode> integer_literal_;
  std::shared_ptr<TrueNode> true_;
  std::shared_ptr<FalseNode> false_;
};

class ArrayElementsNode : public ASTNode {
  friend class Printer;
  friend class FirstChecker;
  friend class ThirdChecker;
public:
  ArrayElementsNode() = delete;
  ArrayElementsNode(const std::vector<Token> &, uint32_t &, const uint32_t &);
  void Accept(VisitorBase *) override;
private:
  std::vector<std::shared_ptr<ExpressionNode>> exprs_;
  uint32_t comma_cnt_ = 0;
  bool semicolon_ = false;
};

class ArrayExpressionNode : public ASTNode {
  friend class Printer;
  friend class FirstChecker;
  friend class ThirdChecker;
public:
  ArrayExpressionNode() = delete;
  ArrayExpressionNode(const std::vector<Token> &, uint32_t &, const uint32_t &);
  void Accept(VisitorBase *) override;
private:
  std::shared_ptr<ArrayElementsNode> array_elements_;
};

class PathInExpressionNode : public ASTNode {
  friend class Printer;
  friend class FirstChecker;
  friend class ThirdChecker;
public:
  PathInExpressionNode() = delete;
  PathInExpressionNode(const std::vector<Token> &, uint32_t &, const uint32_t &);
  void Accept(VisitorBase *) override;
private:
  std::shared_ptr<PathExprSegmentNode> path_expr_segment1_;
  std::shared_ptr<PathExprSegmentNode> path_expr_segment2_;
};

class StructExprFieldNode : public ASTNode {
  friend class Printer;
  friend class FirstChecker;
  friend class ThirdChecker;
public:
  StructExprFieldNode() = delete;
  StructExprFieldNode(const std::vector<Token> &, uint32_t &, const uint32_t &);
  void Accept(VisitorBase *) override;
private:
  std::shared_ptr<IdentifierNode> identifier_;
  std::shared_ptr<ExpressionNode> expr_;
};

class StructExprFieldsNode : public ASTNode {
  friend class Printer;
  friend class FirstChecker;
  friend class ThirdChecker;
public:
  StructExprFieldsNode() = delete;
  StructExprFieldsNode(const std::vector<Token> &, uint32_t &, const uint32_t &);
  void Accept(VisitorBase *) override;
private:
  std::vector<std::shared_ptr<StructExprFieldNode>> struct_expr_field_s_;
  uint32_t comma_cnt_ = 0;
};

class StructExpressionNode : public ASTNode {
  friend class Printer;
  friend class FirstChecker;
  friend class ThirdChecker;
public:
  StructExpressionNode() = delete;
  StructExpressionNode(const std::vector<Token> &, uint32_t &, const uint32_t &);
  void Accept(VisitorBase *) override;
private:
  std::shared_ptr<PathInExpressionNode> path_in_expr_;
  std::shared_ptr<StructExprFieldsNode> struct_expr_fields_;
};

class ExpressionWithoutBlockNode : public ASTNode {
  friend class Printer;
  friend class FirstChecker;
  friend class ThirdChecker;
public:
  ExpressionWithoutBlockNode() = delete;
  ExpressionWithoutBlockNode(const std::vector<Token> &, uint32_t &, const uint32_t &);
  void Accept(VisitorBase *) override;
private:
  std::shared_ptr<ExpressionNode> expr_;
};

class BlockExpressionNode : public ASTNode {
  friend class Printer;
  friend class FirstChecker;
  friend class ThirdChecker;
public:
  BlockExpressionNode() = delete;
  BlockExpressionNode(const std::vector<Token> &, uint32_t &, const uint32_t &);
  void Accept(VisitorBase *) override;
private:
  std::shared_ptr<StatementsNode> statements_;
};

class InfiniteLoopExpressionNode : public ASTNode {
  friend class Printer;
  friend class FirstChecker;
  friend class ThirdChecker;
public:
  InfiniteLoopExpressionNode() = delete;
  InfiniteLoopExpressionNode(const std::vector<Token> &, uint32_t &, const uint32_t &);
  void Accept(VisitorBase *) override;
private:
  std::shared_ptr<BlockExpressionNode> block_expr_;
};

class ConditionsNode : public ASTNode {
  friend class Printer;
  friend class FirstChecker;
  friend class ThirdChecker;
public:
  ConditionsNode() = delete;
  ConditionsNode(const std::vector<Token> &, uint32_t &, const uint32_t &);
  void Accept(VisitorBase *) override;
private:
  std::shared_ptr<ExpressionNode> expr_;
};

class PredicateLoopExpressionNode : public ASTNode {
  friend class Printer;
  friend class FirstChecker;
  friend class ThirdChecker;
public:
  PredicateLoopExpressionNode() = delete;
  PredicateLoopExpressionNode(const std::vector<Token> &, uint32_t &, const uint32_t &);
  void Accept(VisitorBase *) override;
private:
  std::shared_ptr<ConditionsNode> conditions_;
  std::shared_ptr<BlockExpressionNode> block_expr_;
};

class LoopExpressionNode : public ASTNode {
  friend class Printer;
  friend class FirstChecker;
  friend class ThirdChecker;
public:
  LoopExpressionNode() = delete;
  LoopExpressionNode(const std::vector<Token> &, uint32_t &, const uint32_t &);
  void Accept(VisitorBase *) override;
private:
  std::shared_ptr<InfiniteLoopExpressionNode> infinite_loop_expr_;
  std::shared_ptr<PredicateLoopExpressionNode> predicate_loop_expr_;
};

class IfExpressionNode : public ASTNode {
  friend class Printer;
  friend class FirstChecker;
  friend class ThirdChecker;
public:
  IfExpressionNode() = delete;
  IfExpressionNode(const std::vector<Token> &, uint32_t &, const uint32_t &);
  void Accept(VisitorBase *) override;
private:
  std::shared_ptr<ConditionsNode> conditions_;
  std::shared_ptr<BlockExpressionNode> block_expr1_;
  std::shared_ptr<BlockExpressionNode> block_expr2_;
  std::shared_ptr<IfExpressionNode> if_expr_;
};

class ExpressionWithBlockNode : public ASTNode {
  friend class Printer;
  friend class FirstChecker;
  friend class ThirdChecker;
public:
  ExpressionWithBlockNode() = delete;
  ExpressionWithBlockNode(const std::vector<Token> &, uint32_t &, const uint32_t &);
  void Accept(VisitorBase *) override;
private:
  std::shared_ptr<BlockExpressionNode> block_expr_;
  std::shared_ptr<LoopExpressionNode> loop_expr_;
  std::shared_ptr<IfExpressionNode> if_expr_;
};

class CallParamsNode : public ASTNode {
  friend class Printer;
  friend class FirstChecker;
  friend class ThirdChecker;
public:
  CallParamsNode() = delete;
  CallParamsNode(const std::vector<Token> &, uint32_t &, const uint32_t &);
  void Accept(VisitorBase *) override;
private:
  std::vector<std::shared_ptr<ExpressionNode>> exprs_;
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
  friend class FirstChecker;
  friend class ThirdChecker;
public:
  ExpressionNode() = delete;
  ExpressionNode(const ExpressionNode &) = default;
  ExpressionNode(const std::vector<Token> &, uint32_t &, const uint32_t &, const uint32_t & = 0);
  ExpressionType Type() const;
  void Accept(VisitorBase *) override;
private:
  ExpressionType type_;
  std::string op_;
  bool mut_ = false;
  std::shared_ptr<ExpressionNode> expr1_;
  std::shared_ptr<ExpressionNode> expr2_;
  std::shared_ptr<TypeNoBoundsNode> type_no_bounds_;
  std::shared_ptr<CallParamsNode> call_params_;
  std::shared_ptr<PathExprSegmentNode> path_expr_segment_;
  std::shared_ptr<IdentifierNode> identifier_;
  std::shared_ptr<LiteralExpressionNode> literal_expr_;
  std::shared_ptr<PathExpressionNode> path_expr_;
  std::shared_ptr<ArrayExpressionNode> array_expr_;
  std::shared_ptr<StructExpressionNode> struct_expr_;
  std::shared_ptr<ContinueExpressionNode> continue_expr_;
  std::shared_ptr<UnderscoreExpressionNode> underscore_expr_;
  std::shared_ptr<ExpressionWithBlockNode> expr_with_block_;
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
