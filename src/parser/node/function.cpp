#include "parser/node/function.h"
#include "common/error.h"
#include "parser/node/terminal.h"
#include "parser/node/type.h"
#include "parser/node/pattern.h"
#include "parser/node/expression.h"

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
    self_lower_ = std::make_shared<SelfLowerNode>(tokens, pos, length);
  } catch (Error &) {
    throw;
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
    self_lower_ = std::make_shared<SelfLowerNode>(tokens, pos, length);
    CheckLength(pos, length);
    if (tokens[pos].lexeme != ":") {
      throw Error("try parsing Typed Self Node but no :");
    }
    ++pos;
    type_ = std::make_shared<TypeNode>(tokens, pos, length);
  } catch (Error &) {
    throw;
  }
}

SelfParamNode::SelfParamNode(const std::vector<Token> &tokens, uint32_t &pos, const uint32_t &length) : ASTNode("Self Param") {
  try {
    uint32_t tmp = pos;
    try {
      shorthand_self_ = std::make_shared<ShorthandSelfNode>(tokens, pos, length);
    } catch (...) {
      shorthand_self_ = nullptr;
      pos = tmp;
      typed_self_ = std::make_shared<TypedSelfNode>(tokens, pos, length);
    }
  } catch (Error &) {
    throw;
  }
}

FunctionParamNode::FunctionParamNode(const std::vector<Token> &tokens, uint32_t &pos, const uint32_t &length) : ASTNode("Function Param") {
  try {
    pattern_no_top_alt_ = std::make_shared<PatternNoTopAltNode>(tokens, pos, length);
    CheckLength(pos, length);
    if (tokens[pos].lexeme != ":") {
      throw Error("try parsing Function Param Node but no :");
    }
    ++pos;
    type_ = std::make_shared<TypeNode>(tokens, pos, length);
  } catch (Error &) {
    throw;
  }
}

FunctionParametersNode::FunctionParametersNode(const std::vector<Token> &tokens, uint32_t &pos, const uint32_t &length) : ASTNode("Function Parameters") {
  try {
    uint32_t tmp = pos;
    try {
      self_param_ = std::make_shared<SelfParamNode>(tokens, pos, length);
      CheckLength(pos, length);
      if (tokens[pos].lexeme == ",") {
        ++comma_cnt_;
        ++pos;
      }
    } catch (...) {
      self_param_ = nullptr;
      comma_cnt_ = 0;
      pos = tmp;
    }
    CheckLength(pos, length);
    if (tokens[pos].lexeme == ")") {
      return;
    }
    if (self_param_ != nullptr && !comma_cnt_) {
      throw Error("try parsing Function Parameters Node but no comma between Self Param and Function Param");
    }
    function_params_.push_back(std::make_shared<FunctionParamNode>(tokens, pos, length));
    while (pos < length && tokens[pos].lexeme != ")") {
      if (tokens[pos].lexeme != ",") {
        throw Error("try parsing Function Parameters Node but not ,");
      }
      ++comma_cnt_;
      ++pos;
      if (tokens[pos].lexeme == ")") {
        break;
      }
      function_params_.push_back(std::make_shared<FunctionParamNode>(tokens, pos, length));
    }
  } catch (Error &) {
    throw;
  }
}

FunctionReturnTypeNode::FunctionReturnTypeNode(const std::vector<Token> &tokens, uint32_t &pos, const uint32_t &length) : ASTNode("Function Return Type") {
  try {
    CheckLength(pos, length);
    if (tokens[pos].lexeme != "->") {
      throw Error("try parsing Function Return Type Node but no ->");
    }
    ++pos;
    type_ = std::make_shared<TypeNode>(tokens, pos, length);
  } catch (Error &) {
    throw;
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
    if (tokens[pos].lexeme != "fn") {
      throw Error("try parsing Function Node but the first token is not fn");
    }
    ++pos;
    identifier_ = std::make_shared<IdentifierNode>(tokens, pos, length);
    CheckLength(pos, length);
    if (tokens[pos].lexeme != "(") {
      throw Error("try parsing Function Node but not '('");
    }
    ++pos;
    CheckLength(pos, length);
    if (tokens[pos].lexeme != ")") {
      function_parameters_ = std::make_shared<FunctionParametersNode>(tokens, pos, length);
      if (pos >= length || tokens[pos].lexeme != ")") {
        throw Error("try parsing Function Node but not ')' after function parameters");
      }
    }
    ++pos;
    CheckLength(pos, length);
    if (tokens[pos].lexeme == "->") {
      function_return_type_ = std::make_shared<FunctionReturnTypeNode>(tokens, pos, length);
    }
    CheckLength(pos, length);
    if (tokens[pos].lexeme == ";") {
      ++pos;
      semicolon_ = true;
    } else {
      block_expr_ = std::make_shared<BlockExpressionNode>(tokens, pos, length);
    }
  } catch (Error &) {
    throw;
  }
}
