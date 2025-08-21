#ifndef PRINTER_H
#define PRINTER_H

#include "visitor/base/visitor_base.h"
#include <stack>

class Printer : public VisitorBase {
public:
  Printer() = delete;
  explicit Printer(std::ostream &);
  void Prepare();
  void Visit(CrateNode *node) override;
  void Visit(EnumVariantsNode *node) override;
  void Visit(EnumerationNode *node) override;
  void Visit(LiteralExpressionNode *node) override;
  void Visit(ArrayElementsNode *node) override;
  void Visit(ArrayExpressionNode *node) override;
  void Visit(PathInExpressionNode *node) override;
  void Visit(StructExprFieldNode *node) override;
  void Visit(StructExprFieldsNode *node) override;
  void Visit(StructExpressionNode *node) override;
  void Visit(ExpressionWithoutBlockNode *node) override;
  void Visit(BlockExpressionNode *node) override;
  void Visit(ConstBlockExpressionNode *node) override;
  void Visit(InfiniteLoopExpressionNode *node) override;
  void Visit(LetChainConditionNode *node) override;
  void Visit(LetChainNode *node) override;
  void Visit(ConditionsNode *node) override;
  void Visit(PredicateLoopExpressionNode *node) override;
  void Visit(LoopExpressionNode *node) override;
  void Visit(IfExpressionNode *node) override;
  void Visit(ScrutineeNode *node) override;
  void Visit(MatchArmGuardNode *node) override;
  void Visit(MatchArmNode *node) override;
  void Visit(MatchArmsNode *node) override;
  void Visit(MatchExpressionNode *node) override;
  void Visit(ExpressionWithBlockNode *node) override;
  void Visit(CallParamsNode *node) override;
  void Visit(ExpressionNode *node) override;
  void Visit(ShorthandSelfNode *node) override;
  void Visit(TypedSelfNode *node) override;
  void Visit(SelfParamNode *node) override;
  void Visit(FunctionParamNode *node) override;
  void Visit(FunctionParametersNode *node) override;
  void Visit(FunctionReturnTypeNode *node) override;
  void Visit(FunctionNode *node) override;
  void Visit(ImplementationNode *node) override;
  void Visit(ConstantItemNode *node) override;
  void Visit(AssociatedItemNode *node) override;
  void Visit(ItemNode *node) override;
  void Visit(PathIdentSegmentNode *node) override;
  void Visit(LiteralPatternNode *node) override;
  void Visit(IdentifierPatternNode *node) override;
  void Visit(ReferencePatternNode *node) override;
  void Visit(TupleStructItemsNode *node) override;
  void Visit(TupleStructPatternNode *node) override;
  void Visit(PatternWithoutRangeNode *node) override;
  void Visit(LetStatementNode *node) override;
  void Visit(ExpressionStatementNode *node) override;
  void Visit(StatementNode *node) override;
  void Visit(StatementsNode *node) override;
  void Visit(StructFieldNode *node) override;
  void Visit(StructFieldsNode *node) override;
  void Visit(StructNode *node) override;
  void Visit(IdentifierNode *node) override;
  void Visit(CharLiteralNode *node) override;
  void Visit(StringLiteralNode *node) override;
  void Visit(RawStringLiteralNode *node) override;
  void Visit(CStringLiteralNode *node) override;
  void Visit(RawCStringLiteralNode *node) override;
  void Visit(IntegerLiteralNode *node) override;
  void Visit(TrueNode *node) override;
  void Visit(FalseNode *node) override;
  void Visit(SuperNode *node) override;
  void Visit(SelfLowerNode *node) override;
  void Visit(SelfUpperNode *node) override;
  void Visit(UnderscoreExpressionNode *node) override;
  void Visit(ContinueExpressionNode *node) override;
  void Visit(TraitNode *node) override;
  void Visit(ReferenceTypeNode *node) override;
  void Visit(ArrayTypeNode *node) override;
  void Visit(UnitTypeNode *node) override;
  void Visit(TypeNoBoundsNode *node) override;
private:
  std::ostream &os_;
  std::stack<std::string> prefixes_;
  std::stack<bool> is_lasts_;
};

#endif //PRINTER_H
