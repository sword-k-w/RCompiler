#pragma once

#include "parser/class_declaration.h"
#include "lexer/lexer.h"
#include "parser/node/AST_node.h"
#include <cstdint>

class LetStatementNode : public ASTNode {
  friend class Printer;
  friend class FirstChecker;
  friend class SecondChecker;
  friend class ThirdChecker;
public:
  LetStatementNode() = delete;
  LetStatementNode(const std::vector<Token> &, uint32_t &, const uint32_t &);
  void Accept(VisitorBase *) override;
private:
  std::shared_ptr<PatternNoTopAltNode> pattern_no_top_alt_;
  std::shared_ptr<TypeNode> type_;
  std::shared_ptr<ExpressionNode> expr_;
};

class ExpressionStatementNode : public ASTNode {
  friend class Printer;
  friend class FirstChecker;
  friend class SecondChecker;
  friend class ThirdChecker;
public:
  ExpressionStatementNode() = delete;
  ExpressionStatementNode(const std::vector<Token> &, uint32_t &, const uint32_t &);
  void Accept(VisitorBase *) override;
private:
  std::shared_ptr<ExpressionWithoutBlockNode> expr_without_block_;
  std::shared_ptr<ExpressionWithBlockNode> expr_with_block_;
  bool semicolon_ = false;
};

class StatementNode : public ASTNode {
  friend class Printer;
  friend class FirstChecker;
  friend class SecondChecker;
  friend class ThirdChecker;
public:
  StatementNode() = delete;
  StatementNode(const std::vector<Token> &, uint32_t &, const uint32_t &);
  void Accept(VisitorBase *) override;
private:
  bool semicolon_ = false;
  std::shared_ptr<ItemNode> item_;
  std::shared_ptr<LetStatementNode> let_statement_;
  std::shared_ptr<ExpressionStatementNode> expr_statement_;
};

class StatementsNode : public ASTNode {
  friend class Printer;
  friend class FirstChecker;
  friend class SecondChecker;
  friend class ThirdChecker;
public:
  StatementsNode() = delete;
  StatementsNode(const std::vector<Token> &, uint32_t &, const uint32_t &);
  void Accept(VisitorBase *) override;
private:
  std::vector<std::shared_ptr<StatementNode>> statement_s_;
  std::shared_ptr<ExpressionWithoutBlockNode> expr_without_block_;
};
