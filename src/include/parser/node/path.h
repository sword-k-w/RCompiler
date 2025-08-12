#ifndef PATH_H
#define PATH_H

#include "lexer/lexer.h"
#include "parser/node/AST_node.h"

class TypePathNode : public ASTNode {
public:
  TypePathNode() = delete;
  TypePathNode(const std::vector<Token>&, uint32_t&, const uint32_t&);
private:
};

#endif //PATH_H
