#include "parser/node/item.h"
#include "common/error.h"
#include "parser/node/terminal.h"
#include "parser/node/type.h"
#include "parser/node/expression.h"
#include "parser/node/function.h"
#include "parser/node/struct.h"
#include "parser/node/enumeration.h"
#include "parser/node/trait.h"
#include "parser/node/implementation.h"

ConstantItemNode::ConstantItemNode(const std::vector<Token> &tokens, uint32_t &pos, const uint32_t &length) : ASTNode("Constant Item") {
  try {
    CheckLength(pos, length);
    if (tokens[pos].lexeme != "const") {
      throw Error("try parsing Constant Item Node but the first token is not const");
    }
    ++pos;
    identifier_ = std::make_shared<IdentifierNode>(tokens, pos, length);
    CheckLength(pos, length);
    if (tokens[pos].lexeme != ":") {
      throw Error("try parsing Constant Item Node but no :");
    }
    ++pos;
    type_ = std::make_shared<TypeNode>(tokens, pos, length);
    CheckLength(pos, length);
    if (tokens[pos].lexeme != ";") {
      if (tokens[pos].lexeme != "=") {
        throw Error("try parsing Constant Item Node but no =");
      }
      ++pos;
      expr_ = std::make_shared<ExpressionNode>(tokens, pos, length);
      CheckLength(pos, length);
      if (tokens[pos].lexeme != ";") {
        throw Error("try parsing Constant Item Node but no ;");
      }
    }
    ++pos;
  } catch (Error &) {
    throw;
  }
}

AssociatedItemNode::AssociatedItemNode(const std::vector<Token> &tokens, uint32_t &pos, const uint32_t &length) : ASTNode("Associated Item"){
  try {
    CheckLength(pos, length);
    if (tokens[pos].lexeme == "const") {
      if (pos + 1 < length && tokens[pos + 1].lexeme == "fn") {
        function_ = std::make_shared<FunctionNode>(tokens, pos, length);
      } else {
        constant_item_ = std::make_shared<ConstantItemNode>(tokens, pos, length);
      }
    } else if (tokens[pos].lexeme == "fn") {
      function_ = std::make_shared<FunctionNode>(tokens, pos, length);
    } else {
      throw Error("try parsing Associated Item Node but first token isn't const or fn");
    }
  } catch (Error &) {
    throw;
  }
}

ItemNode::ItemNode(const std::vector<Token> &tokens, uint32_t &pos, const uint32_t &length) : ASTNode("Item") {
  try {
    CheckLength(pos, length);
    if (tokens[pos].type != kIDENTIFIER_OR_KEYWORD) {
      throw Error("try parsing Item Node but first token isn't identifier or keyword");
    }
    if (tokens[pos].lexeme == "fn") {
      function_ = std::make_shared<FunctionNode>(tokens, pos, length);
    } else if (tokens[pos].lexeme == "struct") {
      struct_ = std::make_shared<StructNode>(tokens, pos, length);
    } else if (tokens[pos].lexeme == "enum") {
      enumeration_ = std::make_shared<EnumerationNode>(tokens, pos, length);
    } else if (tokens[pos].lexeme == "const") {
      if (pos + 1 < length && tokens[pos + 1].lexeme == "fn") {
        function_ = std::make_shared<FunctionNode>(tokens, pos, length);
      } else {
        constant_item_ = std::make_shared<ConstantItemNode>(tokens, pos, length);
      }
    } else if (tokens[pos].lexeme == "trait") {
      trait_ = std::make_shared<TraitNode>(tokens, pos, length);
    } else if (tokens[pos].lexeme == "impl") {
      implementation_ = std::make_shared<ImplementationNode>(tokens, pos, length);
    } else {
      throw Error("try parsing Item Node but the first identifier or keyword is unexpected");
    }
  } catch (Error &) {
    throw;
  }
}
