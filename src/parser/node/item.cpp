#include "parser/node/item.h"

ConstantItemNode::ConstantItemNode(const std::vector<Token> &tokens, uint32_t &pos, const uint32_t &length) : ASTNode("Constant Item") {
  try {
    CheckLength(pos, length);
    if (tokens[pos].lexeme != "const") {
      throw Error("try parsing Constant Item Node but the first token is not const");
    }
    ++pos;
    identifier_ = node_pool.Make<IdentifierNode>(tokens, pos, length);
    CheckLength(pos, length);
    if (tokens[pos].lexeme != ":") {
      throw Error("try parsing Constant Item Node but no :");
    }
    ++pos;
    type_ = node_pool.Make<TypeNode>(tokens, pos, length);
    CheckLength(pos, length);
    if (tokens[pos].lexeme != ";") {
      if (tokens[pos].lexeme != "=") {
        throw Error("try parsing Constant Item Node but no =");
      }
      ++pos;
      expr_ = node_pool.Make<ExpressionNode>(tokens, pos, length);
      CheckLength(pos, length);
      if (tokens[pos].lexeme != ";") {
        throw Error("try parsing Constant Item Node but no ;");
      }
    }
    ++pos;
  } catch (Error &err) {
    throw err;
  }
}

AssociatedItemNode::AssociatedItemNode(const std::vector<Token> &tokens, uint32_t &pos, const uint32_t &length) : ASTNode("Associated Item"){
  try {
    CheckLength(pos, length);
    if (tokens[pos].lexeme == "const") {
      if (pos + 1 < length && tokens[pos + 1].lexeme == "fn") {
        function_ = node_pool.Make<FunctionNode>(tokens, pos, length);
      } else {
        constant_item_ = node_pool.Make<ConstantItemNode>(tokens, pos, length);
      }
    } else if (tokens[pos].lexeme == "fn") {
      function_ = node_pool.Make<FunctionNode>(tokens, pos, length);
    } else {
      throw Error("try parsing Associated Item Node but first token isn't const or fn");
    }
  } catch (Error &err) {
    throw err;
  }
}

ItemNode::ItemNode(const std::vector<Token> &tokens, uint32_t &pos, const uint32_t &length) : ASTNode("Item") {
  try {
    CheckLength(pos, length);
    if (tokens[pos].type != kIDENTIFIER_OR_KEYWORD) {
      throw Error("try parsing Item Node but first token isn't identifier or keyword");
    }
    if (tokens[pos].lexeme == "fn") {
      function_ = node_pool.Make<FunctionNode>(tokens, pos, length);
    } else if (tokens[pos].lexeme == "struct") {
      struct_ = node_pool.Make<StructNode>(tokens, pos, length);
    } else if (tokens[pos].lexeme == "enum") {
      enumeration_ = node_pool.Make<EnumerationNode>(tokens, pos, length);
    } else if (tokens[pos].lexeme == "const") {
      if (pos + 1 < length && tokens[pos + 1].lexeme == "fn") {
        function_ = node_pool.Make<FunctionNode>(tokens, pos, length);
      } else {
        constant_item_ = node_pool.Make<ConstantItemNode>(tokens, pos, length);
      }
    } else if (tokens[pos].lexeme == "trait") {
      trait_ = node_pool.Make<TraitNode>(tokens, pos, length);
    } else if (tokens[pos].lexeme == "impl") {
      implementation_ = node_pool.Make<ImplementationNode>(tokens, pos, length);
    } else {
      throw Error("try parsing Item Node but the first identifier or keyword is unexpected");
    }
  } catch (Error &err) {
    throw err;
  }
}
