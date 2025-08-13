#ifndef EXPRESSION_H
#define EXPRESSION_H

#include <string>
#include "parser/node/AST_node.h"
#include "parser/node/terminal.h"

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

class PathExpressionNode : public ASTNode {

private:
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

class ArrayElementsNode {

};

class ArrayExpressionNode : public ASTNode {

private:
  ArrayElementsNode *array_elements_ = nullptr;
};

class IndexExpressionNode : public ASTNode {

private:
};

class TupleElements : public ASTNode {

private:
};

class TupleExpressionNode : public ASTNode {

private:
  TupleElements *tuple_elements_ = nullptr;
};

class TupleIndexingExpressionNode : public ASTNode {

private:
};

class PathInExpressionNode : public ASTNode {

};

class StructExprFieldsNode : public ASTNode {

};

class StructBaseNode : public ASTNode {

};

class StructExpressionNode : public ASTNode {

private:
  PathInExpressionNode *path_in_expr_ = nullptr;
  StructExprFieldsNode *struct_expr_fields_ = nullptr;
  StructBaseNode *struct_base_ = nullptr;
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

class ContinueExpressionNode : public ASTNode {

private:
  const std::string content_ = "continue";
};

class BreakExpressionNode : public ASTNode {

private:
  const std::string content_ = "break";
  ExpressionNode *expr_ = nullptr;
};

class RangeExprNode : public ASTNode {

};

class RangeFromExprNode : public ASTNode {

};

class RangeToExprNode : public ASTNode {

};

class RangeFullExprNode : public ASTNode {

};

class RangeInclusiveExprNode : public ASTNode {

};

class RangeToInclusiveExprNode : public ASTNode {

};

class RangeExpressionNode : public ASTNode {

private:
  RangeExprNode *range_expr_ = nullptr;
  RangeFromExprNode *range_from_expr_ = nullptr;
  RangeFullExprNode *range_full_expr_ = nullptr;
  RangeInclusiveExprNode *range_inclusive_expr_ = nullptr;
  RangeToInclusiveExprNode *range_to_inclusive_expr_ = nullptr;
};

class ReturnExpressionNode : public ASTNode {

private:
};

class UnderscoreExpressionNode : public ASTNode {

private:
};

class ExpressionWithoutBlockNode : public ASTNode {
private:
  LiteralExpressionNode *literal_expr_ = nullptr;
  PathExpressionNode *path_expr_ = nullptr;
  OperatorExpressionNode *operator_expr_ = nullptr;
  GroupedExpressionNode *grouped_expr_ = nullptr;
  ArrayExpressionNode *array_expr_ = nullptr;
  IndexExpressionNode *index_expr_ = nullptr;
  TupleExpressionNode *tuple_expr_ = nullptr;
  TupleIndexingExpressionNode *tuple_indexing_expr_ = nullptr;
  StructExpressionNode *struct_expr_ = nullptr;
  CallExpressionNode *call_expr_ = nullptr;
  MethodCallExpressionNode *method_call_expr_ = nullptr;
  FieldExpressionNode *field_expr_ = nullptr;
  ContinueExpressionNode *continue_expr_ = nullptr;
  BreakExpressionNode *break_expr_ = nullptr;
  RangeExpressionNode *range_expr_ = nullptr;
  ReturnExpressionNode *return_expr_ = nullptr;
  UnderscoreExpressionNode *underscore_expr_ = nullptr;
};

class BlockExpressionNode : public ASTNode {
public:
  BlockExpressionNode() = delete;
  BlockExpressionNode(const std::vector<Token> &, uint32_t &, const uint32_t &);
private:
};

class ConstBlockExpressionNode : public ASTNode {

private:
};

class LoopExpressionNode : public ASTNode {

private:
};

class IfExpressionNode : public ASTNode {

private:
};

class MatchExpressionNode : public ASTNode {

private:
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
