#ifndef TERMINAL_H
#define TERMINAL_H

#include "lexer/lexer.h"
#include "parser/node/AST_node.h"

class IdentifierNode : public ASTNode {
public:
  IdentifierNode() = delete;
  IdentifierNode(const std::vector<Token> &, uint32_t &, const uint32_t &);
private:
  std::string val_;
};

class CharLiteralNode : public ASTNode {
public:
  CharLiteralNode() = delete;
  CharLiteralNode(const std::vector<Token> &, uint32_t &, const uint32_t &);
private:
  std::string val_;
};

class StringLiteralNode : public ASTNode {
public:
  StringLiteralNode() = delete;
  StringLiteralNode(const std::vector<Token> &, uint32_t &, const uint32_t &);
private:
  std::string val_;
};

class RawStringLiteralNode : public ASTNode {
public:
  RawStringLiteralNode() = delete;
  RawStringLiteralNode(const std::vector<Token> &, uint32_t &, const uint32_t &);
private:
  std::string val_;
};

class CStringLiteralNode : public ASTNode {
public:
  CStringLiteralNode() = delete;
  CStringLiteralNode(const std::vector<Token> &, uint32_t &, const uint32_t &);
private:
  std::string val_;
};

class RawCStringLiteralNode : public ASTNode {
public:
  RawCStringLiteralNode() = delete;
  RawCStringLiteralNode(const std::vector<Token> &, uint32_t &, const uint32_t &);
private:
  std::string val_;
};

class IntegerLiteralNode : public ASTNode {
public:
  IntegerLiteralNode() = delete;
  IntegerLiteralNode(const std::vector<Token> &, uint32_t &, const uint32_t &);
private:
  std::string val_;
};

class TrueNode : public ASTNode {
public:
  TrueNode() = delete;
  TrueNode(const std::vector<Token> &, uint32_t &, const uint32_t &);
private:
  const bool val_ = true;
};

class FalseNode : public ASTNode {
public:
  FalseNode() = delete;
  FalseNode(const std::vector<Token> &, uint32_t &, const uint32_t &);
private:
  const bool val_ = false;
};

class SuperNode : public ASTNode {
public:
  SuperNode() = delete;
  SuperNode(const std::vector<Token>&, uint32_t&, const uint32_t&);
private:
  const std::string_view val_ = "super";
};

class SelfLowerNode : public ASTNode {
public:
  SelfLowerNode() = delete;
  SelfLowerNode(const std::vector<Token>&, uint32_t&, const uint32_t&);
private:
  const std::string_view val_ = "self";
};

class SelfUpperNode : public ASTNode {
public:
  SelfUpperNode() = delete;
  SelfUpperNode(const std::vector<Token>&, uint32_t&, const uint32_t&);
private:
  const std::string_view val_ = "Self";
};

class CrateValNode : public ASTNode {
public:
  CrateValNode() = delete;
  CrateValNode(const std::vector<Token>&, uint32_t&, const uint32_t&);
private:
  const std::string_view val_ = "crate";
};

class InferredTypeNode : public ASTNode {
public:
  InferredTypeNode() = delete;
  InferredTypeNode(const std::vector<Token>&, uint32_t&, const uint32_t&);
private:
  const std::string_view val_ = "_";
};

class ContinueExpressionNode : public ASTNode {
public:
  ContinueExpressionNode() = delete;
  ContinueExpressionNode(const std::vector<Token>&, uint32_t&, const uint32_t&);
private:
  const std::string val_ = "continue";
};

#endif //TERMINAL_H
