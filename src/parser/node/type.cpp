#include "parser/node/type.h"

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
    type_no_bounds_ = node_pool.Make<TypeNoBoundsNode>(tokens, pos, length);
  } catch (Error &err) {
    throw err;
  }
}

ArrayTypeNode::ArrayTypeNode(const std::vector<Token> &tokens, uint32_t &pos, const uint32_t &length) : ASTNode("Array Type") {
  try {
    CheckLength(pos, length);
    if (tokens[pos].lexeme != "[") {
      throw Error("try parsing Array Type Node but no [");
    }
    ++pos;
    type_ = node_pool.Make<TypeNode>(tokens, pos, length);
    CheckLength(pos, length);
    if (tokens[pos].lexeme != ";") {
      throw Error("try parsing Array Type Node but no ;");
    }
    ++pos;
    expr_ = node_pool.Make<ExpressionNode>(tokens, pos, length);
    CheckLength(pos, length);
    if (tokens[pos].lexeme != "]") {
      throw Error("try parsing Array Type Node but no ]");
    }
    ++pos;
  } catch (Error &err) {
    throw err;
  }
}

SliceTypeNode::SliceTypeNode(const std::vector<Token> &tokens, uint32_t &pos, const uint32_t &length) : ASTNode("Slice Type") {
  try {
    CheckLength(pos, length);
    if (tokens[pos].lexeme != "[") {
      throw Error("try parsing Slice Type Node but no [");
    }
    ++pos;
    type_ = node_pool.Make<TypeNode>(tokens, pos, length);
    CheckLength(pos, length);
    if (tokens[pos].lexeme != "]") {
      throw Error("try parsing Slice Type Node but no ]");
    }
    ++pos;
  } catch (Error &err) {
    throw err;
  }
}

TypeNoBoundsNode::TypeNoBoundsNode(const std::vector<Token> &tokens, uint32_t &pos, const uint32_t &length) : ASTNode("Type No Bounds") {
  try {
    CheckLength(pos, length);
    if (tokens[pos].lexeme == "&") {
      reference_type_ = node_pool.Make<ReferenceTypeNode>(tokens, pos, length);
    } else if (tokens[pos].lexeme == "[") {
      uint32_t tmp = pos;
      try {
        array_type_ = node_pool.Make<ArrayTypeNode>(tokens, pos, length);
      } catch (...) {
        array_type_ = nullptr;
        pos = tmp;
        slice_type_ = node_pool.Make<SliceTypeNode>(tokens, pos, length);
      }
    } else {
      type_path_ = node_pool.Make<TypePathNode>(tokens, pos, length);
    }
  } catch (Error &err) {
    throw err;
  }
}
