#ifndef MODULE_H
#define MODULE_H

#include "parser/AST_node.h"
#include "parser/item.h"
#include "parser/terminal.h"
#include "lexer/lexer.h"

class ModuleNode : public ASTNode {
public:
  ModuleNode() = delete;
  ModuleNode(const std::vector<Token> &, uint32_t &, const uint32_t &);
private:
  IdentifierOrKeywordNode *identifier_or_keyword_ = nullptr;
  std::vector<ItemNode *> items_;
  bool semicolon = false;
};

#endif //MODULE_H
