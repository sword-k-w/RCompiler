#pragma once

#include "parser/class_declaration.h"
#include "lexer/lexer.h"
#include "parser/node/AST_node.h"
#include <cstdint>

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

