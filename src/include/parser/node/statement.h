#pragma once

#include "parser/class_declaration.h"
#include "lexer/lexer.h"
#include "parser/node/AST_node.h"
#include <cstdint>

class LetStatementNode : public ASTNode {
  friend class Printer;
public:
  LetStatementNode() = delete;
  LetStatementNode(const std::vector<Token> &, uint32_t &, const uint32_t &);
private:
  PatternNoTopAltNode *pattern_no_top_alt_ = nullptr;
  TypeNode *type_ = nullptr;
  ExpressionNode *expr_ = nullptr;
};

class ExpressionStatementNode : public ASTNode {
  friend class Printer;
public:
  ExpressionStatementNode() = delete;
  ExpressionStatementNode(const std::vector<Token> &, uint32_t &, const uint32_t &);
private:
  ExpressionNode *expr_;
  bool semicolon_ = false;
};

class StatementNode : public ASTNode {
  friend class Printer;
public:
  StatementNode() = delete;
  StatementNode(const std::vector<Token> &, uint32_t &, const uint32_t &);
private:
  bool semicolon_ = false;
  ItemNode *item_ = nullptr;
  LetStatementNode *let_statement_ = nullptr;
  ExpressionStatementNode *expr_statement_ = nullptr;
};

class StatementsNode : public ASTNode {
  friend class Printer;
public:
  StatementsNode() = delete;
  StatementsNode(const std::vector<Token> &, uint32_t &, const uint32_t &);
private:
  std::vector<StatementNode *> statement_s_;
  ExpressionWithoutBlockNode *expr_without_block_ = nullptr;
};
