#include "visitor/checker/first_checker.h"

#include "parser/node/crate.h"
#include "parser/node/item.h"

void FirstChecker::Visit(CrateNode *node) {
  for (auto &item : node->items_) {
    item->Accept(this);
  }
}

void FirstChecker::Visit(EnumVariantsNode *node) {}
void FirstChecker::Visit(EnumerationNode *node) {}
void FirstChecker::Visit(LiteralExpressionNode *node) {}
void FirstChecker::Visit(ArrayElementsNode *node) {}
void FirstChecker::Visit(ArrayExpressionNode *node) {}
void FirstChecker::Visit(PathInExpressionNode *node) {}
void FirstChecker::Visit(StructExprFieldNode *node) {}
void FirstChecker::Visit(StructExprFieldsNode *node) {}
void FirstChecker::Visit(StructExpressionNode *node) {}
void FirstChecker::Visit(ExpressionWithoutBlockNode *node) {}
void FirstChecker::Visit(BlockExpressionNode *node) {}
void FirstChecker::Visit(ConstBlockExpressionNode *node) {}
void FirstChecker::Visit(InfiniteLoopExpressionNode *node) {}
void FirstChecker::Visit(ConditionsNode *node) {}
void FirstChecker::Visit(PredicateLoopExpressionNode *node) {}
void FirstChecker::Visit(LoopExpressionNode *node) {}
void FirstChecker::Visit(IfExpressionNode *node) {}
void FirstChecker::Visit(ExpressionWithBlockNode *node) {}
void FirstChecker::Visit(CallParamsNode *node) {}
void FirstChecker::Visit(ExpressionNode *node) {}
void FirstChecker::Visit(ShorthandSelfNode *node) {}
void FirstChecker::Visit(TypedSelfNode *node) {}
void FirstChecker::Visit(SelfParamNode *node) {}
void FirstChecker::Visit(FunctionParamNode *node) {}
void FirstChecker::Visit(FunctionParametersNode *node) {}
void FirstChecker::Visit(FunctionReturnTypeNode *node) {}
void FirstChecker::Visit(FunctionNode *node) {}
void FirstChecker::Visit(ImplementationNode *node) {}
void FirstChecker::Visit(ConstantItemNode *node) {}
void FirstChecker::Visit(AssociatedItemNode *node) {}
void FirstChecker::Visit(ItemNode *node) {}
void FirstChecker::Visit(PathIdentSegmentNode *node) {}
void FirstChecker::Visit(LiteralPatternNode *node) {}
void FirstChecker::Visit(IdentifierPatternNode *node) {}
void FirstChecker::Visit(ReferencePatternNode *node) {}
void FirstChecker::Visit(TupleStructItemsNode *node) {}
void FirstChecker::Visit(TupleStructPatternNode *node) {}
void FirstChecker::Visit(PatternWithoutRangeNode *node) {}
void FirstChecker::Visit(LetStatementNode *node) {}
void FirstChecker::Visit(ExpressionStatementNode *node) {}
void FirstChecker::Visit(StatementNode *node) {}
void FirstChecker::Visit(StatementsNode *node) {}
void FirstChecker::Visit(StructFieldNode *node) {}
void FirstChecker::Visit(StructFieldsNode *node) {}
void FirstChecker::Visit(StructNode *node) {}
void FirstChecker::Visit(IdentifierNode *node) {}
void FirstChecker::Visit(CharLiteralNode *node) {}
void FirstChecker::Visit(StringLiteralNode *node) {}
void FirstChecker::Visit(RawStringLiteralNode *node) {}
void FirstChecker::Visit(CStringLiteralNode *node) {}
void FirstChecker::Visit(RawCStringLiteralNode *node) {}
void FirstChecker::Visit(IntegerLiteralNode *node) {}
void FirstChecker::Visit(TrueNode *node) {}
void FirstChecker::Visit(FalseNode *node) {}
void FirstChecker::Visit(SuperNode *node) {}
void FirstChecker::Visit(SelfLowerNode *node) {}
void FirstChecker::Visit(SelfUpperNode *node) {}
void FirstChecker::Visit(UnderscoreExpressionNode *node) {}
void FirstChecker::Visit(ContinueExpressionNode *node) {}
void FirstChecker::Visit(TraitNode *node) {}
void FirstChecker::Visit(ReferenceTypeNode *node) {}
void FirstChecker::Visit(ArrayTypeNode *node) {}
void FirstChecker::Visit(UnitTypeNode *node) {}
void FirstChecker::Visit(TypeNoBoundsNode *node) {}