#include "parser/node/statement.h"
#include "common/error.h"
#include "parser/node/expression.h"
#include "parser/node/pattern.h"
#include "parser/node/type.h"
#include "parser/node/item.h"

LetStatementNode::LetStatementNode(const std::vector<Token> &tokens, uint32_t &pos, const uint32_t &length) : ASTNode("Let Statement") {
  try {
    CheckLength(pos, length);
    if (tokens[pos].lexeme != "let") {
      throw Error("try parsing Let Statement Node but no let");
    }
    ++pos;
    pattern_no_top_alt_ = std::make_shared<PatternNoTopAltNode>(tokens, pos, length);
    CheckLength(pos, length);
    if (tokens[pos].lexeme != ":") {
      throw Error("try parsing Let Statement but no :");
    }
    ++pos;
    type_ = std::make_shared<TypeNode>(tokens, pos, length);
    CheckLength(pos, length);
    if (tokens[pos].lexeme != "=") {
      throw Error("try parsing Let Statement but no =");
    }
    ++pos;
    expr_ = std::make_shared<ExpressionNode>(tokens, pos, length);
    CheckLength(pos, length);
    if (tokens[pos].lexeme != ";") {
      throw Error("try parsing Let Statement Node but no ;");
    }
    ++pos;
  } catch (Error &) { throw; }
}

ExpressionStatementNode::ExpressionStatementNode(const std::vector<Token> &tokens, uint32_t &pos, const uint32_t &length) : ASTNode("Expression Statement") {
  try {
    expr_ = std::make_shared<ExpressionNode>(tokens, pos, length);
    CheckLength(pos, length);
    if (tokens[pos].lexeme == ";") {
      semicolon_ = true;
      ++pos;
    } else if (expr_->Type() != kExprWithBlock) {
      throw Error("try parsing Expression Statement Node but no ;");
    }
  } catch (Error &) { throw; }
}

StatementNode::StatementNode(const std::vector<Token> &tokens, uint32_t &pos, const uint32_t &length) : ASTNode("Statement") {
  try {
    CheckLength(pos, length);
    if (tokens[pos].lexeme == ";") {
      semicolon_ = true;
      ++pos;
    } else if (tokens[pos].lexeme == "let") {
      let_statement_ = std::make_shared<LetStatementNode>(tokens, pos, length);
    } else if (tokens[pos].lexeme == "fn" || tokens[pos].lexeme == "struct"
      || tokens[pos].lexeme == "enum" || tokens[pos].lexeme == "const" || tokens[pos].lexeme == "trait"
      || tokens[pos].lexeme == "impl") {
      item_ = std::make_shared<ItemNode>(tokens, pos, length);
    } else {
      expr_statement_ = std::make_shared<ExpressionStatementNode>(tokens, pos, length);
    }
  } catch (Error &) { throw; }
}

StatementsNode::StatementsNode(const std::vector<Token> &tokens, uint32_t &pos, const uint32_t &length) : ASTNode("Statements") {
  try {
    while (pos < length && tokens[pos].lexeme != "}") {
      uint32_t tmp = pos;
      try {
        statement_s_.push_back(std::make_shared<StatementNode>(tokens, pos, length));
      } catch (...) {
        pos = tmp;
        break;
      }
    }
    if (tokens[pos].lexeme != "}") {
      expr_without_block_ = std::make_shared<ExpressionWithoutBlockNode>(tokens, pos, length);
    }
  } catch (Error &) { throw; }
}
