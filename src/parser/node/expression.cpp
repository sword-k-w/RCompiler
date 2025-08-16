#include "parser/node/expression.h"

LiteralExpressionNode::LiteralExpressionNode(const std::vector<Token> &tokens, uint32_t &pos, const uint32_t &length) : ASTNode("Litearal Expression") {
  try {
    CheckLength(pos, length);
    if (tokens[pos].type == kCHAR_LITERAL) {
      char_literal_ = node_pool.Make<CharLiteralNode>(tokens, pos, length);
    } else if (tokens[pos].type == kSTRING_LITERAL) {
      string_literal_ = node_pool.Make<StringLiteralNode>(tokens, pos, length);
    } else if (tokens[pos].type == kRAW_STRING_LITERAL) {
      raw_string_literal_ = node_pool.Make<RawStringLiteralNode>(tokens, pos, length);
    } else if (tokens[pos].type == kC_STRING_LITERAL) {
      c_string_literal_ = node_pool.Make<CStringLiteralNode>(tokens, pos, length);
    } else if (tokens[pos].type == kRAW_C_STRING_LITERAL) {
      raw_c_string_literal_ = node_pool.Make<RawCStringLiteralNode>(tokens, pos, length);
    } else if (tokens[pos].type == kINTEGER_LITERAL) {
      integer_literal_ = node_pool.Make<IntegerLiteralNode>(tokens, pos, length);
    } else if (tokens[pos].lexeme == "true") {
      true_ = node_pool.Make<TrueNode>(tokens, pos, length);
    } else if (tokens[pos].lexeme == "false") {
      false_ = node_pool.Make<FalseNode>(tokens, pos, length);
    } else {
      throw Error("try parsing Literal Expression Node but unexpected token");
    }
  } catch (Error &err) {
    throw err;
  }
}

ArrayElementsNode::ArrayElementsNode(const std::vector<Token> &tokens, uint32_t &pos, const uint32_t &length) : ASTNode("Array Elements") {
  try {
    exprs_.push_back(node_pool.Make<ExpressionNode>(tokens, pos, length));
    CheckLength(pos, length);
    if (tokens[pos].lexeme == ";") {
      semicolon_ = true;
      ++pos;
      exprs_.push_back(node_pool.Make<ExpressionNode>(tokens, pos, length));
    } else {
      while (pos < length && tokens[pos].lexeme != "]") {
        if (tokens[pos].lexeme != ",") {
          throw Error("try parsing Array Elements Node but not ,");
        }
        ++pos;
        CheckLength(pos, length);
        if (tokens[pos].lexeme == "]") {
          break;
        }
        exprs_.push_back(node_pool.Make<ExpressionNode>(tokens, pos, length));
      }
    }
  } catch (Error &err) {
    throw err;
  }
}

ArrayExpressionNode::ArrayExpressionNode(const std::vector<Token> &tokens, uint32_t &pos, const uint32_t &length) : ASTNode("Array Expression") {
  try {
    CheckLength(pos, length);
    if (tokens[pos].lexeme != "[") {
      throw Error("try parsing Array Expression Node but no [");
    }
    ++pos;
    CheckLength(pos, length);
    if (tokens[pos].lexeme != "]") {
      array_elements_ = node_pool.Make<ArrayElementsNode>(tokens, pos, length);
      CheckLength(pos, length);
      if (tokens[pos].lexeme != "]") {
        throw Error("try parsing Array Expression Node but no ]");
      }
    }
    ++pos;
  } catch (Error &err) {
    throw err;
  }
}

PathInExpressionNode::PathInExpressionNode(const std::vector<Token> &tokens, uint32_t &pos, const uint32_t &length) : ASTNode("Path In Expression"){
  try {
    path_expr_segments_.push_back(node_pool.Make<PathExprSegmentNode>(tokens, pos, length));
    while (pos < length && tokens[pos].lexeme == "::") {
      ++pos;
      path_expr_segments_.push_back(node_pool.Make<PathExprSegmentNode>(tokens, pos, length));
    }
  } catch (Error &err) {
    throw err;
  }
}

