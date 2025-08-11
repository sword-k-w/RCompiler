#include "parser/node/terminal.h"

IdentifierNode::IdentifierNode(const std::vector<Token> &tokens, uint32_t &pos, const uint32_t &length) : ASTNode("Identifier") {
  CheckLength(pos, length);
  if (tokens[pos].type != kIDENTIFIER_OR_KEYWORD || IsKeyword(tokens[pos].lexeme)) {
    Error("expect identifier");
  }
  val_ = tokens[pos].lexeme;
  ++pos;
}
