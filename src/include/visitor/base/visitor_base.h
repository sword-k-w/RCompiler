#ifndef VISITOR_BASE_H
#define VISITOR_BASE_H

#include "parser/node/crate.h"

class VisitorBase {
public:
  virtual void Visit(CrateNode *node) = 0;
  virtual void Visit(EnumVariantsNode *node) = 0;
  virtual void Visit(EnumerationNode *node) = 0;
  virtual void Visit(LiteralExpressionNode *node) = 0;
  virtual void Visit(ArrayElementsNode *node) = 0;
  virtual void Visit(ArrayExpressionNode *node) = 0;
  virtual void Visit(PathInExpressionNode *node) = 0;
  virtual void Visit(StructExprFieldNode *node) = 0;
  virtual void Visit(StructExprFieldsNode *node) = 0;
  virtual void Visit(StructExpressionNode *node) = 0;
  virtual void Visit(ExpressionWithoutBlockNode *node) = 0;
  virtual void Visit(BlockExpressionNode *node) = 0;
  virtual void Visit(ConstBlockExpressionNode *node) = 0;
  virtual void Visit(InfiniteLoopExpressionNode *node) = 0;
  virtual void Visit(ConditionsNode *node) = 0;
  virtual void Visit(PredicateLoopExpressionNode *node) = 0;
  virtual void Visit(LoopExpressionNode *node) = 0;
  virtual void Visit(IfExpressionNode *node) = 0;
  virtual void Visit(ExpressionWithBlockNode *node) = 0;
  virtual void Visit(CallParamsNode *node) = 0;
  virtual void Visit(ExpressionNode *node) = 0;
  virtual void Visit(ShorthandSelfNode *node) = 0;
  virtual void Visit(TypedSelfNode *node) = 0;
  virtual void Visit(SelfParamNode *node) = 0;
  virtual void Visit(FunctionParamNode *node) = 0;
  virtual void Visit(FunctionParametersNode *node) = 0;
  virtual void Visit(FunctionReturnTypeNode *node) = 0;
  virtual void Visit(FunctionNode *node) = 0;
  virtual void Visit(ImplementationNode *node) = 0;
  virtual void Visit(ConstantItemNode *node) = 0;
  virtual void Visit(AssociatedItemNode *node) = 0;
  virtual void Visit(ItemNode *node) = 0;
  virtual void Visit(PathIdentSegmentNode *node) = 0;
  virtual void Visit(LiteralPatternNode *node) = 0;
  virtual void Visit(IdentifierPatternNode *node) = 0;
  virtual void Visit(ReferencePatternNode *node) = 0;
  virtual void Visit(TupleStructItemsNode *node) = 0;
  virtual void Visit(TupleStructPatternNode *node) = 0;
  virtual void Visit(PatternWithoutRangeNode *node) = 0;
  virtual void Visit(LetStatementNode *node) = 0;
  virtual void Visit(ExpressionStatementNode *node) = 0;
  virtual void Visit(StatementNode *node) = 0;
  virtual void Visit(StatementsNode *node) = 0;
  virtual void Visit(StructFieldNode *node) = 0;
  virtual void Visit(StructFieldsNode *node) = 0;
  virtual void Visit(StructNode *node) = 0;
  virtual void Visit(IdentifierNode *node) = 0;
  virtual void Visit(CharLiteralNode *node) = 0;
  virtual void Visit(StringLiteralNode *node) = 0;
  virtual void Visit(RawStringLiteralNode *node) = 0;
  virtual void Visit(CStringLiteralNode *node) = 0;
  virtual void Visit(RawCStringLiteralNode *node) = 0;
  virtual void Visit(IntegerLiteralNode *node) = 0;
  virtual void Visit(TrueNode *node) = 0;
  virtual void Visit(FalseNode *node) = 0;
  virtual void Visit(SuperNode *node) = 0;
  virtual void Visit(SelfLowerNode *node) = 0;
  virtual void Visit(SelfUpperNode *node) = 0;
  virtual void Visit(UnderscoreExpressionNode *node) = 0;
  virtual void Visit(ContinueExpressionNode *node) = 0;
  virtual void Visit(TraitNode *node) = 0;
  virtual void Visit(ReferenceTypeNode *node) = 0;
  virtual void Visit(ArrayTypeNode *node) = 0;
  virtual void Visit(UnitTypeNode *node) = 0;
  virtual void Visit(TypeNoBoundsNode *node) = 0;
};

#endif //VISITOR_BASE_H
