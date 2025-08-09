#ifndef MODULE_H
#define MODULE_H

#include "parser/AST_node.h"
#include "lexer/lexer.h"

class ModuleNode : public ASTNode {
public:
  ModuleNode() = delete;
  ModuleNode(const std::vector<Token> &, uint32_t &, const uint32_t &);
private:
};

#endif //MODULE_H
