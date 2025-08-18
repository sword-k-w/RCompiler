#ifndef PATH_H
#define PATH_H

#include "lexer/lexer.h"
#include "parser/node/AST_node.h"
#include "parser/node/terminal.h"

class PathIdentSegmentNode : public ASTNode {
  friend class Printer;
public:
  PathIdentSegmentNode() = delete;
  PathIdentSegmentNode(const std::vector<Token>&, uint32_t&, const uint32_t&);
private:
  IdentifierNode *identifier_ = nullptr;
  SelfLowerNode *self_lower_ = nullptr;
  SelfUpperNode *self_upper_ = nullptr;
};

#endif //PATH_H
