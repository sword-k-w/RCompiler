#include "parser/node/implementation.h"

ImplementationNode::ImplementationNode(const std::vector<Token> &tokens, uint32_t &pos, const uint32_t &length) : ASTNode("Implementation") {
  try {
    CheckLength(pos, length);
    if (tokens[pos].lexeme != "impl") {
      throw Error("try parsing Implementation Node but the first token is not impl");
    }
    ++pos;
    uint32_t tmp = pos;
    try {
      type_ = node_pool.Make<TypeNode>(tokens, pos, length);
      CheckLength(pos, length);
      if (tokens[pos].lexeme != "{") {
        throw Error("try parsing Implementation Node but no {");
      }
      ++pos;
      while (pos < length && tokens[pos].lexeme != "}") {
        asscociated_items_.push_back(node_pool.Make<AsscociatedItemNode>(tokens, pos, length));
      }
      if (pos >= length || tokens[pos].lexeme != "}") {
        throw Error("try parsing Implementation Node but no }");
      }
      ++pos;
    } catch (...) {
      type_ = nullptr;
      pos = tmp;
      identifier_ = node_pool.Make<IdentifierNode>(tokens, pos, length);
      CheckLength(pos, length);
      if (tokens[pos].lexeme != "for") {
        throw Error("try parsing Implementation Node but no for");
      }
      ++pos;
      type_ = node_pool.Make<TypeNode>(tokens, pos, length);
      CheckLength(pos, length);
      if (tokens[pos].lexeme != "{") {
        throw Error("try parsing Implementation Node but no {");
      }
      ++pos;
      while (pos < length && tokens[pos].lexeme != "}") {
        asscociated_items_.push_back(node_pool.Make<AsscociatedItemNode>(tokens, pos, length));
      }
      if (pos >= length || tokens[pos].lexeme != "}") {
        throw Error("try parsing Implementation Node but no }");
      }
      ++pos;
    }
  } catch (Error &err) {
    throw err;
  }
}
