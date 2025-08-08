#ifndef TERMINAL_H
#define TERMINAL_H

#include "parser/AST_node.h"

class IdentifierOrKeywordNode : public ASTNode {

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

class ByteLiteralNode : public ASTNode {

private:
  int8_t val_;
};

class ByteStringLiteralNode : public ASTNode {

private:
  std::basic_string<int8_t> val_;
};

class RawByteStringLiteralNode : public ASTNode {

private:
  std::basic_string<int8_t> val_;
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

#endif //TERMINAL_H