StructExprFieldNode::StructExprFieldNode(const std::vector<Token> &tokens, uint32_t &pos, const uint32_t &length) : ASTNode("Struct Expr Field") {
  try {
    identifier_ = node_pool.Make<IdentifierNode>(tokens, pos, length);
    CheckLength(pos, length);
    if (tokens[pos].lexeme == ":") {
      ++pos;
      expr_ = node_pool.Make<ExpressionNode>(tokens, pos, length);
    }
  } catch (Error &err) {
    throw err;
  }
}

StructExprFieldsNode::StructExprFieldsNode(const std::vector<Token> &tokens, uint32_t &pos, const uint32_t &length) : ASTNode("Struct Expr Fields") {
  try {
    struct_expr_field_s_.push_back(node_pool.Make<StructExprFieldNode>(tokens, pos, length));
    while (pos < length && tokens[pos].lexeme != "}") {
      if (tokens[pos].lexeme != ",") {
        throw Error("try parsing Struct Expr Fields Node but not ,");
      }
      ++comma_cnt;
      ++pos;
      if (tokens[pos].lexeme == "}") {
        break;
      }
      struct_expr_field_s_.push_back(node_pool.Make<StructExprFieldNode>(tokens, pos, length));
    }
  } catch (Error &err) {
    throw err;
  }
}

StructExpressionNode::StructExpressionNode(const std::vector<Token> &tokens, uint32_t &pos, const uint32_t &length) : ASTNode("Struct Expression") {
  try {
    CheckLength(pos, length);
    if (tokens[pos].lexeme == "Self") {
      self_upper_ = node_pool.Make<SelfUpperNode>(tokens, pos, length);
    } else {
      identifier_ = node_pool.Make<IdentifierNode>(tokens, pos, length);
    }
    CheckLength(pos, length);
    if (tokens[pos].lexeme != "{") {
      throw Error("try parsing Struct Expression Node but no {");
    }
    ++pos;
    CheckLength(pos, length);
    if (tokens[pos].lexeme != "}") {
      struct_expr_fields_ = node_pool.Make<StructExprFieldsNode>(tokens, pos, length);
      CheckLength(pos, length);
      if (tokens[pos].lexeme != "}") {
        throw Error("try parsing Struct Expression Node but no }");
      }
    }
    ++pos;
  } catch (Error &err) {
    throw err;
  }
}

BreakExpressionNode::BreakExpressionNode(const std::vector<Token> &tokens, uint32_t &pos, const uint32_t &length) : ASTNode("Break Expression") {
  try {
    CheckLength(pos, length);
    if (tokens[pos].lexeme != "break") {
      throw Error("try parsing Break Expression Node but no break");
    }
    ++pos;
    CheckLength(pos, length);
    if (tokens[pos].lexeme != ";") {
      expr_ = node_pool.Make<ExpressionNode>(tokens, pos, length);
    }
  } catch (Error &err) {
    throw err;
  }
}

ReturnExpressionNode::ReturnExpressionNode(const std::vector<Token> &tokens, uint32_t &pos, const uint32_t &length) : ASTNode("Return Expression") {
  try {
    CheckLength(pos, length);
    if (tokens[pos].lexeme != "return") {
      throw Error("try parsing Return Expression but no return");
    }
    ++pos;
    CheckLength(pos, length);
    if (tokens[pos].lexeme != ";") {
      expr_ = node_pool.Make<ExpressionNode>(tokens, pos, length);
    }
  } catch (Error &err) {
    throw err;
  }
}

BlockExpressionNode::BlockExpressionNode(const std::vector<Token> &tokens, uint32_t &pos, const uint32_t &length) : ASTNode("Block Expression") {
  try {
    CheckLength(pos, length);
    if (tokens[pos].lexeme != "{") {
      throw Error("try parsing Block Expression Node but no {");
    }
    ++pos;
    CheckLength(pos, length);
    if (tokens[pos].lexeme != "}") {
      statements_s_ = node_pool.Make<StatementsNode>(tokens, pos, length);
      CheckLength(pos, length);
      if (tokens[pos].lexeme != "}") {
        throw Error("try parsing Block Expression Node but no }");
      }
    }
    ++pos;
  } catch (Error &err) {
    throw err;
  }
}

