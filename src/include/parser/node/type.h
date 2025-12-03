#pragma once

#include "parser/class_declaration.h"
#include "lexer/lexer.h"
#include "parser/node/AST_node.h"
#include "semantic/type/type.h"
#include <cstdint>
#include <memory>

class ReferenceTypeNode : public ASTNode {
  friend class Printer;
  friend class FirstChecker;
  friend class SecondChecker;
  friend class ThirdChecker;
  friend class IRGenerator;
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
  friend class FirstChecker;
  friend class SecondChecker;
  friend class ThirdChecker;
  friend class IRGenerator;
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
  friend class FirstChecker;
  friend class SecondChecker;
  friend class ThirdChecker;
  friend class IRGenerator;
  friend std::pair<uint32_t, bool> GetTypeSize(Type *);
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
