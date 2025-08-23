#include "parser/node/implementation.h"
#include "common/error.h"
#include "parser/node/type.h"
#include "parser/node/item.h"
#include "parser/node/terminal.h"

ImplementationNode::ImplementationNode(const std::vector<Token> &tokens, uint32_t &pos, const uint32_t &length) : ASTNode("Implementation") {
  try {
    CheckLength(pos, length);
    if (tokens[pos].lexeme != "impl") {
      throw Error("try parsing Implementation Node but the first token is not impl");
    }
    ++pos;
    uint32_t tmp = pos;
    try {
      type_ = std::make_shared<TypeNode>(tokens, pos, length);
      CheckLength(pos, length);
      if (tokens[pos].lexeme != "{") {
        throw Error("try parsing Implementation Node but no {");
      }
      ++pos;
      while (pos < length && tokens[pos].lexeme != "}") {
        associated_items_.push_back(std::make_shared<AssociatedItemNode>(tokens, pos, length));
      }
      if (pos >= length || tokens[pos].lexeme != "}") {
        throw Error("try parsing Implementation Node but no }");
      }
      ++pos;
    } catch (...) {
      type_ = nullptr;
      pos = tmp;
      identifier_ = std::make_shared<IdentifierNode>(tokens, pos, length);
      CheckLength(pos, length);
      if (tokens[pos].lexeme != "for") {
        throw Error("try parsing Implementation Node but no for");
      }
      ++pos;
      type_ = std::make_shared<TypeNode>(tokens, pos, length);
      CheckLength(pos, length);
      if (tokens[pos].lexeme != "{") {
        throw Error("try parsing Implementation Node but no {");
      }
      ++pos;
      while (pos < length && tokens[pos].lexeme != "}") {
        associated_items_.push_back(std::make_shared<AssociatedItemNode>(tokens, pos, length));
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
