#include "parser/node/trait.h"
#include "common/error.h"
#include "parser/node/terminal.h"
#include "parser/node/item.h"

TraitNode::TraitNode(const std::vector<Token> &tokens, uint32_t &pos, const uint32_t &length) : ASTNode("Trait") {
  try {
    CheckLength(pos, length);
    if (tokens[pos].lexeme != "trait") {
      throw Error("try parsing Trait Node but the first token is not trait");
    }
    ++pos;
    identifier_ = std::make_shared<IdentifierNode>(tokens, pos, length);
    CheckLength(pos, length);
    if (tokens[pos].lexeme != "{") {
      throw Error("try parsing Trait Node but no {");
    }
    ++pos;
    while (pos < length && tokens[pos].lexeme != "}") {
      asscociated_items_.push_back(std::make_shared<AssociatedItemNode>(tokens, pos, length));
    }
    if (pos >= length || tokens[pos].lexeme != "}") {
      throw Error("try parsing Trait Node but no }");
    }
    ++pos;
  } catch (Error &) {
    throw;
  }
}
