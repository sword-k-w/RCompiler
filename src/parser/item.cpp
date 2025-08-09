#include "parser/item.h"
#include "common/error.h"

ItemNode::ItemNode(const std::vector<Token> &tokens, uint32_t &pos, const uint32_t &length) {
  if (pos >= length) {
    Error("try parsing Item Node but no enough length");
  }
  if (tokens[pos].type != kIDENTIFIER_OR_KEYWORD) {
    Error("try parsing Item Node but first token isn't identifier or keyword");
  }
  if (tokens[pos].lexeme == "mod") {
    module_ = new ModuleNode(tokens, pos, length);
  } else if (tokens[pos].lexeme == "fn") {
    function_ = new FunctionNode(tokens, pos, length);
  } else if (tokens[pos].lexeme == "struct") {
    struct_ = new StructNode(tokens, pos, length);
  } else if (tokens[pos].lexeme == "enum") {
    enumeration_ = new EnumerationNode(tokens, pos, length);
  } else if (tokens[pos].lexeme == "const") {
    constant_item_ = new ConstantItemNode(tokens, pos, length);
  } else if (tokens[pos].lexeme == "trait") {
    trait_ = new TraitNode(tokens, pos, length);
  } else if (tokens[pos].lexeme == "impl") {
    implementation_ = new ImplementationNode(tokens, pos, length);
  } else {
    Error("try parsing Item Node but the first identifier or keyword is unexpected");
  }
}
