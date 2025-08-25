#pragma once

#include "parser/class_declaration.h"
#include "lexer/lexer.h"
#include "parser/node/AST_node.h"
#include <cstdint>
#include <memory>

class ReferenceTypeNode : public ASTNode {
  friend class Printer;
public:
  ReferenceTypeNode() = delete;
  ReferenceTypeNode(const std::vector<Token>&, uint32_t&, const uint32_t &);
  void Accept(VisitorBase *) override;
private:
  bool mut_ = false;
  std::shared_ptr<TypeNoBoundsNode> type_no_bounds_;
};

class ArrayTypeNode : public ASTNode {
  friend class Printer;
public:
  ArrayTypeNode() = delete;
  ArrayTypeNode(const std::vector<Token>&, uint32_t&, const uint32_t &);
  void Accept(VisitorBase *) override;
private:
  std::shared_ptr<TypeNode> type_;
  std::shared_ptr<ExpressionNode> expr_;
};

class TypeNoBoundsNode : public ASTNode {
  friend class Printer;
public:
  TypeNoBoundsNode() = delete;
  TypeNoBoundsNode(const std::vector<Token> &, uint32_t &, const uint32_t &);
  void Accept(VisitorBase *) override;
private:
  std::shared_ptr<TypePathNode> type_path_;
  std::shared_ptr<ReferenceTypeNode> reference_type_;
  std::shared_ptr<ArrayTypeNode> array_type_;
  std::shared_ptr<UnitTypeNode> unit_type_;
};
