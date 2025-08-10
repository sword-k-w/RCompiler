#ifndef MODULE_H
#define MODULE_H

#include "lexer/lexer.h"
#include "parser/node/AST_node.h"
#include "parser/node/item.h"
#include "parser/node/terminal.h"

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
