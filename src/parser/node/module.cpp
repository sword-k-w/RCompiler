#include "parser/node/module.h"

ModuleNode::ModuleNode(const std::vector<Token> &tokens, uint32_t &pos, const uint32_t &length) : ASTNode("Module") {
  try {
    CheckLength(pos, length);
    if (tokens[pos].lexeme != "mod") {
      throw Error("try parsing Module Node but the first token is not mod");
    }
    ++pos;
    identifier_ = node_pool.Make<IdentifierNode>(tokens, pos, length);
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
        throw Error("try parsing Module Node but not get }");
      }
      ++pos;
    } else {
      throw Error("try parsing Module Node but not get ; or {");
    }
  } catch (Error &err) {
    throw err;
  }
}
