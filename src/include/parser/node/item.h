#pragma once

#include "parser/class_declaration.h"
#include "lexer/lexer.h"
#include "parser/node/AST_node.h"
#include <cstdint>
#include <memory>

class ConstantItemNode : public ASTNode {
  friend class Printer;
  friend class FirstChecker;
  friend class SecondChecker;
  friend class ThirdChecker;
public:
  ConstantItemNode() = delete;
  ConstantItemNode(const std::vector<Token> &, uint32_t &, const uint32_t &);
  void Accept(VisitorBase *) override;
private:
  std::shared_ptr<IdentifierNode> identifier_;
  std::shared_ptr<TypeNode> type_;
  std::shared_ptr<ExpressionNode> expr_;

  bool in_trait_ = false;
  bool in_implement_ = false;
};

class AssociatedItemNode : public ASTNode {
  friend class Printer;
  friend class FirstChecker;
  friend class SecondChecker;
  friend class ThirdChecker;
public:
  AssociatedItemNode() = delete;
  AssociatedItemNode(const std::vector<Token> &, uint32_t &, const uint32_t &);
  void Accept(VisitorBase *) override;
private:
  std::shared_ptr<ConstantItemNode> constant_item_;
  std::shared_ptr<FunctionNode> function_;

  bool in_trait_ = false;
  bool in_implement_ = false;
};

class ItemNode : public ASTNode {
  friend class Printer;
  friend class FirstChecker;
  friend class SecondChecker;
  friend class ThirdChecker;
public:
  ItemNode() = delete;
  ItemNode(const std::vector<Token> &, uint32_t &, const uint32_t &);
  void Accept(VisitorBase *) override;
private:
  std::shared_ptr<FunctionNode> function_;
  std::shared_ptr<StructNode> struct_;
  std::shared_ptr<EnumerationNode> enumeration_;
  std::shared_ptr<ConstantItemNode> constant_item_;
  std::shared_ptr<TraitNode> trait_;
  std::shared_ptr<ImplementationNode> implementation_;
};
