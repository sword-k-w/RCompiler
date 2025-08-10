#include "parser/node/module.h"

ModuleNode::ModuleNode(const std::vector<Token> &tokens, uint32_t &pos, const uint32_t &length) : ASTNode("Module") {
  CheckLength(pos, length);
  if (tokens[pos].lexeme != "mod") {
    Error("try parsing Module Node but the first token is not mod");
  }
  ++pos;
  CheckLength(pos, length);
  if (tokens[pos].type != kIDENTIFIER_OR_KEYWORD || IsKeyword(tokens[pos].lexeme)) {
    Error("try parsing Module Node but not identifier");
  }
  identifier_or_keyword_ = node_pool.Make<IdentifierOrKeywordNode>(tokens, pos, length);
  CheckLength(pos, length);
  if (tokens[pos].lexeme == ";") {
    ++pos;
    semicolon_ = true;
  } else if (tokens[pos].lexeme == "{") {
    ++pos;
    while (pos < length && tokens[pos].lexeme != "}") {
      items_.push_back(node_pool.Make<ItemNode>(tokens, pos, length));
    }
    if (pos >= length) {
      Error("try parsing Module Node but not get }");
    }
    ++pos;
  } else {
    Error("try parsing Module Node but not get ; or {");
  }
}