ConstBlockExpressionNode::ConstBlockExpressionNode(const std::vector<Token> &tokens, uint32_t &pos, const uint32_t &length) : ASTNode("Const Block Expression") {
  try {
    CheckLength(pos, length);
    if (tokens[pos].lexeme != "const") {
      throw Error("try parsing Const Block Expression Node but no const");
    }
    ++pos;
    block_expr_ = node_pool.Make<BlockExpressionNode>(tokens, pos, length);
  } catch (Error &err) {
    throw err;
  }
}

InfiniteLoopExpressionNode::InfiniteLoopExpressionNode(const std::vector<Token> &tokens, uint32_t &pos, const uint32_t &length) : ASTNode("Infinite Loop Expression") {
  try {
    CheckLength(pos, length);
    if (tokens[pos].lexeme != "loop") {
      throw Error("try parsing Loop Expression Node but no loop");
    }
    ++pos;
    block_expr_ = node_pool.Make<BlockExpressionNode>(tokens, pos, length);
  } catch (Error &err) {
    throw err;
  }
}

PredicateLoopExpressionNode::PredicateLoopExpressionNode(const std::vector<Token> &tokens, uint32_t &pos, const uint32_t &length) : ASTNode("Predicate Loop Expression") {
  try {
    CheckLength(pos, length);
    if (tokens[pos].lexeme != "while") {
      throw Error("try parsing Predicate Loop Expression Node but no while");
    }
    ++pos;
    conditions_ = node_pool.Make<ConditionsNode>(tokens, pos, length);
    block_expr = node_pool.Make<BlockExpressionNode>(tokens, pos, length);
  } catch (Error &err) {
    throw err;
  }
}

LoopExpressionNode::LoopExpressionNode(const std::vector<Token> &tokens, uint32_t &pos, const uint32_t &length) : ASTNode("Loop Expression") {
  try {
    CheckLength(pos, length);
    if (tokens[pos].lexeme == "loop") {
      infinite_loop_expr_ = node_pool.Make<InfiniteLoopExpressionNode>(tokens, pos, length);
    } else if (tokens[pos].lexeme == "while") {
      predicate_loop_expr_ = node_pool.Make<PredicateLoopExpressionNode>(tokens, pos, length);
    } else {
      throw Error("try parsing Loop Expression Node but unexpected token");
    }
  } catch (Error &err) {
    throw err;
  }
}

IfExpressionNode::IfExpressionNode(const std::vector<Token> &tokens, uint32_t &pos, const uint32_t &length) : ASTNode("If Expression") {
  try {
    CheckLength(pos, length);
    if (tokens[pos].lexeme != "if") {
      throw Error("try parsing If Expression Node but no if");
    }
    ++pos;
    conditions_ = node_pool.Make<ConditionsNode>(tokens, pos, length);
    block_expr1_ = node_pool.Make<BlockExpressionNode>(tokens, pos, length);
    CheckLength(pos, length);
    if (tokens[pos].lexeme == "else") {
      ++pos;
      CheckLength(pos, length);
      if (tokens[pos].lexeme == "{") {
        block_expr2_ = node_pool.Make<BlockExpressionNode>(tokens, pos, length);
      } else if (tokens[pos].lexeme == "if") {
        if_expr_ = node_pool.Make<IfExpressionNode>(tokens, pos, length);
      } else {
        throw Error("try parsing If Expression Node but unexpected token after else");
      }
    }
  } catch (Error &err) {
    throw err;
  }
}

MatchExpressionNode::MatchExpressionNode(const std::vector<Token> &tokens, uint32_t &pos, const uint32_t &length) : ASTNode("Match Expression") {
  try {
    CheckLength(pos, length);
    if (tokens[pos].lexeme != "match") {
      throw Error("try parsing Match Expression Node but no match");
    }
    ++pos;
    scrutinee_ = node_pool.Make<ScrutineeNode>(tokens, pos, length);
    CheckLength(pos, length);
    if (tokens[pos].lexeme != "{") {
      throw Error("try parsing Match Expression Node but no {");
    }
    ++pos;
    CheckLength(pos, length);
    if (tokens[pos].lexeme != "}") {
      match_arms_ = node_pool.Make<MatchArmsNode>(tokens, pos, length);
      if (tokens[pos].lexeme != "}") {
        throw Error("try parsing Match Expression Node but no }");
      }
    }
    ++pos;
  } catch (Error &err) {
    throw err;
  }
}
