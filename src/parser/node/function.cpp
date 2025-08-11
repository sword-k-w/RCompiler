#include "parser/node/function.h"

FunctionNode::FunctionNode(const std::vector<Token> &tokens, uint32_t &pos, const uint32_t &length) : ASTNode("Function") {
  CheckLength(pos, length);
  if (tokens[pos].lexeme != "fn") {
    Error("try parsing Function Node but the first token is not fn");
  }
  ++pos;
  identifier_ = node_pool.Make<IdentifierNode>(tokens, pos, length);
  CheckLength(pos, length);
  if (tokens[pos].lexeme != "(") {
    generic_params_ = node_pool.Make<GenericParamsNode>(tokens, pos, length);
    if (pos >= length || tokens[pos].lexeme != "(") {
      Error("try parsing Function Node but not '(' after generic params");
    }
  }
  ++pos;
  CheckLength(pos, length);
  if (tokens[pos].lexeme != ")") {
    function_parameters_ = node_pool.Make<FunctionParametersNode>(tokens, pos, length);
    if (pos >= length || tokens[pos].lexeme != ")") {
      Error("try parsing Function Node but not ')' after function parameters");
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
}
