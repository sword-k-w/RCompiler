#include "parser/node/struct.h"
#include "common/error.h"
#include "parser/node/terminal.h"
#include "parser/node/type.h"

StructFieldNode::StructFieldNode(const std::vector<Token> &tokens, uint32_t &pos, const uint32_t &length) : ASTNode("Struct Field") {
  try {
    identifier_ = std::make_shared<IdentifierNode>(tokens, pos, length);
    CheckLength(pos, length);
    if (tokens[pos].lexeme != ":") {
      throw Error("try parsing Struct Field Node but no :");
    }
    ++pos;
    type_ = std::make_shared<TypeNode>(tokens, pos, length);
  } catch (Error &) {
    throw;
  }
}

StructFieldsNode::StructFieldsNode(const std::vector<Token> &tokens, uint32_t &pos, const uint32_t &length) : ASTNode("Struct Fields") {
  try {
    struct_field_s_.push_back(std::make_shared<StructFieldNode>(tokens, pos, length));
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
      struct_field_s_.push_back(std::make_shared<StructFieldNode>(tokens, pos, length));
    }
    if (pos >= length) {
      throw Error("try parsing Struct Fields Node but no }");
    }
  } catch (Error &) {
    throw;
  }
}

StructNode::StructNode(const std::vector<Token> &tokens, uint32_t &pos, const uint32_t &length) : ASTNode("Struct") {
  try {
    CheckLength(pos, length);
    if (tokens[pos].lexeme != "struct") {
      throw Error("try parsing Struct Node but the first token is not struct");
    }
    ++pos;
    identifier_ = std::make_shared<IdentifierNode>(tokens, pos, length);
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
        struct_fields_ = std::make_shared<StructFieldsNode>(tokens, pos, length);
        CheckLength(pos, length);
        if (tokens[pos].lexeme != "}") {
          throw Error("try parsing StructStruct Node but not get }");
        }
      }
      ++pos;
    }
  } catch (Error &) {
    throw;
  }
}