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

CrateValNode::CrateValNode(const std::vector<Token> &tokens, uint32_t &pos, const uint32_t &length) : ASTNode("Crate Val") {
  try {
    CheckLength(pos, length);
    if (tokens[pos].lexeme != "crate") {
      throw Error("expect crate");
    }
    ++pos;
  } catch (Error &err) {
    throw err;
  }
}

NeverTypeNode::NeverTypeNode(const std::vector<Token> &tokens, uint32_t &pos, const uint32_t &length) : ASTNode("Never Type") {
  try {
    CheckLength(pos, length);
    if (tokens[pos].lexeme != "!") {
      throw Error("expect !");
    }
    ++pos;
  } catch (Error &err) {
    throw err;
  }
}

InferredTypeNode::InferredTypeNode(const std::vector<Token> &tokens, uint32_t &pos, const uint32_t &length) : ASTNode("Inferred Type") {
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

