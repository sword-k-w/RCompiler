#ifndef PRINTER_H
#define PRINTER_H

#include "visitor/base/visitor_base.h"
#include <stack>
#include <iostream>

class Printer : public VisitorBase {
public:
  Printer() = delete;
  explicit Printer(std::ostream &);
  void Prepare();
  void Visit(std::shared_ptr<CrateNode> node) override;
  void Visit(std::shared_ptr<EnumVariantsNode> node) override;
  void Visit(std::shared_ptr<EnumerationNode> node) override;
  void Visit(std::shared_ptr<LiteralExpressionNode> node) override;
  void Visit(std::shared_ptr<ArrayElementsNode> node) override;
  void Visit(std::shared_ptr<ArrayExpressionNode> node) override;
  void Visit(std::shared_ptr<PathInExpressionNode> node) override;
  void Visit(std::shared_ptr<StructExprFieldNode> node) override;
  void Visit(std::shared_ptr<StructExprFieldsNode> node) override;
  void Visit(std::shared_ptr<StructExpressionNode> node) override;
  void Visit(std::shared_ptr<ExpressionWithoutBlockNode> node) override;
  void Visit(std::shared_ptr<BlockExpressionNode> node) override;
  void Visit(std::shared_ptr<ConstBlockExpressionNode> node) override;
  void Visit(std::shared_ptr<InfiniteLoopExpressionNode> node) override;
  void Visit(std::shared_ptr<ConditionsNode> node) override;
  void Visit(std::shared_ptr<PredicateLoopExpressionNode> node) override;
  void Visit(std::shared_ptr<LoopExpressionNode> node) override;
  void Visit(std::shared_ptr<IfExpressionNode> node) override;
  void Visit(std::shared_ptr<ExpressionWithBlockNode> node) override;
  void Visit(std::shared_ptr<CallParamsNode> node) override;
  void Visit(std::shared_ptr<ExpressionNode> node) override;
  void Visit(std::shared_ptr<ShorthandSelfNode> node) override;
  void Visit(std::shared_ptr<TypedSelfNode> node) override;
  void Visit(std::shared_ptr<SelfParamNode> node) override;
  void Visit(std::shared_ptr<FunctionParamNode> node) override;
  void Visit(std::shared_ptr<FunctionParametersNode> node) override;
  void Visit(std::shared_ptr<FunctionReturnTypeNode> node) override;
  void Visit(std::shared_ptr<FunctionNode> node) override;
  void Visit(std::shared_ptr<ImplementationNode> node) override;
  void Visit(std::shared_ptr<ConstantItemNode> node) override;
  void Visit(std::shared_ptr<AssociatedItemNode> node) override;
  void Visit(std::shared_ptr<ItemNode> node) override;
  void Visit(std::shared_ptr<PathIdentSegmentNode> node) override;
  void Visit(std::shared_ptr<LiteralPatternNode> node) override;
  void Visit(std::shared_ptr<IdentifierPatternNode> node) override;
  void Visit(std::shared_ptr<ReferencePatternNode> node) override;
  void Visit(std::shared_ptr<TupleStructItemsNode> node) override;
  void Visit(std::shared_ptr<TupleStructPatternNode> node) override;
  void Visit(std::shared_ptr<PatternWithoutRangeNode> node) override;
  void Visit(std::shared_ptr<LetStatementNode> node) override;
  void Visit(std::shared_ptr<ExpressionStatementNode> node) override;
  void Visit(std::shared_ptr<StatementNode> node) override;
  void Visit(std::shared_ptr<StatementsNode> node) override;
  void Visit(std::shared_ptr<StructFieldNode> node) override;
  void Visit(std::shared_ptr<StructFieldsNode> node) override;
  void Visit(std::shared_ptr<StructNode> node) override;
  void Visit(std::shared_ptr<IdentifierNode> node) override;
  void Visit(std::shared_ptr<CharLiteralNode> node) override;
  void Visit(std::shared_ptr<StringLiteralNode> node) override;
  void Visit(std::shared_ptr<RawStringLiteralNode> node) override;
  void Visit(std::shared_ptr<CStringLiteralNode> node) override;
  void Visit(std::shared_ptr<RawCStringLiteralNode> node) override;
  void Visit(std::shared_ptr<IntegerLiteralNode> node) override;
  void Visit(std::shared_ptr<TrueNode> node) override;
  void Visit(std::shared_ptr<FalseNode> node) override;
  void Visit(std::shared_ptr<SuperNode> node) override;
  void Visit(std::shared_ptr<SelfLowerNode> node) override;
  void Visit(std::shared_ptr<SelfUpperNode> node) override;
  void Visit(std::shared_ptr<UnderscoreExpressionNode> node) override;
  void Visit(std::shared_ptr<ContinueExpressionNode> node) override;
  void Visit(std::shared_ptr<TraitNode> node) override;
  void Visit(std::shared_ptr<ReferenceTypeNode> node) override;
  void Visit(std::shared_ptr<ArrayTypeNode> node) override;
  void Visit(std::shared_ptr<UnitTypeNode> node) override;
  void Visit(std::shared_ptr<TypeNoBoundsNode> node) override;
private:
  std::ostream &os_;
  std::stack<std::string> prefixes_;
  std::stack<bool> is_lasts_;
};

#endif //PRINTER_H
