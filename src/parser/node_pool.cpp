#include "parser/node_pool.h"
#include "common/error.h"
#include "parser/node/AST_node.h"
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

template<class T>
T *NodePool::Make(const std::vector<Token> &tokens, uint32_t &pos, const uint32_t &length, const uint32_t &context_precedence) {
  try {
    T *ptr = static_cast<T *>(operator new(sizeof(T)));
    pool_.emplace_back(ptr);
    new(ptr) T(tokens, pos, length, context_precedence);
    return ptr;
  } catch (Error &err) {
    throw err;
  }
}

template<class T>
T *NodePool::Make(const std::vector<Token> &tokens, uint32_t &pos, const uint32_t &length) {
  try {
    T *ptr = static_cast<T *>(operator new(sizeof(T)));
    pool_.emplace_back(ptr);
    new(ptr) T(tokens, pos, length);
    return ptr;
  } catch (Error &err) {
    throw err;
  }
}

template<class T>
T *NodePool::Make(const T &other) {
  try {
    T *ptr = static_cast<T *>(operator new(sizeof(T)));
    pool_.emplace_back(ptr);
    new(ptr) T(other);
    return ptr;
  } catch (Error &err) {
    throw err;
  }
}

void NodePool::Clear() {
  while (!pool_.empty()) {
    ASTNode *ptr = pool_.back();
    pool_.pop_back();
    delete ptr;
  }
}

