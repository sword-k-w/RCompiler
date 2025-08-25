#include "parser/node/crate.h"
#include "parser/node/enumeration.h"
#include "parser/node/expression.h"
#include "parser/node/function.h"
#include "parser/node/implementation.h"
#include "parser/node/item.h"
#include "parser/node/path.h"
#include "parser/node/pattern.h"
#include "parser/node/statement.h"
#include "parser/node/struct.h"
#include "parser/node/terminal.h"
#include "parser/node/trait.h"
#include "parser/node/type.h"

void CrateNode::Accept(VisitorBase * ptr) {
  ptr->Visit(this);
}
void EnumVariantsNode::Accept(VisitorBase * ptr) {
  ptr->Visit(this);
}
void EnumerationNode::Accept(VisitorBase * ptr) {
  ptr->Visit(this);
}
void LiteralExpressionNode::Accept(VisitorBase * ptr) {
  ptr->Visit(this);
}
void ArrayElementsNode::Accept(VisitorBase * ptr) {
  ptr->Visit(this);
}
void ArrayExpressionNode::Accept(VisitorBase * ptr) {
  ptr->Visit(this);
}
void PathInExpressionNode::Accept(VisitorBase * ptr) {
  ptr->Visit(this);
}
void StructExprFieldNode::Accept(VisitorBase * ptr) {
  ptr->Visit(this);
}
void StructExprFieldsNode::Accept(VisitorBase * ptr) {
  ptr->Visit(this);
}
void StructExpressionNode::Accept(VisitorBase * ptr) {
  ptr->Visit(this);
}
void ExpressionWithoutBlockNode::Accept(VisitorBase * ptr) {
  ptr->Visit(this);
}
void BlockExpressionNode::Accept(VisitorBase * ptr) {
  ptr->Visit(this);
}
void ConstBlockExpressionNode::Accept(VisitorBase * ptr) {
  ptr->Visit(this);
}
void InfiniteLoopExpressionNode::Accept(VisitorBase * ptr) {
  ptr->Visit(this);
}
void ConditionsNode::Accept(VisitorBase * ptr) {
  ptr->Visit(this);
}
void PredicateLoopExpressionNode::Accept(VisitorBase * ptr) {
  ptr->Visit(this);
}
void LoopExpressionNode::Accept(VisitorBase * ptr) {
  ptr->Visit(this);
}
void IfExpressionNode::Accept(VisitorBase * ptr) {
  ptr->Visit(this);
}
void ExpressionWithBlockNode::Accept(VisitorBase * ptr) {
  ptr->Visit(this);
}
void CallParamsNode::Accept(VisitorBase * ptr) {
  ptr->Visit(this);
}
void ExpressionNode::Accept(VisitorBase * ptr) {
  ptr->Visit(this);
}
void ShorthandSelfNode::Accept(VisitorBase * ptr) {
  ptr->Visit(this);
}
void TypedSelfNode::Accept(VisitorBase * ptr) {
  ptr->Visit(this);
}
void SelfParamNode::Accept(VisitorBase * ptr) {
  ptr->Visit(this);
}
void FunctionParamNode::Accept(VisitorBase * ptr) {
  ptr->Visit(this);
}
void FunctionParametersNode::Accept(VisitorBase * ptr) {
  ptr->Visit(this);
}
void FunctionReturnTypeNode::Accept(VisitorBase * ptr) {
  ptr->Visit(this);
}
void FunctionNode::Accept(VisitorBase * ptr) {
  ptr->Visit(this);
}
void ImplementationNode::Accept(VisitorBase * ptr) {
  ptr->Visit(this);
}
void ConstantItemNode::Accept(VisitorBase * ptr) {
  ptr->Visit(this);
}
void AssociatedItemNode::Accept(VisitorBase * ptr) {
  ptr->Visit(this);
}
void ItemNode::Accept(VisitorBase * ptr) {
  ptr->Visit(this);
}
void PathIdentSegmentNode::Accept(VisitorBase * ptr) {
  ptr->Visit(this);
}
void LiteralPatternNode::Accept(VisitorBase * ptr) {
  ptr->Visit(this);
}
void IdentifierPatternNode::Accept(VisitorBase * ptr) {
  ptr->Visit(this);
}
void ReferencePatternNode::Accept(VisitorBase * ptr) {
  ptr->Visit(this);
}
void TupleStructItemsNode::Accept(VisitorBase * ptr) {
  ptr->Visit(this);
}
void TupleStructPatternNode::Accept(VisitorBase * ptr) {
  ptr->Visit(this);
}
void PatternWithoutRangeNode::Accept(VisitorBase * ptr) {
  ptr->Visit(this);
}
void LetStatementNode::Accept(VisitorBase * ptr) {
  ptr->Visit(this);
}
void ExpressionStatementNode::Accept(VisitorBase * ptr) {
  ptr->Visit(this);
}
void StatementNode::Accept(VisitorBase * ptr) {
  ptr->Visit(this);
}
void StatementsNode::Accept(VisitorBase * ptr) {
  ptr->Visit(this);
}
void StructFieldNode::Accept(VisitorBase * ptr) {
  ptr->Visit(this);
}
void StructFieldsNode::Accept(VisitorBase * ptr) {
  ptr->Visit(this);
}
void StructNode::Accept(VisitorBase * ptr) {
  ptr->Visit(this);
}
void IdentifierNode::Accept(VisitorBase * ptr) {
  ptr->Visit(this);
}
void CharLiteralNode::Accept(VisitorBase * ptr) {
  ptr->Visit(this);
}
void StringLiteralNode::Accept(VisitorBase * ptr) {
  ptr->Visit(this);
}
void RawStringLiteralNode::Accept(VisitorBase * ptr) {
  ptr->Visit(this);
}
void CStringLiteralNode::Accept(VisitorBase * ptr) {
  ptr->Visit(this);
}
void RawCStringLiteralNode::Accept(VisitorBase * ptr) {
  ptr->Visit(this);
}
void IntegerLiteralNode::Accept(VisitorBase * ptr) {
  ptr->Visit(this);
}
void TrueNode::Accept(VisitorBase * ptr) {
  ptr->Visit(this);
}
void FalseNode::Accept(VisitorBase * ptr) {
  ptr->Visit(this);
}
void SuperNode::Accept(VisitorBase * ptr) {
  ptr->Visit(this);
}
void SelfLowerNode::Accept(VisitorBase * ptr) {
  ptr->Visit(this);
}
void SelfUpperNode::Accept(VisitorBase * ptr) {
  ptr->Visit(this);
}
void UnderscoreExpressionNode::Accept(VisitorBase * ptr) {
  ptr->Visit(this);
}
void ContinueExpressionNode::Accept(VisitorBase * ptr) {
  ptr->Visit(this);
}
void TraitNode::Accept(VisitorBase * ptr) {
  ptr->Visit(this);
}
void ReferenceTypeNode::Accept(VisitorBase * ptr) {
  ptr->Visit(this);
}
void ArrayTypeNode::Accept(VisitorBase * ptr) {
  ptr->Visit(this);
}
void UnitTypeNode::Accept(VisitorBase * ptr) {
  ptr->Visit(this);
}
void TypeNoBoundsNode::Accept(VisitorBase * ptr) {
  ptr->Visit(this);
}
