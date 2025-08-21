#include "parser/node/terminal.h"

IdentifierNode::IdentifierNode(const std::vector<Token> &tokens, uint32_t &pos, const uint32_t &length) : ASTNode("Identifier") {
  try {
    CheckLength(pos, length);
    if (tokens[pos].type != kIDENTIFIER_OR_KEYWORD || IsKeyword(tokens[pos].lexeme)) {
      throw Error("expect identifier");
    }
    val_ = tokens[pos].lexeme;
    ++pos;
  } catch (Error &err) {
    throw err;
  }
}

CharLiteralNode::CharLiteralNode(const std::vector<Token> &tokens, uint32_t &pos, const uint32_t &length) : ASTNode("Char Literal") {
  try {
    CheckLength(pos, length);
    if (tokens[pos].type != kCHAR_LITERAL) {
      throw Error("expect char literal");
    }
    val_ = tokens[pos].lexeme;
    ++pos;
  } catch (Error &err) {
    throw err;
  }
}

StringLiteralNode::StringLiteralNode(const std::vector<Token> &tokens, uint32_t &pos, const uint32_t &length) : ASTNode("String Literal") {
  try {
    CheckLength(pos, length);
    if (tokens[pos].type != kSTRING_LITERAL) {
      throw Error("expect string literal");
    }
    val_ = tokens[pos].lexeme;
    ++pos;
  } catch (Error &err) {
    throw err;
  }
}

RawStringLiteralNode::RawStringLiteralNode(const std::vector<Token> &tokens, uint32_t &pos, const uint32_t &length) : ASTNode("Raw String Literal") {
  try {
    CheckLength(pos, length);
    if (tokens[pos].type != kRAW_STRING_LITERAL) {
      throw Error("expect raw string literal");
    }
    val_ = tokens[pos].lexeme;
    ++pos;
  } catch (Error &err) {
    throw err;
  }
}

CStringLiteralNode::CStringLiteralNode(const std::vector<Token> &tokens, uint32_t &pos, const uint32_t &length) : ASTNode("C String Literal") {
  try {
    CheckLength(pos, length);
    if (tokens[pos].type != kC_STRING_LITERAL) {
      throw Error("expect c string literal");
    }
    val_ = tokens[pos].lexeme;
    ++pos;
  } catch (Error &err) {
    throw err;
  }
}

RawCStringLiteralNode::RawCStringLiteralNode(const std::vector<Token> &tokens, uint32_t &pos, const uint32_t &length) : ASTNode("Raw C String Literal") {
  try {
    CheckLength(pos, length);
    if (tokens[pos].type != kRAW_C_STRING_LITERAL) {
      throw Error("expect raw c string literal");
    }
    val_ = tokens[pos].lexeme;
    ++pos;
  } catch (Error &err) {
    throw err;
  }
}

IntegerLiteralNode::IntegerLiteralNode(const std::vector<Token> &tokens, uint32_t &pos, const uint32_t &length) : ASTNode("Integer Literal") {
  try {
    CheckLength(pos, length);
    if (tokens[pos].type != kINTEGER_LITERAL) {
      throw Error("expect integer literal");
    }
    val_ = tokens[pos].lexeme;
    ++pos;
  } catch (Error &err) {
    throw err;
  }
}

TrueNode::TrueNode(const std::vector<Token> &tokens, uint32_t &pos, const uint32_t &length) : ASTNode("True") {
  try {
    CheckLength(pos, length);
    if (tokens[pos].lexeme != "true") {
      throw Error("expect true");
    }
    ++pos;
  } catch (Error &err) {
    throw err;
  }
}

FalseNode::FalseNode(const std::vector<Token> &tokens, uint32_t &pos, const uint32_t &length) : ASTNode("False") {
  try {
    CheckLength(pos, length);
    if (tokens[pos].lexeme != "false") {
      throw Error("expect false");
    }
    ++pos;
  } catch (Error &err) {
    throw err;
  }
}

SuperNode::SuperNode(const std::vector<Token> &tokens, uint32_t &pos, const uint32_t &length) : ASTNode("Super") {
  try {
    CheckLength(pos, length);
    if (tokens[pos].lexeme != "super") {
      throw Error("expect super");
    }
    ++pos;
  } catch (Error &err) {
    throw err;
  }
}

SelfLowerNode::SelfLowerNode(const std::vector<Token> &tokens, uint32_t &pos, const uint32_t &length) : ASTNode("Self Lower") {
  try {
    CheckLength(pos, length);
    if (tokens[pos].lexeme != "self") {
      throw Error("expect self");
    }
    ++pos;
  } catch (Error &err) {
    throw err;
  }
}

SelfUpperNode::SelfUpperNode(const std::vector<Token> &tokens, uint32_t &pos, const uint32_t &length) : ASTNode("Self Upper") {
  try {
    CheckLength(pos, length);
    if (tokens[pos].lexeme != "Self") {
      throw Error("expect Self");
    }
    ++pos;
  } catch (Error &err) {
    throw err;
  }
}

UnderscoreExpressionNode::UnderscoreExpressionNode(const std::vector<Token> &tokens, uint32_t &pos, const uint32_t &length) : ASTNode("Inferred Type") {
  try {
    CheckLength(pos, length);
    if (tokens[pos].lexeme != "_") {
      throw Error("expect _");
    }
    ++pos;
  } catch (Error &err) {
    throw err;
  }
}

ContinueExpressionNode::ContinueExpressionNode(const std::vector<Token> &tokens, uint32_t &pos, const uint32_t &length) : ASTNode("Continue Expression") {
  try {
    CheckLength(pos, length);
    if (tokens[pos].lexeme != "continue") {
      throw Error("expect continue");
    }
    ++pos;
  } catch (Error &err) {
    throw err;
  }
}

UnitTypeNode::UnitTypeNode(const std::vector<Token> &tokens, uint32_t &pos, const uint32_t &length) : ASTNode("Unit Type") {
  try {
    CheckLength(pos + 1, length);
    if (tokens[pos].lexeme != "(" || tokens[pos].lexeme != ")") {
      throw Error("expect ()");
    }
    ++pos;
    ++pos;
  } catch (Error &err) {
    throw err;
  }
}
