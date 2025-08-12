#include "parser/node/terminal.h"

IdentifierNode::IdentifierNode(const std::vector<Token> &tokens, uint32_t &pos, const uint32_t &length) : ASTNode("Identifier") {
  try {
    CheckLength(pos, length);
    if (tokens[pos].type != kIDENTIFIER_OR_KEYWORD || IsKeyword(tokens[pos].lexeme)) {
      throw Error("expect identifier");
    }
    val_ = tokens[pos].lexeme;
    ++pos;
  } catch (Error &err) {
    throw err;
  }
}

NeverTypeNode::NeverTypeNode(const std::vector<Token> &tokens, uint32_t &pos, const uint32_t &length) : ASTNode("Never Type") {
  try {
    CheckLength(pos, length);
    if (tokens[pos].lexeme != "!") {
      throw Error("expect !");
    }
    ++pos;
  } catch (Error &err) {
    throw err;
  }
}

InferredTypeNode::InferredTypeNode(const std::vector<Token> &tokens, uint32_t &pos, const uint32_t &length) : ASTNode("Inferred Type") {
  try {
    CheckLength(pos, length);
    if (tokens[pos].lexeme != "_") {
      throw Error("expect _");
    }
    ++pos;
  } catch (Error &err) {
    throw err;
  }
}

