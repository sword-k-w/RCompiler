#ifndef EXPRESSION_H
#define EXPRESSION_H

#include "parser/AST_node.h"
#include "parser/terminal.h"

class LiteralExpressionNode : public ASTNode {

private:

};

class PathExpressionNode : public ASTNode {

private:
};

class OperatorExpressionNode : public ASTNode {

private:
};

class GroupedExpressionNode : public ASTNode {

private:
  ExpressionNode *expr_;
};

class ArrayExpressionNode : public ASTNode {

private:
};

class IndexExpressionNode : public ASTNode {

private:
};

class TupleExpressionNode : public ASTNode {

private:
};

class TupleIndexingExpressionNode : public ASTNode {

private:
};

class StructExpressionNode : public ASTNode {

private:
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
};

class BreakExpressionNode : public ASTNode {

private:
};

class RangeExpressionNode : public ASTNode {

private:
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
