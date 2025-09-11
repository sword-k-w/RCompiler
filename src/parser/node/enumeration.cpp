#include "parser/node/enumeration.h"
#include "parser/node/terminal.h"
#include "common/error.h"

EnumVariantsNode::EnumVariantsNode(const std::vector<Token> &tokens, uint32_t &pos, const uint32_t &length) : ASTNode("Enum Variants") {
  try {
    enum_variant_s_.push_back(std::make_shared<EnumVariantNode>(tokens, pos, length));
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
      enum_variant_s_.push_back(std::make_shared<EnumVariantNode>(tokens, pos, length));
    }
    if (pos >= length) {
      throw Error("try parsing Enum Variants Node but no }");
    }
  } catch (Error &) { throw; }
}

EnumerationNode::EnumerationNode(const std::vector<Token> &tokens, uint32_t &pos, const uint32_t &length) : ASTNode("Enumeration") {
  try {
    CheckLength(pos, length);
    if (tokens[pos].lexeme != "enum") {
      throw Error("try parsing Enumeration Node but the first token is not enum");
    }
    ++pos;
    identifier_ = std::make_shared<IdentifierNode>(tokens, pos, length);
    CheckLength(pos, length);
    if (tokens[pos].lexeme != "{") {
      throw Error("try parsing Enumeration Node but no {");
    }
    ++pos;
    enum_variants_ = std::make_shared<EnumVariantsNode>(tokens, pos, length);
    CheckLength(pos, length);
    if (tokens[pos].lexeme != "}") {
      throw Error("try parsing Enumeration Node but no }");
    }
    ++pos;
  } catch (Error &) { throw; }
}