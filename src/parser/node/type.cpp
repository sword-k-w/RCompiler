#include "parser/node/type.h"
#include "common/error.h"
#include "parser/node/expression.h"
#include "parser/node/path.h"
#include "parser/node/terminal.h"

ReferenceTypeNode::ReferenceTypeNode(const std::vector<Token> &tokens, uint32_t &pos, const uint32_t &length) : ASTNode("Reference Type") {
  try {
    CheckLength(pos, length);
    if (tokens[pos].lexeme != "&") {
      throw Error("try parsing Reference Type Node but no &");
    }
    ++pos;
    CheckLength(pos, length);
    if (tokens[pos].lexeme == "mut") {
      mut_ = true;
      ++pos;
    }
    type_no_bounds_ = std::make_shared<TypeNoBoundsNode>(tokens, pos, length);
  } catch (Error &) { throw; }
}

ArrayTypeNode::ArrayTypeNode(const std::vector<Token> &tokens, uint32_t &pos, const uint32_t &length) : ASTNode("Array Type") {
  try {
    CheckLength(pos, length);
    if (tokens[pos].lexeme != "[") {
      throw Error("try parsing Array Type Node but no [");
    }
    ++pos;
    type_ = std::make_shared<TypeNode>(tokens, pos, length);
    CheckLength(pos, length);
    if (tokens[pos].lexeme != ";") {
      throw Error("try parsing Array Type Node but no ;");
    }
    ++pos;
    expr_ = std::make_shared<ExpressionNode>(tokens, pos, length);
    CheckLength(pos, length);
    if (tokens[pos].lexeme != "]") {
      throw Error("try parsing Array Type Node but no ]");
    }
    ++pos;
  } catch (Error &) { throw; }
}

TypeNoBoundsNode::TypeNoBoundsNode(const std::vector<Token> &tokens, uint32_t &pos, const uint32_t &length) : ASTNode("Type No Bounds") {
  try {
    CheckLength(pos, length);
    if (tokens[pos].lexeme == "&") {
      reference_type_ = std::make_shared<ReferenceTypeNode>(tokens, pos, length);
    } else if (tokens[pos].lexeme == "[") {
      array_type_ = std::make_shared<ArrayTypeNode>(tokens, pos, length);
    } else if (tokens[pos].lexeme == "(") {
      unit_type_ = std::make_shared<UnitTypeNode>(tokens, pos, length);
    } else {
      type_path_ = std::make_shared<TypePathNode>(tokens, pos, length);
    }
  } catch (Error &) { throw; }
}
