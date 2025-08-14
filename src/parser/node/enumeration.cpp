#include "parser/node/enumeration.h"

EnumVariantsNode::EnumVariantsNode(const std::vector<Token> &tokens, uint32_t &pos, const uint32_t &length) : ASTNode("Enum Variants") {
  try {
    enum_variant_s_.push_back(node_pool.Make<EnumVariantNode>(tokens, pos, length));
    while (pos < length && tokens[pos].lexeme != "}") {
      if (tokens[pos].lexeme != ",") {
        throw Error("try parsing Enum Variants Node but not comma");
      }
      ++pos;
      ++comma_cnt_;
      CheckLength(pos, length);
      if (tokens[pos].lexeme == "}") {
        break;
      }
      enum_variant_s_.push_back(node_pool.Make<EnumVariantNode>(tokens, pos, length));
    }
    if (pos >= length) {
      throw Error("try parsing Enum Variants Node but no }");
    }
  } catch (Error &err) {
    throw err;
  }
}

EnumerationNode::EnumerationNode(const std::vector<Token> &tokens, uint32_t &pos, const uint32_t &length) : ASTNode("Enumeration") {
  try {
    CheckLength(pos, length);
    if (tokens[pos].lexeme != "enum") {
      throw Error("try parsing Enumeration Node but the first token is not enum");
    }
    ++pos;
    identifier_ = node_pool.Make<IdentifierNode>(tokens, pos, length);
    CheckLength(pos, length);
    if (tokens[pos].lexeme != "{") {
      throw Error("try parsing Enumeration Node but no {");
    }
    ++pos;
    enum_variants_ = node_pool.Make<EnumVariantsNode>(tokens, pos, length);
    CheckLength(pos, length);
    if (tokens[pos].lexeme != "}") {
      throw Error("try parsing Enumeration Node but no }");
    }
    ++pos;
  } catch (Error &err) {
    throw err;
  }
}