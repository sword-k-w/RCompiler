#pragma once

#include "parser/class_declaration.h"
#include "lexer/lexer.h"
#include "parser/node/AST_node.h"
#include <cstdint>
#include <memory>

class TraitNode : public ASTNode {
  friend class Printer;
  friend class FirstChecker;
  friend class SecondChecker;
public:
  TraitNode() = delete;
  TraitNode(const std::vector<Token> &, uint32_t &, const uint32_t &);
  void Accept(VisitorBase *) override;
private:
  std::shared_ptr<IdentifierNode> identifier_;
  std::vector<std::shared_ptr<AssociatedItemNode>> asscociated_items_;

  std::map<std::string, ASTNode *> items_;
};
