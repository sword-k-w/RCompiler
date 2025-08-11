#include "parser/node/generic.h"

TypeParamNode::TypeParamNode(const std::vector<Token> &tokens, uint32_t &pos, const uint32_t &length) : ASTNode("Type Param") {
  identifier_ = node_pool.Make<IdentifierNode>(tokens, pos, length);
  CheckLength(pos, length);
  if (tokens[pos].lexeme == ":") {
    colon_ = true;
    ++pos;
    CheckLength(pos, length);
    if (tokens[pos].lexeme != "=" && tokens[pos].lexeme != "," && tokens[pos].lexeme != ">") {
      type_param_bounds_ = node_pool.Make<TypeParamBoundsNode>(tokens, pos, length);
      CheckLength(pos, length);
    }
  }
  if (tokens[pos].lexeme == "=") {
    ++pos;
    type_ = node_pool.Make<TypeNode>(tokens, pos, length);
  }
}

ConstParamNode::ConstParamNode(const std::vector<Token> &tokens, uint32_t &pos, const uint32_t &length) : ASTNode("Const Param") {
  CheckLength(pos, length);
  if (tokens[pos].lexeme != "const") {
    Error("try parsing Const Param Node but not const");
  }
  ++pos;
  identifier1_ = node_pool.Make<IdentifierNode>(tokens, pos, length);
  CheckLength(pos, length);
  if (tokens[pos].lexeme != ":") {
    Error("try parsing Const Param Node but no :");
  }
  ++pos;
  type_ = node_pool.Make<TypeNode>(tokens, pos, length);
  CheckLength(pos, length);
  if (tokens[pos].lexeme == "=") {
    ++pos;
    block_expr_ = node_pool.Make<BlockExpressionNode>(tokens, pos, length);
  } else if (tokens[pos].type == kIDENTIFIER_OR_KEYWORD) {
    identifier2_ = node_pool.Make<IdentifierNode>(tokens, pos, length);
  } else {
    if (tokens[pos].lexeme == "-") {
      hyphen_ = true;
      ++pos;
    }
    litral_expr_ = node_pool.Make<LiteralExpressionNode>(tokens, pos, length);
  }
}

GenericParamNode::GenericParamNode(const std::vector<Token> &tokens, uint32_t &pos, const uint32_t &length) : ASTNode("Generic Param") {
  CheckLength(pos, length);
  if (tokens[pos].lexeme == "const") {
    const_param_ = node_pool.Make<ConstParamNode>(tokens, pos, length);
  } else {
    type_param_ = node_pool.Make<TypeParamNode>(tokens, pos, length);
  }
}

GenericParamsNode::GenericParamsNode(const std::vector<Token> &tokens, uint32_t &pos, const uint32_t &length) : ASTNode("Generic Params") {
  CheckLength(pos, length);
  if (tokens[pos].lexeme != "<") {
    ++pos;
  }
  while (pos < length && tokens[pos].lexeme != ">") {
    generic_param_s_.push_back(node_pool.Make<GenericParamNode>(tokens, pos, length));
    CheckLength(pos, length);
    if (tokens[pos].lexeme == ">") {
      break;
    }
    if (tokens[pos].lexeme == ",") {
      ++comma_cnt_;
      ++pos;
    } else {
      Error("try parsing Generic Params Node but not ,");
    }
  }
  if (pos >= length || tokens[pos].lexeme != ">") {
    Error("try parsing Generic Params Node but no >");
  }
  ++pos;
}