template CrateNode *NodePool::Make<CrateNode>(const std::vector<Token> & , uint32_t &, const uint32_t &);
template EnumVariantsNode *NodePool::Make<EnumVariantsNode>(const std::vector<Token> & , uint32_t &, const uint32_t &);
template EnumerationNode *NodePool::Make<EnumerationNode>(const std::vector<Token> & , uint32_t &, const uint32_t &);
template LiteralExpressionNode *NodePool::Make<LiteralExpressionNode>(const std::vector<Token> & , uint32_t &, const uint32_t &);
template ArrayElementsNode *NodePool::Make<ArrayElementsNode>(const std::vector<Token> & , uint32_t &, const uint32_t &);
template ArrayExpressionNode *NodePool::Make<ArrayExpressionNode>(const std::vector<Token> & , uint32_t &, const uint32_t &);
template PathInExpressionNode *NodePool::Make<PathInExpressionNode>(const std::vector<Token> & , uint32_t &, const uint32_t &);
template StructExprFieldNode *NodePool::Make<StructExprFieldNode>(const std::vector<Token> & , uint32_t &, const uint32_t &);
template StructExprFieldsNode *NodePool::Make<StructExprFieldsNode>(const std::vector<Token> & , uint32_t &, const uint32_t &);
template StructExpressionNode *NodePool::Make<StructExpressionNode>(const std::vector<Token> & , uint32_t &, const uint32_t &);
template ExpressionWithoutBlockNode *NodePool::Make<ExpressionWithoutBlockNode>(const std::vector<Token> & , uint32_t &, const uint32_t &);
template BlockExpressionNode *NodePool::Make<BlockExpressionNode>(const std::vector<Token> & , uint32_t &, const uint32_t &);
template ConstBlockExpressionNode *NodePool::Make<ConstBlockExpressionNode>(const std::vector<Token> & , uint32_t &, const uint32_t &);
template InfiniteLoopExpressionNode *NodePool::Make<InfiniteLoopExpressionNode>(const std::vector<Token> & , uint32_t &, const uint32_t &);
template ConditionsNode *NodePool::Make<ConditionsNode>(const std::vector<Token> & , uint32_t &, const uint32_t &);
template PredicateLoopExpressionNode *NodePool::Make<PredicateLoopExpressionNode>(const std::vector<Token> & , uint32_t &, const uint32_t &);
template LoopExpressionNode *NodePool::Make<LoopExpressionNode>(const std::vector<Token> & , uint32_t &, const uint32_t &);
template IfExpressionNode *NodePool::Make<IfExpressionNode>(const std::vector<Token> & , uint32_t &, const uint32_t &);
template ExpressionWithBlockNode *NodePool::Make<ExpressionWithBlockNode>(const std::vector<Token> & , uint32_t &, const uint32_t &);
template CallParamsNode *NodePool::Make<CallParamsNode>(const std::vector<Token> & , uint32_t &, const uint32_t &);
template ExpressionNode *NodePool::Make<ExpressionNode>(const std::vector<Token> & , uint32_t &, const uint32_t &, const uint32_t &);
template ExpressionNode *NodePool::Make<ExpressionNode>(const std::vector<Token> & , uint32_t &, const uint32_t &);
template ExpressionNode *NodePool::Make<ExpressionNode>(const ExpressionNode &);
template ShorthandSelfNode *NodePool::Make<ShorthandSelfNode>(const std::vector<Token> & , uint32_t &, const uint32_t &);
template TypedSelfNode *NodePool::Make<TypedSelfNode>(const std::vector<Token> & , uint32_t &, const uint32_t &);
template SelfParamNode *NodePool::Make<SelfParamNode>(const std::vector<Token> & , uint32_t &, const uint32_t &);
template FunctionParamNode *NodePool::Make<FunctionParamNode>(const std::vector<Token> & , uint32_t &, const uint32_t &);
template FunctionParametersNode *NodePool::Make<FunctionParametersNode>(const std::vector<Token> & , uint32_t &, const uint32_t &);
template FunctionReturnTypeNode *NodePool::Make<FunctionReturnTypeNode>(const std::vector<Token> & , uint32_t &, const uint32_t &);
template FunctionNode *NodePool::Make<FunctionNode>(const std::vector<Token> & , uint32_t &, const uint32_t &);
template ImplementationNode *NodePool::Make<ImplementationNode>(const std::vector<Token> & , uint32_t &, const uint32_t &);
template ConstantItemNode *NodePool::Make<ConstantItemNode>(const std::vector<Token> & , uint32_t &, const uint32_t &);
template AssociatedItemNode *NodePool::Make<AssociatedItemNode>(const std::vector<Token> & , uint32_t &, const uint32_t &);
template ItemNode *NodePool::Make<ItemNode>(const std::vector<Token> & , uint32_t &, const uint32_t &);
template PathIdentSegmentNode *NodePool::Make<PathIdentSegmentNode>(const std::vector<Token> & , uint32_t &, const uint32_t &);
template LiteralPatternNode *NodePool::Make<LiteralPatternNode>(const std::vector<Token> & , uint32_t &, const uint32_t &);
template IdentifierPatternNode *NodePool::Make<IdentifierPatternNode>(const std::vector<Token> & , uint32_t &, const uint32_t &);
template ReferencePatternNode *NodePool::Make<ReferencePatternNode>(const std::vector<Token> & , uint32_t &, const uint32_t &);
template TupleStructItemsNode *NodePool::Make<TupleStructItemsNode>(const std::vector<Token> & , uint32_t &, const uint32_t &);
template TupleStructPatternNode *NodePool::Make<TupleStructPatternNode>(const std::vector<Token> & , uint32_t &, const uint32_t &);
template PatternWithoutRangeNode *NodePool::Make<PatternWithoutRangeNode>(const std::vector<Token> & , uint32_t &, const uint32_t &);
template LetStatementNode *NodePool::Make<LetStatementNode>(const std::vector<Token> & , uint32_t &, const uint32_t &);
template ExpressionStatementNode *NodePool::Make<ExpressionStatementNode>(const std::vector<Token> & , uint32_t &, const uint32_t &);
template StatementNode *NodePool::Make<StatementNode>(const std::vector<Token> & , uint32_t &, const uint32_t &);
template StatementsNode *NodePool::Make<StatementsNode>(const std::vector<Token> & , uint32_t &, const uint32_t &);
template StructFieldNode *NodePool::Make<StructFieldNode>(const std::vector<Token> & , uint32_t &, const uint32_t &);
template StructFieldsNode *NodePool::Make<StructFieldsNode>(const std::vector<Token> & , uint32_t &, const uint32_t &);
template StructNode *NodePool::Make<StructNode>(const std::vector<Token> & , uint32_t &, const uint32_t &);
template IdentifierNode *NodePool::Make<IdentifierNode>(const std::vector<Token> & , uint32_t &, const uint32_t &);
template CharLiteralNode *NodePool::Make<CharLiteralNode>(const std::vector<Token> & , uint32_t &, const uint32_t &);
template StringLiteralNode *NodePool::Make<StringLiteralNode>(const std::vector<Token> & , uint32_t &, const uint32_t &);
template RawStringLiteralNode *NodePool::Make<RawStringLiteralNode>(const std::vector<Token> & , uint32_t &, const uint32_t &);
template CStringLiteralNode *NodePool::Make<CStringLiteralNode>(const std::vector<Token> & , uint32_t &, const uint32_t &);
template RawCStringLiteralNode *NodePool::Make<RawCStringLiteralNode>(const std::vector<Token> & , uint32_t &, const uint32_t &);
template IntegerLiteralNode *NodePool::Make<IntegerLiteralNode>(const std::vector<Token> & , uint32_t &, const uint32_t &);
template TrueNode *NodePool::Make<TrueNode>(const std::vector<Token> & , uint32_t &, const uint32_t &);
template FalseNode *NodePool::Make<FalseNode>(const std::vector<Token> & , uint32_t &, const uint32_t &);
template SuperNode *NodePool::Make<SuperNode>(const std::vector<Token> & , uint32_t &, const uint32_t &);
template SelfLowerNode *NodePool::Make<SelfLowerNode>(const std::vector<Token> & , uint32_t &, const uint32_t &);
template SelfUpperNode *NodePool::Make<SelfUpperNode>(const std::vector<Token> & , uint32_t &, const uint32_t &);
template UnderscoreExpressionNode *NodePool::Make<UnderscoreExpressionNode>(const std::vector<Token> & , uint32_t &, const uint32_t &);
template ContinueExpressionNode *NodePool::Make<ContinueExpressionNode>(const std::vector<Token> & , uint32_t &, const uint32_t &);
template TraitNode *NodePool::Make<TraitNode>(const std::vector<Token> & , uint32_t &, const uint32_t &);
template ReferenceTypeNode *NodePool::Make<ReferenceTypeNode>(const std::vector<Token> & , uint32_t &, const uint32_t &);
template ArrayTypeNode *NodePool::Make<ArrayTypeNode>(const std::vector<Token> & , uint32_t &, const uint32_t &);
template UnitTypeNode *NodePool::Make<UnitTypeNode>(const std::vector<Token> & , uint32_t &, const uint32_t &);
template TypeNoBoundsNode *NodePool::Make<TypeNoBoundsNode>(const std::vector<Token> & , uint32_t &, const uint32_t &);