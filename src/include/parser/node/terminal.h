#pragma once

#include "parser/class_declaration.h"
#include "lexer/lexer.h"
#include "parser/node/AST_node.h"
#include <cstdint>
#include <memory>

class IdentifierNode : public ASTNode {
  friend class Printer;
  friend class FirstChecker;
  friend class SecondChecker;
  friend class ThirdChecker;
public:
  IdentifierNode() = delete;
  IdentifierNode(const std::vector<Token> &, uint32_t &, const uint32_t &);
  void Accept(VisitorBase *) override;
private:
  std::shared_ptr<std::string> val_;
};

class CharLiteralNode : public ASTNode {
  friend class Printer;
  friend class FirstChecker;
  friend class SecondChecker;
  friend class ThirdChecker;
public:
  CharLiteralNode() = delete;
  CharLiteralNode(const std::vector<Token> &, uint32_t &, const uint32_t &);
  void Accept(VisitorBase *) override;
private:
  std::shared_ptr<std::string> val_;
};

class StringLiteralNode : public ASTNode {
  friend class Printer;
  friend class FirstChecker;
  friend class SecondChecker;
  friend class ThirdChecker;
public:
  StringLiteralNode() = delete;
  StringLiteralNode(const std::vector<Token> &, uint32_t &, const uint32_t &);
  void Accept(VisitorBase *) override;
private:
  std::shared_ptr<std::string> val_;
};

class RawStringLiteralNode : public ASTNode {
  friend class Printer;
  friend class FirstChecker;
  friend class SecondChecker;
  friend class ThirdChecker;
public:
  RawStringLiteralNode() = delete;
  RawStringLiteralNode(const std::vector<Token> &, uint32_t &, const uint32_t &);
  void Accept(VisitorBase *) override;
private:
  std::shared_ptr<std::string> val_;
};

class CStringLiteralNode : public ASTNode {
  friend class Printer;
  friend class FirstChecker;
  friend class SecondChecker;
  friend class ThirdChecker;
public:
  CStringLiteralNode() = delete;
  CStringLiteralNode(const std::vector<Token> &, uint32_t &, const uint32_t &);
  void Accept(VisitorBase *) override;
private:
  std::shared_ptr<std::string> val_;
};

class RawCStringLiteralNode : public ASTNode {
  friend class Printer;
  friend class FirstChecker;
  friend class SecondChecker;
  friend class ThirdChecker;
public:
  RawCStringLiteralNode() = delete;
  RawCStringLiteralNode(const std::vector<Token> &, uint32_t &, const uint32_t &);
  void Accept(VisitorBase *) override;
private:
  std::shared_ptr<std::string> val_;
};

class IntegerLiteralNode : public ASTNode {
  friend class Printer;
  friend class FirstChecker;
  friend class SecondChecker;
  friend class ThirdChecker;
public:
  IntegerLiteralNode() = delete;
  IntegerLiteralNode(const std::vector<Token> &, uint32_t &, const uint32_t &);
  void Accept(VisitorBase *) override;
private:
  std::shared_ptr<std::string> val_;
};

class TrueNode : public ASTNode {
  friend class Printer;
  friend class FirstChecker;
  friend class SecondChecker;
  friend class ThirdChecker;
public:
  TrueNode() = delete;
  TrueNode(const std::vector<Token> &, uint32_t &, const uint32_t &);
  void Accept(VisitorBase *) override;
private:
  const bool val_ = true;
};

class FalseNode : public ASTNode {
  friend class Printer;
  friend class FirstChecker;
  friend class SecondChecker;
  friend class ThirdChecker;
public:
  FalseNode() = delete;
  FalseNode(const std::vector<Token> &, uint32_t &, const uint32_t &);
  void Accept(VisitorBase *) override;
private:
  const bool val_ = false;
};

class SelfLowerNode : public ASTNode {
  friend class Printer;
  friend class FirstChecker;
  friend class SecondChecker;
  friend class ThirdChecker;
public:
  SelfLowerNode() = delete;
  SelfLowerNode(const std::vector<Token>&, uint32_t&, const uint32_t &);
  void Accept(VisitorBase *) override;
private:
  std::shared_ptr<std::string> val_;
};

class SelfUpperNode : public ASTNode {
  friend class Printer;
  friend class FirstChecker;
  friend class SecondChecker;
  friend class ThirdChecker;
public:
  SelfUpperNode() = delete;
  SelfUpperNode(const std::vector<Token>&, uint32_t&, const uint32_t &);
  void Accept(VisitorBase *) override;
private:
  std::shared_ptr<std::string> val_;
};

class UnderscoreExpressionNode : public ASTNode {
  friend class Printer;
  friend class FirstChecker;
  friend class SecondChecker;
  friend class ThirdChecker;
public:
  UnderscoreExpressionNode() = delete;
  UnderscoreExpressionNode(const std::vector<Token>&, uint32_t&, const uint32_t &);
  void Accept(VisitorBase *) override;
private:
  std::shared_ptr<std::string> val_;
};

class ContinueExpressionNode : public ASTNode {
  friend class Printer;
  friend class FirstChecker;
  friend class SecondChecker;
  friend class ThirdChecker;
public:
  ContinueExpressionNode() = delete;
  ContinueExpressionNode(const std::vector<Token>&, uint32_t&, const uint32_t &);
  void Accept(VisitorBase *) override;
private:
  std::shared_ptr<std::string> val_;
};

class UnitTypeNode : public ASTNode {
  friend class Printer;
  friend class FirstChecker;
  friend class SecondChecker;
  friend class ThirdChecker;
public:
  UnitTypeNode() = delete;
  UnitTypeNode(const std::vector<Token>&, uint32_t&, const uint32_t &);
  void Accept(VisitorBase *) override;
private:
  std::shared_ptr<std::string> val_;
};
