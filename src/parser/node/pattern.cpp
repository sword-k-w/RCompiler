#include "parser/node/pattern.h"

LiteralPatternNode::LiteralPatternNode(const std::vector<Token> &tokens, uint32_t &pos, const uint32_t &length) : ASTNode("Literal Pattern") {
  try {
    CheckLength(pos, length);
    if (tokens[pos].lexeme ==  "-") {
      hyphen_ = true;
      ++pos;
    }
    literal_expr_ = node_pool.Make<LiteralExpressionNode>(tokens, pos, length);
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
    identifier_ = node_pool.Make<IdentifierNode>(tokens, pos, length);
    CheckLength(pos, length);
    if (tokens[pos].lexeme == "@") {
      ++pos;
      CheckLength(pos, length);
      pattern_no_top_alt_ = node_pool.Make<PatternNoTopAltNode>(tokens, pos, length);
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
    pattern_without_range_ = node_pool.Make<PatternWithoutRangeNode>(tokens, pos, length);
  } catch (Error &err) {
    throw err;
  }
}

StructPatternFieldNode::StructPatternFieldNode(const std::vector<Token> &tokens, uint32_t &pos, const uint32_t &length) : ASTNode("Struct Pattern Field") {
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
    identifier_ = node_pool.Make<IdentifierNode>(tokens, pos, length);
    if (!ref_ && !mut_) {
      CheckLength(pos, length);
      if (tokens[pos].lexeme == ":") {
        colon_ = true;
        ++pos;
        pattern_ = node_pool.Make<PatternNode>(tokens, pos, length);
      }
    }
  } catch (Error &err) {
    throw err;
  }
}

StructPatternFieldsNode::StructPatternFieldsNode(const std::vector<Token> &tokens, uint32_t &pos, const uint32_t &length) : ASTNode("Struct Pattern Fields") {
  try {
    struct_pattern_field_s_.push_back(node_pool.Make<StructPatternFieldNode>(tokens, pos, length));
    while (pos + 1 < length && tokens[pos].lexeme == "," && tokens[pos + 1].lexeme != ".." && tokens[pos + 1].lexeme != "}") {
      ++pos;
      struct_pattern_field_s_.push_back(node_pool.Make<StructPatternFieldNode>(tokens, pos, length));
    }
  } catch (Error &err) {
    throw err;
  }
}

StructPatternElementsNode::StructPatternElementsNode(const std::vector<Token> &tokens, uint32_t &pos, const uint32_t &length) : ASTNode("Struct Pattern Elements"){
  try {
    CheckLength(pos, length);
    if (tokens[pos].lexeme == "..") {
      struct_pattern_et_cetera_ = node_pool.Make<StructPatternEtCeteraNode>(tokens, pos, length);
    } else {
      struct_pattern_fields_ = node_pool.Make<StructPatternFieldsNode>(tokens, pos, length);
      CheckLength(pos, length);
      if (tokens[pos].lexeme == ",") {
        comma_ = true;
        ++pos;
        CheckLength(pos, length);
        if (tokens[pos].lexeme == "..") {
          struct_pattern_et_cetera_ = node_pool.Make<StructPatternEtCeteraNode>(tokens, pos, length);
        }
      }
    }
  } catch (Error &err) {
    throw err;
  }
}

StructPatternNode::StructPatternNode(const std::vector<Token> &tokens, uint32_t &pos, const uint32_t &length) : ASTNode("Struct Pattern") {
  try {
    path_in_expr_ = node_pool.Make<PathInExpressionNode>(tokens, pos, length);
    CheckLength(pos, length);
    if (tokens[pos].lexeme != "{") {
      throw Error("try parsing Struct Pattern Node but no {");
    }
    ++pos;
    CheckLength(pos, length);
    if (tokens[pos].lexeme != "}") {
      struct_pattern_elements_ = node_pool.Make<StructPatternElementsNode>(tokens, pos, length);
      CheckLength(pos, length);
      if (tokens[pos].lexeme != "}") {
        throw Error("try parsing Struct Pattern Node but no }");
      }
    }
    ++pos;
  } catch (Error &err) {
    throw err;
  }
}

TuplePatternItemsNode::TuplePatternItemsNode(const std::vector<Token> &tokens, uint32_t &pos, const uint32_t &length) : ASTNode("Tuple Pattern Items") {
  try {
    CheckLength(pos, length);
    if (tokens[pos].lexeme == "::") {
      rest_pattern_ = node_pool.Make<RestPatternNode>(tokens, pos, length);
    } else {
      patterns_.push_back(node_pool.Make<PatternNode>(tokens, pos, length));
      CheckLength(pos, length);
      if (tokens[pos].lexeme != ",") {
        throw Error("try parsing Tuple Pattern Items Node but not ,");
      }
      ++comma_cnt_;
      ++pos;
      while (pos < length && tokens[pos].lexeme != ")") {
        patterns_.push_back(node_pool.Make<PatternNode>(tokens, pos, length));
        CheckLength(pos, length);
        if (tokens[pos].lexeme == ")") {
          break;
        }
        if (tokens[pos].lexeme != ",") {
          throw Error("try parsing Tuple Pattern Items Node but not ,");
        }
        ++comma_cnt_;
        ++pos;
      }
    }
  } catch (Error &err) {
    throw err;
  }
}

