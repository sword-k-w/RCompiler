#include "parser/node/terminal.h"

IdentifierOrKeywordNode::IdentifierOrKeywordNode(const std::vector<Token> &tokens, uint32_t &pos, const uint32_t &length) :
  ASTNode("Identifier Or Keyword"), val_(tokens[pos].lexeme) {
  ++pos;
}
