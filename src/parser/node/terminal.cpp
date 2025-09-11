#include "parser/node/terminal.h"
#include "common/error.h"

IdentifierNode::IdentifierNode(const std::vector<Token> &tokens, uint32_t &pos, const uint32_t &length) : ASTNode("Identifier") {
  try {
    CheckLength(pos, length);
    if (tokens[pos].type != kIDENTIFIER_OR_KEYWORD || IsKeyword(tokens[pos].lexeme)) {
      throw Error("expect identifier");
    }
    val_ = std::make_shared<std::string>(tokens[pos].lexeme);
    ++pos;
  } catch (Error &) { throw; }
}

CharLiteralNode::CharLiteralNode(const std::vector<Token> &tokens, uint32_t &pos, const uint32_t &length) : ASTNode("Char Literal") {
  try {
    CheckLength(pos, length);
    if (tokens[pos].type != kCHAR_LITERAL) {
      throw Error("expect char literal");
    }
    val_ = std::make_shared<std::string>(tokens[pos].lexeme);
    ++pos;
  } catch (Error &) { throw; }
}

StringLiteralNode::StringLiteralNode(const std::vector<Token> &tokens, uint32_t &pos, const uint32_t &length) : ASTNode("String Literal") {
  try {
    CheckLength(pos, length);
    if (tokens[pos].type != kSTRING_LITERAL) {
      throw Error("expect string literal");
    }
    val_ = std::make_shared<std::string>(tokens[pos].lexeme);
    ++pos;
  } catch (Error &) { throw; }
}

RawStringLiteralNode::RawStringLiteralNode(const std::vector<Token> &tokens, uint32_t &pos, const uint32_t &length) : ASTNode("Raw String Literal") {
  try {
    CheckLength(pos, length);
    if (tokens[pos].type != kRAW_STRING_LITERAL) {
      throw Error("expect raw string literal");
    }
    val_ = std::make_shared<std::string>(tokens[pos].lexeme);
    ++pos;
  } catch (Error &) { throw; }
}

CStringLiteralNode::CStringLiteralNode(const std::vector<Token> &tokens, uint32_t &pos, const uint32_t &length) : ASTNode("C String Literal") {
  try {
    CheckLength(pos, length);
    if (tokens[pos].type != kC_STRING_LITERAL) {
      throw Error("expect c string literal");
    }
    val_ = std::make_shared<std::string>(tokens[pos].lexeme);
    ++pos;
  } catch (Error &) { throw; }
}

RawCStringLiteralNode::RawCStringLiteralNode(const std::vector<Token> &tokens, uint32_t &pos, const uint32_t &length) : ASTNode("Raw C String Literal") {
  try {
    CheckLength(pos, length);
    if (tokens[pos].type != kRAW_C_STRING_LITERAL) {
      throw Error("expect raw c string literal");
    }
    val_ = std::make_shared<std::string>(tokens[pos].lexeme);
    ++pos;
  } catch (Error &) { throw; }
}

IntegerLiteralNode::IntegerLiteralNode(const std::vector<Token> &tokens, uint32_t &pos, const uint32_t &length) : ASTNode("Integer Literal") {
  try {
    CheckLength(pos, length);
    if (tokens[pos].type != kINTEGER_LITERAL) {
      throw Error("expect integer literal");
    }
    val_ = std::make_shared<std::string>(tokens[pos].lexeme);
    ++pos;
  } catch (Error &) { throw; }
}

TrueNode::TrueNode(const std::vector<Token> &tokens, uint32_t &pos, const uint32_t &length) : ASTNode("True") {
  try {
    CheckLength(pos, length);
    if (tokens[pos].lexeme != "true") {
      throw Error("expect true");
    }
    ++pos;
  } catch (Error &) { throw; }
}

FalseNode::FalseNode(const std::vector<Token> &tokens, uint32_t &pos, const uint32_t &length) : ASTNode("False") {
  try {
    CheckLength(pos, length);
    if (tokens[pos].lexeme != "false") {
      throw Error("expect false");
    }
    ++pos;
  } catch (Error &) { throw; }
}

SelfLowerNode::SelfLowerNode(const std::vector<Token> &tokens, uint32_t &pos, const uint32_t &length) : ASTNode("Self Lower") {
  try {
    CheckLength(pos, length);
    if (tokens[pos].lexeme != "self") {
      throw Error("expect self");
    }
    val_ = std::make_shared<std::string>(tokens[pos].lexeme);
    ++pos;
  } catch (Error &) { throw; }
}

SelfUpperNode::SelfUpperNode(const std::vector<Token> &tokens, uint32_t &pos, const uint32_t &length) : ASTNode("Self Upper") {
  try {
    CheckLength(pos, length);
    if (tokens[pos].lexeme != "Self") {
      throw Error("expect Self");
    }
    val_ = std::make_shared<std::string>(tokens[pos].lexeme);
    ++pos;
  } catch (Error &) { throw; }
}

UnderscoreExpressionNode::UnderscoreExpressionNode(const std::vector<Token> &tokens, uint32_t &pos, const uint32_t &length) : ASTNode("Inferred Type") {
  try {
    CheckLength(pos, length);
    if (tokens[pos].lexeme != "_") {
      throw Error("expect _");
    }
    val_ = std::make_shared<std::string>(tokens[pos].lexeme);
    ++pos;
  } catch (Error &) { throw; }
}

ContinueExpressionNode::ContinueExpressionNode(const std::vector<Token> &tokens, uint32_t &pos, const uint32_t &length) : ASTNode("Continue Expression") {
  try {
    CheckLength(pos, length);
    if (tokens[pos].lexeme != "continue") {
      throw Error("expect continue");
    }
    val_ = std::make_shared<std::string>(tokens[pos].lexeme);
    ++pos;
  } catch (Error &) { throw; }
}

UnitTypeNode::UnitTypeNode(const std::vector<Token> &tokens, uint32_t &pos, const uint32_t &length) : ASTNode("Unit Type") {
  try {
    CheckLength(pos + 1, length);
    if (tokens[pos].lexeme != "(" || tokens[pos].lexeme != ")") {
      throw Error("expect ()");
    }
    ++pos;
    ++pos;
    val_ = std::make_shared<std::string>("()");
  } catch (Error &) { throw; }
}
