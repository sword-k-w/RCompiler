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
private:
  char val_;
};

class StringLiteralNode : public ASTNode {

private:
  std::string val_;
};

class RawStringLiteralNode : public ASTNode {

private:
  std::string val_;
};

class CStringLiteralNode : public ASTNode {

private:
  std::string val_;
};

class RawCStringLiteralNode : public ASTNode {

private:
  std::string val_;
};

class IntegerLiteralNode : public ASTNode {

};

class FloatLiteralNode : public ASTNode {

};

class TrueNode : public ASTNode {

private:
  const bool val_ = true;
};

class FalseNode : public ASTNode {

private:
  const bool val_ = false;
};

class NeverTypeNode : public ASTNode {
public:
  NeverTypeNode() = delete;
  NeverTypeNode(const std::vector<Token>&, uint32_t&, const uint32_t&);
private:
  const std::string_view val_ = "!";
};

class InferredTypeNode : public ASTNode {
public:
  InferredTypeNode() = delete;
  InferredTypeNode(const std::vector<Token>&, uint32_t&, const uint32_t&);
private:
  const std::string_view val_ = "_";
};

#endif //TERMINAL_H
