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

StructExpressionNode::StructExpressionNode(const std::vector<Token> &tokens, uint32_t &pos, const uint32_t &length) : ASTNode("Struct Expression") {
  try {
    CheckLength(pos, length);
    if (tokens[pos].lexeme == "Self") {

    }
  } catch (Error &err) {
    throw err;
  }
}
