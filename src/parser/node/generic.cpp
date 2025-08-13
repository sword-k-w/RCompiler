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
    throw Error("try parsing Const Param Node but not const");
  }
  ++pos;
  identifier1_ = node_pool.Make<IdentifierNode>(tokens, pos, length);
  CheckLength(pos, length);
  if (tokens[pos].lexeme != ":") {
    throw Error("try parsing Const Param Node but no :");
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
      throw Error("try parsing Generic Params Node but not ,");
    }
  }
  if (pos >= length || tokens[pos].lexeme != ">") {
    throw Error("try parsing Generic Params Node but no >");
  }
  ++pos;
}

GenericArgsConstNode::GenericArgsConstNode(const std::vector<Token> &tokens, uint32_t &pos, const uint32_t &length) : ASTNode("Generic Args Const") {
  try {
    CheckLength(pos, length);
    if (tokens[pos].lexeme == "{") {
      block_expr_ = node_pool.Make<BlockExpressionNode>(tokens, pos, length);
    } else if (tokens[pos].lexeme == "-") {
      hyphen_ = true;
      ++pos;
      literal_expr = node_pool.Make<LiteralExpressionNode>(tokens, pos, length);
    } else {
      uint32_t tmp = pos;
      try {
        literal_expr = node_pool.Make<LiteralExpressionNode>(tokens, pos, length);
      } catch (...) {
        literal_expr = nullptr;
        pos = tmp;
        simple_path_segment_ = node_pool.Make<SimplePathSegmentNode>(tokens, pos, length);
      }
    }
  } catch (Error &err) {
    throw err;
  }
}

GenericArgsBindingNode::GenericArgsBindingNode(const std::vector<Token> &tokens, uint32_t &pos, const uint32_t &length) : ASTNode("Generic Args Binding") {
  try {
    identifier_ = node_pool.Make<IdentifierNode>(tokens, pos, length);
    CheckLength(pos, length);
    if (tokens[pos].lexeme == "<") {
      generic_args_ = node_pool.Make<GenericArgsNode>(tokens, pos, length);
      CheckLength(pos, length);
    }
    if (tokens[pos].lexeme != "=") {
      throw Error("try parsing Generic Args Binding Node but no =");
    }
    ++pos;
    type_ = node_pool.Make<TypeNode>(tokens, pos, length);
  } catch (Error &err) {
    throw err;
  }
}

GenericArgsBoundsNode::GenericArgsBoundsNode(const std::vector<Token> &tokens, uint32_t &pos, const uint32_t &length) : ASTNode("Generic Args Bounds") {
  try {
    identifier_ = node_pool.Make<IdentifierNode>(tokens, pos, length);
    CheckLength(pos, length);
    if (tokens[pos].lexeme == "<") {
      generic_args_ = node_pool.Make<GenericArgsNode>(tokens, pos, length);
      CheckLength(pos, length);
    }
    if (tokens[pos].lexeme != ":") {
      throw Error("try parsing Generic Args Bounds Node but no :");
    }
    ++pos;
    type_param_bounds_ = node_pool.Make<TypeParamBoundsNode>(tokens, pos, length);
  } catch (Error &err) {
    throw err;
  }
}

GenericArgNode::GenericArgNode(const std::vector<Token> &tokens, uint32_t &pos, const uint32_t &length) : ASTNode("Generic Arg") {
  try {
    uint32_t tmp = pos;
    try {
      type_ = node_pool.Make<TypeNode>(tokens, pos, length);
    } catch (...) {
      type_ = nullptr;
      pos = tmp;
      try {
        generic_args_const_ = node_pool.Make<GenericArgsConstNode>(tokens, pos, length);
      } catch (...) {
        generic_args_const_ = nullptr;
        pos = tmp;
        try {
          generic_args_binding_ = node_pool.Make<GenericArgsBindingNode>(tokens, pos, length);
        } catch (...) {
          generic_args_binding_ = nullptr;
          pos = tmp;
          generic_args_bounds_ = node_pool.Make<GenericArgsBoundsNode>(tokens, pos, length);
        }
      }
    }
  } catch (Error &err) {
    throw err;
  }
}

GenericArgsNode::GenericArgsNode(const std::vector<Token> &tokens, uint32_t &pos, const uint32_t &length) : ASTNode("Generic Args") {
  try {
    CheckLength(pos, length);
    if (tokens[pos].lexeme != "<") {
      throw Error("try parsing Generic Args Node but no <");
    }
    ++pos;
    while (pos < length && tokens[pos].lexeme != ">") {
      generic_arg_s_.push_back(node_pool.Make<GenericArgNode>(tokens, pos, length));
      if (tokens[pos].lexeme == ">") {
        break;
      }
      if (tokens[pos].lexeme != ",") {
        throw Error("try parsing Generic Args Node but not ,");
      }
      ++pos;
      ++comma_cnt_;
    }
    if (pos >= length || tokens[pos].lexeme != ">") {
      throw Error("try parsing Generic Args Node but no >");
    }
    ++pos;
  } catch (Error &err) {
    throw err;
  }
}

WhereClauseItemNode::WhereClauseItemNode(const std::vector<Token> &tokens, uint32_t &pos, const uint32_t &length) : ASTNode("Where Clause Item") {
  try {
    type_ = node_pool.Make<TypeNode>(tokens, pos, length);
    CheckLength(pos, length);
    if (tokens[pos].lexeme != ":") {
      throw Error("try parsing Where Clause Item Node but no :");
    }
    CheckLength(pos, length);
    if (tokens[pos].lexeme != "," && tokens[pos].lexeme != ";" && tokens[pos].lexeme != "{") {
      type_param_bounds_ = node_pool.Make<TypeParamBoundsNode>(tokens, pos, length);
    }
  } catch (Error &err) {
    throw err;
  }
}

WhereClauseNode::WhereClauseNode(const std::vector<Token> &tokens, uint32_t &pos, const uint32_t &length) : ASTNode("Where Clause") {
  try {
    CheckLength(pos, length);
    if (tokens[pos].lexeme != "where") {
      throw Error("try parsing Where Clause Node but no where");
    }
    while (pos < length && tokens[pos].lexeme != "{" && tokens[pos].lexeme != ";") {
      where_clause_items_.push_back(node_pool.Make<WhereClauseItemNode>(tokens, pos, length));
      CheckLength(pos, length);
      if (tokens[pos].lexeme == "{" || tokens[pos].lexeme == ";") {
        break;
      }
      if (tokens[pos].lexeme != ",") {
        throw Error("try parsing Where Clause Node but not ,");
      }
      ++pos;
    }
  } catch (Error &err) {
    throw err;
  }
}
