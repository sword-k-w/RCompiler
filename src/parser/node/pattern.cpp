#include "parser/node/pattern.h"
#include "common/error.h"
#include "parser/node/terminal.h"
#include "parser/node/expression.h"

IdentifierPatternNode::IdentifierPatternNode(const std::vector<Token> &tokens, uint32_t &pos, const uint32_t &length) : ASTNode("Identifier Pattern") {
  try {
    CheckLength(pos, length);
    if (tokens[pos].lexeme == "ref") {
      ref_ = true;
      ++pos;
      CheckLength(pos, length);
    }
    if (tokens[pos].lexeme == "mut") {
      mut_ = true;
      ++pos;
      CheckLength(pos, length);
    }
    identifier_ = std::make_shared<IdentifierNode>(tokens, pos, length);
  } catch (Error &) { throw; }
}

ReferencePatternNode::ReferencePatternNode(const std::vector<Token> &tokens, uint32_t &pos, const uint32_t &length) : ASTNode("Reference Pattern") {
  try {
    CheckLength(pos, length);
    if (tokens[pos].lexeme == "&") {
      single_ = true;
    } else if (tokens[pos].lexeme != "&&") {
      throw Error("try parsing Reference Pattern but no & or &&");
    }
    ++pos;
    CheckLength(pos, length);
    if (tokens[pos].lexeme == "mut") {
      mut_ = true;
    }
    pattern_without_range_ = std::make_shared<PatternWithoutRangeNode>(tokens, pos, length);
  } catch (Error &) { throw; }
}

PatternWithoutRangeNode::PatternWithoutRangeNode(const std::vector<Token> &tokens, uint32_t &pos, const uint32_t &length) : ASTNode("Pattern Without Range") {
  try {
    CheckLength(pos, length);
    if (tokens[pos].lexeme == "_") {
      wildcard_pattern_ = std::make_shared<WildcardPatternNode>(tokens, pos, length);
    } else if (tokens[pos].lexeme == "&" || tokens[pos].lexeme == "&&") {
      reference_pattern_ = std::make_shared<ReferencePatternNode>(tokens, pos, length);
    } else {
      identifier_pattern_ = std::make_shared<IdentifierPatternNode>(tokens, pos, length);
    }
  } catch (Error &) { throw; }
}
