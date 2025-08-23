#ifndef VISITOR_BASE_H
#define VISITOR_BASE_H


#include "parser/class_declaration.h"
#include <memory>

class VisitorBase {
public:
  virtual void Visit(std::shared_ptr<CrateNode> node) = 0;
  virtual void Visit(std::shared_ptr<EnumVariantsNode> node) = 0;
  virtual void Visit(std::shared_ptr<EnumerationNode> node) = 0;
  virtual void Visit(std::shared_ptr<LiteralExpressionNode> node) = 0;
  virtual void Visit(std::shared_ptr<ArrayElementsNode> node) = 0;
  virtual void Visit(std::shared_ptr<ArrayExpressionNode> node) = 0;
  virtual void Visit(std::shared_ptr<PathInExpressionNode> node) = 0;
  virtual void Visit(std::shared_ptr<StructExprFieldNode> node) = 0;
  virtual void Visit(std::shared_ptr<StructExprFieldsNode> node) = 0;
  virtual void Visit(std::shared_ptr<StructExpressionNode> node) = 0;
  virtual void Visit(std::shared_ptr<ExpressionWithoutBlockNode> node) = 0;
  virtual void Visit(std::shared_ptr<BlockExpressionNode> node) = 0;
  virtual void Visit(std::shared_ptr<ConstBlockExpressionNode> node) = 0;
  virtual void Visit(std::shared_ptr<InfiniteLoopExpressionNode> node) = 0;
  virtual void Visit(std::shared_ptr<ConditionsNode> node) = 0;
  virtual void Visit(std::shared_ptr<PredicateLoopExpressionNode> node) = 0;
  virtual void Visit(std::shared_ptr<LoopExpressionNode> node) = 0;
  virtual void Visit(std::shared_ptr<IfExpressionNode> node) = 0;
  virtual void Visit(std::shared_ptr<ExpressionWithBlockNode> node) = 0;
  virtual void Visit(std::shared_ptr<CallParamsNode> node) = 0;
  virtual void Visit(std::shared_ptr<ExpressionNode> node) = 0;
  virtual void Visit(std::shared_ptr<ShorthandSelfNode> node) = 0;
  virtual void Visit(std::shared_ptr<TypedSelfNode> node) = 0;
  virtual void Visit(std::shared_ptr<SelfParamNode> node) = 0;
  virtual void Visit(std::shared_ptr<FunctionParamNode> node) = 0;
  virtual void Visit(std::shared_ptr<FunctionParametersNode> node) = 0;
  virtual void Visit(std::shared_ptr<FunctionReturnTypeNode> node) = 0;
  virtual void Visit(std::shared_ptr<FunctionNode> node) = 0;
  virtual void Visit(std::shared_ptr<ImplementationNode> node) = 0;
  virtual void Visit(std::shared_ptr<ConstantItemNode> node) = 0;
  virtual void Visit(std::shared_ptr<AssociatedItemNode> node) = 0;
  virtual void Visit(std::shared_ptr<ItemNode> node) = 0;
  virtual void Visit(std::shared_ptr<PathIdentSegmentNode> node) = 0;
  virtual void Visit(std::shared_ptr<LiteralPatternNode> node) = 0;
  virtual void Visit(std::shared_ptr<IdentifierPatternNode> node) = 0;
  virtual void Visit(std::shared_ptr<ReferencePatternNode> node) = 0;
  virtual void Visit(std::shared_ptr<TupleStructItemsNode> node) = 0;
  virtual void Visit(std::shared_ptr<TupleStructPatternNode> node) = 0;
  virtual void Visit(std::shared_ptr<PatternWithoutRangeNode> node) = 0;
  virtual void Visit(std::shared_ptr<LetStatementNode> node) = 0;
  virtual void Visit(std::shared_ptr<ExpressionStatementNode> node) = 0;
  virtual void Visit(std::shared_ptr<StatementNode> node) = 0;
  virtual void Visit(std::shared_ptr<StatementsNode> node) = 0;
  virtual void Visit(std::shared_ptr<StructFieldNode> node) = 0;
  virtual void Visit(std::shared_ptr<StructFieldsNode> node) = 0;
  virtual void Visit(std::shared_ptr<StructNode> node) = 0;
  virtual void Visit(std::shared_ptr<IdentifierNode> node) = 0;
  virtual void Visit(std::shared_ptr<CharLiteralNode> node) = 0;
  virtual void Visit(std::shared_ptr<StringLiteralNode> node) = 0;
  virtual void Visit(std::shared_ptr<RawStringLiteralNode> node) = 0;
  virtual void Visit(std::shared_ptr<CStringLiteralNode> node) = 0;
  virtual void Visit(std::shared_ptr<RawCStringLiteralNode> node) = 0;
  virtual void Visit(std::shared_ptr<IntegerLiteralNode> node) = 0;
  virtual void Visit(std::shared_ptr<TrueNode> node) = 0;
  virtual void Visit(std::shared_ptr<FalseNode> node) = 0;
  virtual void Visit(std::shared_ptr<SuperNode> node) = 0;
  virtual void Visit(std::shared_ptr<SelfLowerNode> node) = 0;
  virtual void Visit(std::shared_ptr<SelfUpperNode> node) = 0;
  virtual void Visit(std::shared_ptr<UnderscoreExpressionNode> node) = 0;
  virtual void Visit(std::shared_ptr<ContinueExpressionNode> node) = 0;
  virtual void Visit(std::shared_ptr<TraitNode> node) = 0;
  virtual void Visit(std::shared_ptr<ReferenceTypeNode> node) = 0;
  virtual void Visit(std::shared_ptr<ArrayTypeNode> node) = 0;
  virtual void Visit(std::shared_ptr<UnitTypeNode> node) = 0;
  virtual void Visit(std::shared_ptr<TypeNoBoundsNode> node) = 0;
};

#endif //VISITOR_BASE_H
