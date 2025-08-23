#include "parser/node/pattern.h"
#include "common/error.h"
#include "parser/node/terminal.h"
#include "parser/node/expression.h"

LiteralPatternNode::LiteralPatternNode(const std::vector<Token> &tokens, uint32_t &pos, const uint32_t &length) : ASTNode("Literal Pattern") {
  try {
    CheckLength(pos, length);
    if (tokens[pos].lexeme ==  "-") {
      hyphen_ = true;
      ++pos;
    }
    literal_expr_ = std::make_shared<LiteralExpressionNode>(tokens, pos, length);
  } catch (Error &err) {
    throw err;
  }
}

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
    CheckLength(pos, length);
    if (tokens[pos].lexeme == "@") {
      ++pos;
      CheckLength(pos, length);
      pattern_no_top_alt_ = std::make_shared<PatternNoTopAltNode>(tokens, pos, length);
    }
  } catch (Error &err) {
    throw err;
  }
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
  } catch (Error &err) {
    throw err;
  }
}

TupleStructItemsNode::TupleStructItemsNode(const std::vector<Token> &tokens, uint32_t &pos, const uint32_t &length) : ASTNode("Tuple Struct Itmes") {
  try {
    patterns_.push_back(std::make_shared<PatternNode>(tokens, pos, length));
    while (pos < length && tokens[pos].lexeme != ")") {
      if (tokens[pos].lexeme != ",") {
        throw Error("try parsing Tuple Struct Items Node but not ,");
      }
      ++comma_cnt_;
      ++pos;
      if (tokens[pos].lexeme == ")") {
        break;
      }
      patterns_.push_back(std::make_shared<PatternNode>(tokens, pos, length));
    }
  } catch (Error &err) {
    throw err;
  }
}

TupleStructPatternNode::TupleStructPatternNode(const std::vector<Token> &tokens, uint32_t &pos, const uint32_t &length) : ASTNode("Tuple Struct Pattern") {
  try {
    path_in_expr_ = std::make_shared<PathInExpressionNode>(tokens, pos, length);
    CheckLength(pos, length);
    if (tokens[pos].lexeme != "(") {
      throw Error("try parsing Tuple Struct Pattern Node but no (");
    }
    ++pos;
    CheckLength(pos, length);
    if (tokens[pos].lexeme != ")") {
      tuple_struct_items_ = std::make_shared<TupleStructItemsNode>(tokens, pos, length);
      CheckLength(pos, length);
      if (tokens[pos].lexeme != ")") {
        throw Error("try parsing Tuple Struct Pattern Node but no }");
      }
    }
    ++pos;
  } catch (Error &err) {
    throw err;
  }
}

PatternWithoutRangeNode::PatternWithoutRangeNode(const std::vector<Token> &tokens, uint32_t &pos, const uint32_t &length) : ASTNode("Pattern Without Range") {
  try {
    CheckLength(pos, length);
    if (tokens[pos].lexeme == "_") {
      wildcard_pattern_ = std::make_shared<WildcardPatternNode>(tokens, pos, length);
    } else if (tokens[pos].lexeme == "&" || tokens[pos].lexeme == "&&") {
      reference_pattern_ = std::make_shared<ReferencePatternNode>(tokens, pos, length);
    } else {
      uint32_t tmp = pos;
      try {
        literal_pattern_ = std::make_shared<LiteralPatternNode>(tokens, pos, length);
      } catch (...) {
        literal_pattern_ = nullptr;
        pos = tmp;
        try {
          path_pattern_ = std::make_shared<PathPatternNode>(tokens, pos, length);
        } catch (...) {
          path_pattern_ = nullptr;
          pos = tmp;
          try {
            tuple_struct_pattern_ = std::make_shared<TupleStructPatternNode>(tokens, pos, length);
          } catch (...) {
            tuple_struct_pattern_ = nullptr;
            pos = tmp;
            identifier_pattern_ = std::make_shared<IdentifierPatternNode>(tokens, pos, length);
          }
        }
      }
    }
  } catch (Error &err) {
    throw err;
  }
}
