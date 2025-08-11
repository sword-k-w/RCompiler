#include "parser/node/trait.h"

TraitNode::TraitNode(const std::vector<Token> &tokens, uint32_t &pos, const uint32_t &length) : ASTNode("Trait") {
  CheckLength(pos, length);
  if (tokens[pos].lexeme != "trait") {
    Error("try parsing Trait Node but the first token is not trait");
  }
  ++pos;
  identifier_ = node_pool.Make<IdentifierNode>(tokens, pos, length);
  CheckLength(pos, length);
  if (tokens[pos].lexeme == "<") {
    generic_params_ = node_pool.Make<GenericParamsNode>(tokens, pos, length);
  }
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
  if (tokens[pos].lexeme == "where") {
    where_clause_ = node_pool.Make<WhereClauseNode>(tokens, pos, length);
    CheckLength(pos, length);
  }
  if (tokens[pos].lexeme != "{") {
    Error("try parsing Trait Node but no {");
  }
  ++pos;
  while (pos < length && tokens[pos].lexeme != "}") {
    asscociated_items_.push_back(node_pool.Make<AsscociatedItemNode>(tokens, pos, length));
  }
  if (pos >= length || tokens[pos].lexeme != "}") {
    Error("try parsing Trait Node but no }");
  }
  ++pos;
}
