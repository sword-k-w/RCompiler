#include "parser/node/statement.h"

StatementNode::StatementNode(const std::vector<Token> &tokens, uint32_t &pos, const uint32_t &length) : ASTNode("Statement") {
  try {
    CheckLength(pos, length);
    if (tokens[pos].lexeme == ";") {
      semicolon_ = true;
      ++pos;
    } else if (tokens[pos].lexeme == "let") {
      let_statement_ = node_pool.Make<LetStatementNode>(tokens, pos, length);
    } else if (tokens[pos].lexeme == "mod" || tokens[pos].lexeme == "fn" || tokens[pos].lexeme == "struct"
      || tokens[pos].lexeme == "enum" || tokens[pos].lexeme == "const" || tokens[pos].lexeme == "trait"
      || tokens[pos].lexeme == "impl") {
      item_ = node_pool.Make<ItemNode>(tokens, pos, length);
    } else {
      expr_statement_ = node_pool.Make<ExpressionStatementNode>(tokens, pos, length);
    }
  } catch (Error &err) {
    throw err;
  }
}
