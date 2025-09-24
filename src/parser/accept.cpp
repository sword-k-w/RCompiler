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
#include "common/error.h"

void CrateNode::Accept(VisitorBase * ptr) {
  try {
    ptr->Visit(this);
  } catch (Error &) { throw; }
}
void EnumVariantsNode::Accept(VisitorBase * ptr) {
  try {
    ptr->Visit(this);  
  } catch (Error &) { throw; }
}
void EnumerationNode::Accept(VisitorBase * ptr) {
  try {
    ptr->Visit(this);  
  } catch (Error &) { throw; }
}
void LiteralExpressionNode::Accept(VisitorBase * ptr) {
  try {
    ptr->Visit(this);  
  } catch (Error &) { throw; }
}
void ArrayElementsNode::Accept(VisitorBase * ptr) {
  try {
    ptr->Visit(this);  
  } catch (Error &) { throw; }
}
void ArrayExpressionNode::Accept(VisitorBase * ptr) {
  try {
    ptr->Visit(this);  
  } catch (Error &) { throw; }
}
void PathInExpressionNode::Accept(VisitorBase * ptr) {
  try {
    ptr->Visit(this);  
  } catch (Error &) { throw; }
}
void StructExprFieldNode::Accept(VisitorBase * ptr) {
  try {
    ptr->Visit(this);  
  } catch (Error &) { throw; }
}
void StructExprFieldsNode::Accept(VisitorBase * ptr) {
  try {
    ptr->Visit(this);  
  } catch (Error &) { throw; }
}
void StructExpressionNode::Accept(VisitorBase * ptr) {
  try {
    ptr->Visit(this);  
  } catch (Error &) { throw; }
}
void ExpressionWithoutBlockNode::Accept(VisitorBase * ptr) {
  try {
    ptr->Visit(this);  
  } catch (Error &) { throw; }
}
void BlockExpressionNode::Accept(VisitorBase * ptr) {
  try {
    ptr->Visit(this);  
  } catch (Error &) { throw; }
}
void InfiniteLoopExpressionNode::Accept(VisitorBase * ptr) {
  try {
    ptr->Visit(this);  
  } catch (Error &) { throw; }
}
void ConditionsNode::Accept(VisitorBase * ptr) {
  try {
    ptr->Visit(this);  
  } catch (Error &) { throw; }
}
void PredicateLoopExpressionNode::Accept(VisitorBase * ptr) {
  try {
    ptr->Visit(this);  
  } catch (Error &) { throw; }
}
void LoopExpressionNode::Accept(VisitorBase * ptr) {
  try {
    ptr->Visit(this);  
  } catch (Error &) { throw; }
}
void IfExpressionNode::Accept(VisitorBase * ptr) {
  try {
    ptr->Visit(this);  
  } catch (Error &) { throw; }
}
void ExpressionWithBlockNode::Accept(VisitorBase * ptr) {
  try {
    ptr->Visit(this);  
  } catch (Error &) { throw; }
}
void CallParamsNode::Accept(VisitorBase * ptr) {
  try {
    ptr->Visit(this);  
  } catch (Error &) { throw; }
}
void ExpressionNode::Accept(VisitorBase * ptr) {
  try {
    ptr->Visit(this);  
  } catch (Error &) { throw; }
}
void ShorthandSelfNode::Accept(VisitorBase * ptr) {
  try {
    ptr->Visit(this);  
  } catch (Error &) { throw; }
}
void SelfParamNode::Accept(VisitorBase * ptr) {
  try {
    ptr->Visit(this);  
  } catch (Error &) { throw; }
}
void FunctionParamNode::Accept(VisitorBase * ptr) {
  try {
    ptr->Visit(this);  
  } catch (Error &) { throw; }
}
void FunctionParametersNode::Accept(VisitorBase * ptr) {
  try {
    ptr->Visit(this);  
  } catch (Error &) { throw; }
}
void FunctionReturnTypeNode::Accept(VisitorBase * ptr) {
  try {
    ptr->Visit(this);  
  } catch (Error &) { throw; }
}
void FunctionNode::Accept(VisitorBase * ptr) {
  try {
    ptr->Visit(this);  
  } catch (Error &) { throw; }
}
void ImplementationNode::Accept(VisitorBase * ptr) {
  try {
    ptr->Visit(this);  
  } catch (Error &) { throw; }
}
void ConstantItemNode::Accept(VisitorBase * ptr) {
  try {
    ptr->Visit(this);  
  } catch (Error &) { throw; }
}
void AssociatedItemNode::Accept(VisitorBase * ptr) {
  try {
    ptr->Visit(this);  
  } catch (Error &) { throw; }
}
void ItemNode::Accept(VisitorBase * ptr) {
  try {
    ptr->Visit(this);  
  } catch (Error &) { throw; }
}
void PathIdentSegmentNode::Accept(VisitorBase * ptr) {
  try {
    ptr->Visit(this);  
  } catch (Error &) { throw; }
}
void IdentifierPatternNode::Accept(VisitorBase * ptr) {
  try {
    ptr->Visit(this);  
  } catch (Error &) { throw; }
}
void ReferencePatternNode::Accept(VisitorBase * ptr) {
  try {
    ptr->Visit(this);  
  } catch (Error &) { throw; }
}
void PatternWithoutRangeNode::Accept(VisitorBase * ptr) {
  try {
    ptr->Visit(this);  
  } catch (Error &) { throw; }
}
void LetStatementNode::Accept(VisitorBase * ptr) {
  try {
    ptr->Visit(this);  
  } catch (Error &) { throw; }
}
void ExpressionStatementNode::Accept(VisitorBase * ptr) {
  try {
    ptr->Visit(this);  
  } catch (Error &) { throw; }
}
void StatementNode::Accept(VisitorBase * ptr) {
  try {
    ptr->Visit(this);  
  } catch (Error &) { throw; }
}
void StatementsNode::Accept(VisitorBase * ptr) {
  try {
    ptr->Visit(this);  
  } catch (Error &) { throw; }
}
void StructFieldNode::Accept(VisitorBase * ptr) {
  try {
    ptr->Visit(this);  
  } catch (Error &) { throw; }
}
void StructFieldsNode::Accept(VisitorBase * ptr) {
  try {
    ptr->Visit(this);  
  } catch (Error &) { throw; }
}
void StructNode::Accept(VisitorBase * ptr) {
  try {
    ptr->Visit(this);  
  } catch (Error &) { throw; }
}
void IdentifierNode::Accept(VisitorBase * ptr) {
  try {
    ptr->Visit(this);  
  } catch (Error &) { throw; }
}
void CharLiteralNode::Accept(VisitorBase * ptr) {
  try {
    ptr->Visit(this);  
  } catch (Error &) { throw; }
}
void StringLiteralNode::Accept(VisitorBase * ptr) {
  try {
    ptr->Visit(this);  
  } catch (Error &) { throw; }
}
void RawStringLiteralNode::Accept(VisitorBase * ptr) {
  try {
    ptr->Visit(this);  
  } catch (Error &) { throw; }
}
void CStringLiteralNode::Accept(VisitorBase * ptr) {
  try {
    ptr->Visit(this);  
  } catch (Error &) { throw; }
}
void RawCStringLiteralNode::Accept(VisitorBase * ptr) {
  try {
    ptr->Visit(this);  
  } catch (Error &) { throw; }
}
void IntegerLiteralNode::Accept(VisitorBase * ptr) {
  try {
    ptr->Visit(this);  
  } catch (Error &) { throw; }
}
void TrueNode::Accept(VisitorBase * ptr) {
  try {
    ptr->Visit(this);  
  } catch (Error &) { throw; }
}
void FalseNode::Accept(VisitorBase * ptr) {
  try {
    ptr->Visit(this);  
  } catch (Error &) { throw; }
}
void SelfLowerNode::Accept(VisitorBase * ptr) {
  try {
    ptr->Visit(this);  
  } catch (Error &) { throw; }
}
void SelfUpperNode::Accept(VisitorBase * ptr) {
  try {
    ptr->Visit(this);  
  } catch (Error &) { throw; }
}
void UnderscoreExpressionNode::Accept(VisitorBase * ptr) {
  try {
    ptr->Visit(this);  
  } catch (Error &) { throw; }
}
void ContinueExpressionNode::Accept(VisitorBase * ptr) {
  try {
    ptr->Visit(this);  
  } catch (Error &) { throw; }
}
void TraitNode::Accept(VisitorBase * ptr) {
  try {
    ptr->Visit(this);  
  } catch (Error &) { throw; }
}
void ReferenceTypeNode::Accept(VisitorBase * ptr) {
  try {
    ptr->Visit(this);  
  } catch (Error &) { throw; }
}
void ArrayTypeNode::Accept(VisitorBase * ptr) {
  try {
    ptr->Visit(this);  
  } catch (Error &) { throw; }
}
void UnitTypeNode::Accept(VisitorBase * ptr) {
  try {
    ptr->Visit(this);  
  } catch (Error &) { throw; }
}
void TypeNoBoundsNode::Accept(VisitorBase * ptr) {
  try {
    ptr->Visit(this);  
  } catch (Error &) { throw; }
}
