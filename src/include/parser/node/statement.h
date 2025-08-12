#ifndef STATEMENT_H
#define STATEMENT_H

#include "lexer/lexer.h"
#include "parser/node/AST_node.h"
#include "parser/node/item.h"

class LetStatementNode : public ASTNode {
public:
  LetStatementNode() = delete;
  LetStatementNode(const std::vector<Token> &, uint32_t &, const uint32_t &);
private:
};

class ExpressionStatementNode : public ASTNode {
public:
  ExpressionStatementNode() = delete;
  ExpressionStatementNode(const std::vector<Token> &, uint32_t &, const uint32_t &);
private:
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

#endif //STATEMENT_H
