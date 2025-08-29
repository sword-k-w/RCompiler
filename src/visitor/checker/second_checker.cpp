#include "visitor/checker/second_checker.h"

#include "parser/node/enumeration.h"
#include "parser/node/expression.h"
#include "parser/node/function.h"
#include "parser/node/implementation.h"
#include "parser/node/struct.h"
#include "parser/node/terminal.h"
#include "parser/node/trait.h"
#include "parser/node/type.h"
#include "parser/node/pattern.h"
#include "parser/node/crate.h"
#include "parser/node/item.h"
#include "parser/node/path.h"
#include "parser/node/statement.h"
#include "common/error.h"

void SecondChecker::Visit(CrateNode *node) {
  try {
    for (auto &item : node->items_) {
      item->Accept(this);
    }
  } catch (Error &) {
    throw;
  }
}

void SecondChecker::Visit(EnumVariantsNode *node) {

}

void SecondChecker::Visit(EnumerationNode *node) {

}

void SecondChecker::Visit(LiteralExpressionNode *node) {}
void SecondChecker::Visit(ArrayElementsNode *node) {}
void SecondChecker::Visit(ArrayExpressionNode *node) {}
void SecondChecker::Visit(PathInExpressionNode *node) {}
void SecondChecker::Visit(StructExprFieldNode *node) {}
void SecondChecker::Visit(StructExprFieldsNode *node) {}
void SecondChecker::Visit(StructExpressionNode *node) {}
void SecondChecker::Visit(ExpressionWithoutBlockNode *node) {}
void SecondChecker::Visit(BlockExpressionNode *node) {}
void SecondChecker::Visit(InfiniteLoopExpressionNode *node) {}
void SecondChecker::Visit(ConditionsNode *node) {}
void SecondChecker::Visit(PredicateLoopExpressionNode *node) {}
void SecondChecker::Visit(LoopExpressionNode *node) {}
void SecondChecker::Visit(IfExpressionNode *node) {}
void SecondChecker::Visit(ExpressionWithBlockNode *node) {}
void SecondChecker::Visit(CallParamsNode *node) {}
void SecondChecker::Visit(ExpressionNode *node) {}
void SecondChecker::Visit(ShorthandSelfNode *node) {}
void SecondChecker::Visit(TypedSelfNode *node) {}
void SecondChecker::Visit(SelfParamNode *node) {}
void SecondChecker::Visit(FunctionParamNode *node) {}
void SecondChecker::Visit(FunctionParametersNode *node) {}
void SecondChecker::Visit(FunctionReturnTypeNode *node) {}
void SecondChecker::Visit(FunctionNode *node) {}
void SecondChecker::Visit(ImplementationNode *node) {}
void SecondChecker::Visit(ConstantItemNode *node) {}
void SecondChecker::Visit(AssociatedItemNode *node) {}

void SecondChecker::Visit(ItemNode *node) {
  try {
    if (node->function_ != nullptr) {
      node->function_->Accept(this);
    } else if (node->struct_ != nullptr) {
      node->struct_->Accept(this);
    } else if (node->enumeration_ != nullptr) {
      node->enumeration_->Accept(this);
    } else if (node->constant_item_ != nullptr) {
      node->constant_item_->Accept(this);
    } else if (node->trait_ != nullptr) {
      node->trait_->Accept(this);
    } else {
      node->implementation_->Accept(this);
    }
  } catch (Error &) {
    throw;
  }
}

void SecondChecker::Visit(PathIdentSegmentNode *node) {}
void SecondChecker::Visit(LiteralPatternNode *node) {}
void SecondChecker::Visit(IdentifierPatternNode *node) {}
void SecondChecker::Visit(ReferencePatternNode *node) {}
void SecondChecker::Visit(PatternWithoutRangeNode *node) {}
void SecondChecker::Visit(LetStatementNode *node) {}
void SecondChecker::Visit(ExpressionStatementNode *node) {}
void SecondChecker::Visit(StatementNode *node) {}
void SecondChecker::Visit(StatementsNode *node) {}
void SecondChecker::Visit(StructFieldNode *node) {}
void SecondChecker::Visit(StructFieldsNode *node) {}
void SecondChecker::Visit(StructNode *node) {}
void SecondChecker::Visit(IdentifierNode *node) {}
void SecondChecker::Visit(CharLiteralNode *node) {}
void SecondChecker::Visit(StringLiteralNode *node) {}
void SecondChecker::Visit(RawStringLiteralNode *node) {}
void SecondChecker::Visit(CStringLiteralNode *node) {}
void SecondChecker::Visit(RawCStringLiteralNode *node) {}
void SecondChecker::Visit(IntegerLiteralNode *node) {}
void SecondChecker::Visit(TrueNode *node) {}
void SecondChecker::Visit(FalseNode *node) {}
void SecondChecker::Visit(SuperNode *node) {}
void SecondChecker::Visit(SelfLowerNode *node) {}
void SecondChecker::Visit(SelfUpperNode *node) {}
void SecondChecker::Visit(UnderscoreExpressionNode *node) {}
void SecondChecker::Visit(ContinueExpressionNode *node) {}
void SecondChecker::Visit(TraitNode *node) {}
void SecondChecker::Visit(ReferenceTypeNode *node) {}
void SecondChecker::Visit(ArrayTypeNode *node) {}
void SecondChecker::Visit(UnitTypeNode *node) {}
void SecondChecker::Visit(TypeNoBoundsNode *node) {}