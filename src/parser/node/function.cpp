#include "parser/node/function.h"

ShorthandSelfNode::ShorthandSelfNode(const std::vector<Token> &tokens, uint32_t &pos, const uint32_t &length) : ASTNode("Shorthand Self") {
  try {
    CheckLength(pos, length);
    if (tokens[pos].lexeme == "&") {
      quote_ = true;
      ++pos;
      CheckLength(pos, length);
    }
    if (tokens[pos].lexeme == "mut") {
      mut_ = true;
      ++pos;
      CheckLength(pos, length);
    }
    self_lower_ = node_pool.Make<SelfLowerNode>(tokens, pos, length);
  } catch (Error &err) {
    throw err;
  }
}

TypedSelfNode::TypedSelfNode(const std::vector<Token> &tokens, uint32_t &pos, const uint32_t &length) : ASTNode("Typed Self") {
  try {
    CheckLength(pos, length);
    if (tokens[pos].lexeme == "mut") {
      mut_ = true;
      ++pos;
      CheckLength(pos, length);
    }
    self_lower_ = node_pool.Make<SelfLowerNode>(tokens, pos, length);
    CheckLength(pos, length);
    if (tokens[pos].lexeme != ":") {
      throw Error("try parsing Typed Self Node but no :");
    }
    ++pos;
    type_ = node_pool.Make<TypeNode>(tokens, pos, length);
  } catch (Error &err) {
    throw err;
  }
}

SelfParamNode::SelfParamNode(const std::vector<Token> &tokens, uint32_t &pos, const uint32_t &length) : ASTNode("Self Param") {
  try {
    uint32_t tmp = pos;
    try {
      shorthand_self_ = node_pool.Make<ShorthandSelfNode>(tokens, pos, length);
    } catch (...) {
      shorthand_self_ = nullptr;
      pos = tmp;
      typed_self_ = node_pool.Make<TypedSelfNode>(tokens, pos, length);
    }
  } catch (Error &err) {
    throw err;
  }
}

FunctionParamPatternNode::FunctionParamPatternNode(const std::vector<Token> &tokens, uint32_t &pos, const uint32_t &length) : ASTNode("Function Param Pattern") {
  try {
    pattern_no_top_alt_ = node_pool.Make<PatternNoTopAltNode>(tokens, pos, length);
    CheckLength(pos, length);
    if (tokens[pos].lexeme != ":") {
      throw Error("try parsing Function Param Pattern Node but no :");
    }
    ++pos;
    CheckLength(pos, length);
    if (tokens[pos].lexeme == "...") {
      ellipsis_ = true;
      ++pos;
    } else {
      type_ = node_pool.Make<TypeNode>(tokens, pos, length);
    }
  } catch (Error &err) {
    throw err;
  }
}

FunctionParamNode::FunctionParamNode(const std::vector<Token> &tokens, uint32_t &pos, const uint32_t &length) : ASTNode("Function Param") {
  try {
    CheckLength(pos, length);
    if (tokens[pos].lexeme == "...") {
      ellipsis_ = true;
      ++pos;
    } else {
      uint32_t tmp = pos;
      try {
        function_param_pattern_ = node_pool.Make<FunctionParamPatternNode>(tokens, pos, length);
      } catch (...) {
        function_param_pattern_ = nullptr;
        pos = tmp;
        type_ = node_pool.Make<TypeNode>(tokens, pos, length);
      }
    }
  } catch (Error &err) {
    throw err;
  }
}

FunctionParametersNode::FunctionParametersNode(const std::vector<Token> &tokens, uint32_t &pos, const uint32_t &length) : ASTNode("Function Parameters") {
  try {
    uint32_t tmp = pos;
    try {
      self_param_ = node_pool.Make<SelfParamNode>(tokens, pos, length);
      CheckLength(pos, length);
      if (tokens[pos].lexeme == ",") {
        ++comma_cnt_;
        ++pos;
      }
    } catch (...) {
      self_param_ = nullptr;
      comma_cnt_ = 0;
      pos = tmp;
      function_params_.push_back(node_pool.Make<FunctionParamNode>(tokens, pos, length));
      while (pos < length && tokens[pos].lexeme != ")") {
        if (tokens[pos].lexeme != ",") {
          throw Error("try parsing Function Parameters Node but not ,");
        }
        ++comma_cnt_;
        ++pos;
        if (tokens[pos].lexeme == ")") {
          break;
        }
        function_params_.push_back(node_pool.Make<FunctionParamNode>(tokens, pos, length));
      }
    }
  } catch (Error &err) {
    throw err;
  }
}

FunctionReturnTypeNode::FunctionReturnTypeNode(const std::vector<Token> &tokens, uint32_t &pos, const uint32_t &length) : ASTNode("Function Return Type") {
  try {
    CheckLength(pos, length);
    if (tokens[pos].lexeme != "->") {
      throw Error("try parsing Function Return Type Node but no ->");
    }
    ++pos;
    type_ = node_pool.Make<TypeNode>(tokens, pos, length);
  } catch (Error &err) {
    throw err;
  }
}

FunctionNode::FunctionNode(const std::vector<Token> &tokens, uint32_t &pos, const uint32_t &length) : ASTNode("Function") {
  try {
    CheckLength(pos, length);
    if (tokens[pos].lexeme == "const") {
      const_ = true;
      ++pos;
      CheckLength(pos, length);
    }
    CheckLength(pos, length);
    if (tokens[pos].lexeme != "fn") {
      throw Error("try parsing Function Node but the first token is not fn");
    }
    ++pos;
    identifier_ = node_pool.Make<IdentifierNode>(tokens, pos, length);
    CheckLength(pos, length);
    if (tokens[pos].lexeme != "(") {
      generic_params_ = node_pool.Make<GenericParamsNode>(tokens, pos, length);
      if (pos >= length || tokens[pos].lexeme != "(") {
        throw Error("try parsing Function Node but not '(' after generic params");
      }
    }
    ++pos;
    CheckLength(pos, length);
    if (tokens[pos].lexeme != ")") {
      function_parameters_ = node_pool.Make<FunctionParametersNode>(tokens, pos, length);
      if (pos >= length || tokens[pos].lexeme != ")") {
        throw Error("try parsing Function Node but not ')' after function parameters");
      }
    }
    ++pos;
    CheckLength(pos, length);
    if (tokens[pos].lexeme == "->") {
      function_return_type_ = node_pool.Make<FunctionReturnTypeNode>(tokens, pos, length);
    }
    CheckLength(pos, length);
    if (tokens[pos].lexeme == "where") {
      where_clause_ = node_pool.Make<WhereClauseNode>(tokens, pos, length);
    }
    CheckLength(pos, length);
    if (tokens[pos].lexeme == ";") {
      ++pos;
      semicolon_ = true;
    } else {
      block_expr_ = node_pool.Make<BlockExpressionNode>(tokens, pos, length);
    }
  } catch (Error &err) {
    throw err;
  }
}
