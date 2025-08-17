#ifndef STATEMENT_H
#define STATEMENT_H

#include "lexer/lexer.h"
#include "parser/node/AST_node.h"
#include "parser/node/item.h"
#include "parser/node/pattern.h"
#include "parser/node/type.h"
#include "parser/node/expression.h"

class LetStatementNode : public ASTNode {
public:
  LetStatementNode() = delete;
  LetStatementNode(const std::vector<Token> &, uint32_t &, const uint32_t &);
private:
  PatternNoTopAltNode *pattern_no_top_alt_ = nullptr;
  TypeNode *type_ = nullptr;
  ExpressionNode *expr_ = nullptr;
};

class ExpressionStatementNode : public ASTNode {
public:
  ExpressionStatementNode() = delete;
  ExpressionStatementNode(const std::vector<Token> &, uint32_t &, const uint32_t &);
private:
  ExpressionNode *expr_;
};

class StatementNode : public ASTNode {
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
public:
  StatementsNode() = delete;
  StatementsNode(const std::vector<Token> &, uint32_t &, const uint32_t &);
private:
  std::vector<StatementNode *> statement_s_;
  ExpressionWithoutBlockNode *expr_without_block_ = nullptr;
};

#endif //STATEMENT_H
