#ifndef AST_NODE_H
#define AST_NODE_H

#include "parser/node_pool.hpp"

class FunctionNode;

class ExpressionNode;

class ItemNode;

class AsscociatedItemNode;

class TypeNoBoundsNode;

using TypeNode = TypeNoBoundsNode;

class GenericParamsNode;

class WhereClauseNode;

class GenericArgsNode;

class PatternWithoutRangeNode;

using PatternNoTopAltNode = PatternWithoutRangeNode;

using PatternNode = PatternNoTopAltNode;

class PatternWithoutRangeNode;

class PathInExpressionNode;

using PathExpressionNode = PathInExpressionNode;

using PathPatternNode = PathInExpressionNode;

class PathIdentSegmentNode;

using PathExprSegmentNode = PathIdentSegmentNode;

using TypePathSegmentNode = PathIdentSegmentNode;

using TypePathNode = TypePathSegmentNode;

class IdentifierNode;

using EnumVariantNode = IdentifierNode;

class InferredTypeNode;

using WildcardPatternNode = InferredTypeNode;

using UnderscoreExpressionNode = InferredTypeNode;

class BlockExpressionNode;

class StatementsNode;

class StructNode;

class TraitNode;

class ImplementationNode;

class EnumerationNode;

class LiteralExpressionNode;

class ExpressionWithoutBlockNode;

class ASTNode {
public:
  ASTNode() = delete;
  ASTNode(const std::string_view &);
  void CheckLength(const uint32_t &, const uint32_t &) const;
  virtual ~ASTNode() = 0;
  // virtual void accept(Visitor *) = 0;
private:
  const std::string name_;
};

#endif //AST_NODE_H
