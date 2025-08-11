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
  IdentifierNode *identifier_ = nullptr;
  std::vector<ItemNode *> items_;
  bool semicolon_ = false;
};

#endif //MODULE_H
