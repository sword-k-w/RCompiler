#pragma once

#include "visitor/base/visitor_base.h"
#include "parser/node/AST_node.h"
#include <stack>

class SecondChecker : public VisitorBase {
public:
  SecondChecker() = default;
  void Visit(CrateNode *) override;
  void Visit(EnumVariantsNode *) override;
  void Visit(EnumerationNode *) override;
  void Visit(LiteralExpressionNode *) override;
  void Visit(ArrayElementsNode *) override;
  void Visit(ArrayExpressionNode *) override;
  void Visit(PathInExpressionNode *) override;
  void Visit(StructExprFieldNode *) override;
  void Visit(StructExprFieldsNode *) override;
  void Visit(StructExpressionNode *) override;
  void Visit(ExpressionWithoutBlockNode *) override;
  void Visit(BlockExpressionNode *) override;
  void Visit(InfiniteLoopExpressionNode *) override;
  void Visit(ConditionsNode *) override;
  void Visit(PredicateLoopExpressionNode *) override;
  void Visit(LoopExpressionNode *) override;
  void Visit(IfExpressionNode *) override;
  void Visit(ExpressionWithBlockNode *) override;
  void Visit(CallParamsNode *) override;
  void Visit(ExpressionNode *) override;
  void Visit(ShorthandSelfNode *) override;
  void Visit(SelfParamNode *) override;
  void Visit(FunctionParamNode *) override;
  void Visit(FunctionParametersNode *) override;
  void Visit(FunctionReturnTypeNode *) override;
  void Visit(FunctionNode *) override;
  void Visit(ImplementationNode *) override;
  void Visit(ConstantItemNode *) override;
  void Visit(AssociatedItemNode *) override;
  void Visit(ItemNode *) override;
  void Visit(PathIdentSegmentNode *) override;
  void Visit(IdentifierPatternNode *) override;
  void Visit(ReferencePatternNode *) override;
  void Visit(PatternWithoutRangeNode *) override;
  void Visit(LetStatementNode *) override;
  void Visit(ExpressionStatementNode *) override;
  void Visit(StatementNode *) override;
  void Visit(StatementsNode *) override;
  void Visit(StructFieldNode *) override;
  void Visit(StructFieldsNode *) override;
  void Visit(StructNode *) override;
  void Visit(IdentifierNode *) override;
  void Visit(CharLiteralNode *) override;
  void Visit(StringLiteralNode *) override;
  void Visit(RawStringLiteralNode *) override;
  void Visit(CStringLiteralNode *) override;
  void Visit(RawCStringLiteralNode *) override;
  void Visit(IntegerLiteralNode *) override;
  void Visit(TrueNode *) override;
  void Visit(FalseNode *) override;
  void Visit(SelfLowerNode *) override;
  void Visit(SelfUpperNode *) override;
  void Visit(UnderscoreExpressionNode *) override;
  void Visit(ContinueExpressionNode *) override;
  void Visit(TraitNode *) override;
  void Visit(ReferenceTypeNode *) override;
  void Visit(ArrayTypeNode *) override;
  void Visit(UnitTypeNode *) override;
  void Visit(TypeNoBoundsNode *) override;
private:
  void GoDown(ASTNode *, ASTNode *);
  std::stack<StructNode *> current_Self_;
  std::stack<LoopExpressionNode *> current_loop_;
  std::stack<FunctionNode *> current_function_;
};