TuplePatternNode::TuplePatternNode(const std::vector<Token> &tokens, uint32_t &pos, const uint32_t &length) : ASTNode("Tuple Pattern"){
  try {
    CheckLength(pos, length);
    if (tokens[pos].lexeme != "(") {
      throw Error("try parsing Tuple Pattern Node but no (");
    }
    ++pos;
    CheckLength(pos, length);
    if (tokens[pos].lexeme != ")") {
      tuple_pattern_items_ = node_pool.Make<TuplePatternItemsNode>(tokens, pos, length);
      CheckLength(pos, length);
      if (tokens[pos].lexeme != ")") {
        throw Error("try parsing Tuple Pattern Node but no )");
      }
    }
    ++pos;
  } catch (Error &err) {
    throw err;
  }
}

GroupedPatternNode::GroupedPatternNode(const std::vector<Token> &tokens, uint32_t &pos, const uint32_t &length) : ASTNode("Grouped Pattern") {
  try {
    CheckLength(pos, length);
    if (tokens[pos].lexeme != "(") {
      throw Error("try parsing Grouped Pattern Node but no (");
    }
    ++pos;
    pattern_ = node_pool.Make<PatternNode>(tokens, pos, length);
    CheckLength(pos, length);
    if (tokens[pos].lexeme != ")") {
      throw Error("try parsing Grouped Pattern Node but no )");
    }
    ++pos;
  } catch (Error &err) {
    throw err;
  }
}

SlicePatternItemsNode::SlicePatternItemsNode(const std::vector<Token> &tokens, uint32_t &pos, const uint32_t &length) : ASTNode("Slice Pattern Items") {
  try {
    patterns_.push_back(node_pool.Make<PatternNode>(tokens, pos, length));
    while (pos < length && tokens[pos].lexeme != "]") {
      if (tokens[pos].lexeme != ",") {
        throw Error("try parsing Slice Pattern Items Node but not ,");
      }
      ++comma_cnt_;
      ++pos;
      if (tokens[pos].lexeme == "]") {
        break;
      }
      patterns_.push_back(node_pool.Make<PatternNode>(tokens, pos, length));
    }
  } catch (Error &err) {
    throw err;
  }
}

SlicePatternNode::SlicePatternNode(const std::vector<Token> &tokens, uint32_t &pos, const uint32_t &length) : ASTNode("Slice Pattern") {
  try {
    CheckLength(pos, length);
    if (tokens[pos].lexeme != "[") {
      throw Error("try parsing Slice Pattern Node but no [");
    }
    ++pos;
    slice_pattern_items_ = node_pool.Make<SlicePatternItemsNode>(tokens, pos, length);
    CheckLength(pos, length);
    if (tokens[pos].lexeme != "]") {
      throw Error("try parsing Slice Pattern Node but no ]");
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
      wildcard_pattern_ = node_pool.Make<WildcardPatternNode>(tokens, pos, length);
    } else if (tokens[pos].lexeme == "..") {
      rest_pattern_ = node_pool.Make<RestPatternNode>(tokens, pos, length);
    } else if (tokens[pos].lexeme == "&" || tokens[pos].lexeme == "&&") {
      reference_pattern_ = node_pool.Make<ReferencePatternNode>(tokens, pos, length);
    } else if (tokens[pos].lexeme == "[") {
      slice_pattern_ = node_pool.Make<SlicePatternNode>(tokens, pos, length);
    } else {
      uint32_t tmp = pos;
      try {
        literal_pattern_ = node_pool.Make<LiteralPatternNode>(tokens, pos, length);
      } catch (...) {
        literal_pattern_ = nullptr;
        pos = tmp;
        try {
          identifier_pattern_ = node_pool.Make<IdentifierPatternNode>(tokens, pos, length);
        } catch (...) {
          identifier_pattern_ = nullptr;
          pos = tmp;
          try {
            struct_pattern_ = node_pool.Make<StructPatternNode>(tokens, pos, length);
          } catch (...) {
            struct_pattern_ = nullptr;
            pos = tmp;
            try {
              tuple_pattern_ = node_pool.Make<TuplePatternNode>(tokens, pos, length);
            } catch (...) {
              tuple_pattern_ = nullptr;
              pos = tmp;
              try {
                grouped_pattern_ = node_pool.Make<GroupedPatternNode>(tokens, pos, length);
              } catch (...) {
                grouped_pattern_ = nullptr;
                pos = tmp;
                path_pattern_ = node_pool.Make<PathPatternNode>(tokens, pos, length);
              }
            }
          }
        }
      }
    }
  } catch (Error &err) {
    throw err;
  }
}

PatternNoTopAltNode::PatternNoTopAltNode(const std::vector<Token> &tokens, uint32_t &pos, const uint32_t &length) : ASTNode("Pattern No Top Alt") {
  try {
    uint32_t tmp = pos;
    try {
      pattern_without_range_ = node_pool.Make<PatternWithoutRangeNode>(tokens, pos, length);
    } catch (...) {
      pattern_without_range_ = nullptr;
      pos = tmp;
      range_pattern_ = node_pool.Make<RangePatternNode>(tokens, pos, length);
    }
  } catch (Error &err) {
    throw err;
  }
}

PatternNode::PatternNode(const std::vector<Token> &tokens, uint32_t &pos, const uint32_t &length) : ASTNode("Pattern") {
  try {
    CheckLength(pos, length);
    if (tokens[pos].lexeme == "|") {
      ++or_cnt_;
      ++pos;
      CheckLength(pos, length);
    }
    pattern_no_top_alts_.push_back(node_pool.Make<PatternNoTopAltNode>(tokens, pos, length));
    while (pos < length && tokens[pos].lexeme == "|") {
      ++or_cnt_;
      ++pos;
      pattern_no_top_alts_.push_back(node_pool.Make<PatternNoTopAltNode>(tokens, pos, length));
    }
  } catch (Error &err) {
    throw err;
  }
}
