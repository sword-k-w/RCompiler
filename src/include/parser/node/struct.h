#pragma once

#include "parser/class_declaration.h"
#include "lexer/lexer.h"
#include "parser/node/AST_node.h"
#include <cstdint>

class StructFieldNode : public ASTNode {
  friend class Printer;
  friend class FirstChecker;
public:
  StructFieldNode() = delete;
  StructFieldNode(const std::vector<Token> &, uint32_t &, const uint32_t &);
  void Accept(VisitorBase *) override;
private:
  std::shared_ptr<IdentifierNode> identifier_;
  std::shared_ptr<TypeNode> type_;
};

class StructFieldsNode : public ASTNode {
  friend class Printer;
  friend class FirstChecker;
public:
  StructFieldsNode() = delete;
  StructFieldsNode(const std::vector<Token> &, uint32_t &, const uint32_t &);
  void Accept(VisitorBase *) override;
private:
  std::vector<std::shared_ptr<StructFieldNode>> struct_field_s_;
  uint32_t comma_cnt_ = 0;
};

class StructNode : public ASTNode {
  friend class Printer;
  friend class FirstChecker;
public:
  StructNode() = delete;
  StructNode(const std::vector<Token> &, uint32_t &, const uint32_t &);
  void Accept(VisitorBase *) override;
private:
  std::shared_ptr<Type> type_info_;
  std::shared_ptr<IdentifierNode> identifier_;
  std::shared_ptr<StructFieldsNode> struct_fields_;
  bool semicolon_ = false;
};
