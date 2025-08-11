#include "parser/node/trait.h"

TraitNode::TraitNode(const std::vector<Token> &tokens, uint32_t &pos, const uint32_t &length) : ASTNode("Trait") {
  try {
    CheckLength(pos, length);
    if (tokens[pos].lexeme != "trait") {
      throw Error("try parsing Trait Node but the first token is not trait");
    }
    ++pos;
    identifier_ = node_pool.Make<IdentifierNode>(tokens, pos, length);
    CheckLength(pos, length);
    if (tokens[pos].lexeme == ":") {
      colon_ = true;
      ++pos;
      CheckLength(pos, length);
      if (tokens[pos].lexeme != "where" && tokens[pos].lexeme != "{") {
        type_param_bounds_ = node_pool.Make<TypeParamBoundsNode>(tokens, pos, length);
        CheckLength(pos, length);
      }
    }
    if (tokens[pos].lexeme != "{") {
      throw Error("try parsing Trait Node but no {");
    }
    ++pos;
    while (pos < length && tokens[pos].lexeme != "}") {
      asscociated_items_.push_back(node_pool.Make<AsscociatedItemNode>(tokens, pos, length));
    }
    if (pos >= length || tokens[pos].lexeme != "}") {
      throw Error("try parsing Trait Node but no }");
    }
    ++pos;
  } catch (Error &err) {
    throw err;
  }
}
