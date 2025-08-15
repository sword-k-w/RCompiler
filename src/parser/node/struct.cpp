#include "parser/node/struct.h"

StructFieldNode::StructFieldNode(const std::vector<Token> &tokens, uint32_t &pos, const uint32_t &length) : ASTNode("Struct Field") {
  try {
    identifier_ = node_pool.Make<IdentifierNode>(tokens, pos, length);
    CheckLength(pos, length);
    if (tokens[pos].lexeme != ":") {
      throw Error("try parsing Struct Field Node but no :");
    }
    ++pos;
    type_ = node_pool.Make<TypeNode>(tokens, pos, length);
  } catch (Error &err) {
    throw err;
  }
}

StructFieldsNode::StructFieldsNode(const std::vector<Token> &tokens, uint32_t &pos, const uint32_t &length) : ASTNode("Struct Fields") {
  try {
    struct_field_s_.push_back(node_pool.Make<StructFieldNode>(tokens, pos, length));
    while (pos < length && tokens[pos].lexeme != "}") {
      if (tokens[pos].lexeme != ",") {
        throw Error("try parsing Struct Fields Node but not comma");
      }
      ++pos;
      ++comma_cnt_;
      CheckLength(pos, length);
      if (tokens[pos].lexeme == "}") {
        break;
      }
      struct_field_s_.push_back(node_pool.Make<StructFieldNode>(tokens, pos, length));
    }
    if (pos >= length) {
      throw Error("try parsing Struct Fields Node but no }");
    }
  } catch (Error &err) {
    throw err;
  }
}

StructNode::StructNode(const std::vector<Token> &tokens, uint32_t &pos, const uint32_t &length) : ASTNode("Struct") {
  try {
    CheckLength(pos, length);
    if (tokens[pos].lexeme != "struct") {
      throw Error("try parsing Struct Node but the first token is not struct");
    }
    ++pos;
    identifier_ = node_pool.Make<IdentifierNode>(tokens, pos, length);
    CheckLength(pos, length);
    if (tokens[pos].lexeme == ";") {
      semicolon_ = true;
      ++pos;
    } else {
      if (tokens[pos].lexeme != "{") {
        throw Error("try parsing StructStruct Node but not get {");
      }
      ++pos;
      CheckLength(pos, length);
      if (tokens[pos].lexeme != "}") {
        struct_fields_ = node_pool.Make<StructFieldsNode>(tokens, pos, length);
        CheckLength(pos, length);
        if (tokens[pos].lexeme != "}") {
          throw Error("try parsing StructStruct Node but not get }");
        }
      }
      ++pos;
    }
  } catch (Error &err) {
    throw err;
  }
}