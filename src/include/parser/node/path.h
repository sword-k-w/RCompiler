#pragma once

#include "parser/class_declaration.h"
#include "lexer/lexer.h"
#include "parser/node/AST_node.h"
#include <cstdint>
#include <memory>

class PathIdentSegmentNode : public ASTNode {
  friend class Printer;
  friend class FirstChecker;
  friend class SecondChecker;
  friend class ThirdChecker;
  friend class IRGenerator;
public:
  PathIdentSegmentNode() = delete;
  PathIdentSegmentNode(const std::vector<Token>&, uint32_t&, const uint32_t &);
  void Accept(VisitorBase *) override;
private:
  std::shared_ptr<IdentifierNode> identifier_;
  std::shared_ptr<SelfLowerNode> self_lower_;
  std::shared_ptr<SelfUpperNode> self_upper_;
};

