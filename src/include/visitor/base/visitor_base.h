#pragma once

#include "parser/class_declaration.h"
#include <memory>

class VisitorBase {
public:
  virtual void Visit(CrateNode *) = 0;
  virtual void Visit(EnumVariantsNode *) = 0;
  virtual void Visit(EnumerationNode *) = 0;
  virtual void Visit(LiteralExpressionNode *) = 0;
  virtual void Visit(ArrayElementsNode *) = 0;
  virtual void Visit(ArrayExpressionNode *) = 0;
  virtual void Visit(PathInExpressionNode *) = 0;
  virtual void Visit(StructExprFieldNode *) = 0;
  virtual void Visit(StructExprFieldsNode *) = 0;
  virtual void Visit(StructExpressionNode *) = 0;
  virtual void Visit(ExpressionWithoutBlockNode *) = 0;
  virtual void Visit(BlockExpressionNode *) = 0;
  virtual void Visit(InfiniteLoopExpressionNode *) = 0;
  virtual void Visit(ConditionsNode *) = 0;
  virtual void Visit(PredicateLoopExpressionNode *) = 0;
  virtual void Visit(LoopExpressionNode *) = 0;
  virtual void Visit(IfExpressionNode *) = 0;
  virtual void Visit(ExpressionWithBlockNode *) = 0;
  virtual void Visit(CallParamsNode *) = 0;
  virtual void Visit(ExpressionNode *) = 0;
  virtual void Visit(ShorthandSelfNode *) = 0;
  virtual void Visit(TypedSelfNode *) = 0;
  virtual void Visit(SelfParamNode *) = 0;
  virtual void Visit(FunctionParamNode *) = 0;
  virtual void Visit(FunctionParametersNode *) = 0;
  virtual void Visit(FunctionReturnTypeNode *) = 0;
  virtual void Visit(FunctionNode *) = 0;
  virtual void Visit(ImplementationNode *) = 0;
  virtual void Visit(ConstantItemNode *) = 0;
  virtual void Visit(AssociatedItemNode *) = 0;
  virtual void Visit(ItemNode *) = 0;
  virtual void Visit(PathIdentSegmentNode *) = 0;
  virtual void Visit(LiteralPatternNode *) = 0;
  virtual void Visit(IdentifierPatternNode *) = 0;
  virtual void Visit(ReferencePatternNode *) = 0;
  virtual void Visit(PatternWithoutRangeNode *) = 0;
  virtual void Visit(LetStatementNode *) = 0;
  virtual void Visit(ExpressionStatementNode *) = 0;
  virtual void Visit(StatementNode *) = 0;
  virtual void Visit(StatementsNode *) = 0;
  virtual void Visit(StructFieldNode *) = 0;
  virtual void Visit(StructFieldsNode *) = 0;
  virtual void Visit(StructNode *) = 0;
  virtual void Visit(IdentifierNode *) = 0;
  virtual void Visit(CharLiteralNode *) = 0;
  virtual void Visit(StringLiteralNode *) = 0;
  virtual void Visit(RawStringLiteralNode *) = 0;
  virtual void Visit(CStringLiteralNode *) = 0;
  virtual void Visit(RawCStringLiteralNode *) = 0;
  virtual void Visit(IntegerLiteralNode *) = 0;
  virtual void Visit(TrueNode *) = 0;
  virtual void Visit(FalseNode *) = 0;
  virtual void Visit(SelfLowerNode *) = 0;
  virtual void Visit(SelfUpperNode *) = 0;
  virtual void Visit(UnderscoreExpressionNode *) = 0;
  virtual void Visit(ContinueExpressionNode *) = 0;
  virtual void Visit(TraitNode *) = 0;
  virtual void Visit(ReferenceTypeNode *) = 0;
  virtual void Visit(ArrayTypeNode *) = 0;
  virtual void Visit(UnitTypeNode *) = 0;
  virtual void Visit(TypeNoBoundsNode *) = 0;
};
