#pragma once

#include "parser/class_declaration.h"
#include "lexer/lexer.h"
#include "parser/node/AST_node.h"
#include <cstdint>
#include <memory>
#include <set>

class EnumVariantsNode : public ASTNode {
  friend class Printer;
  friend class FirstChecker;
  friend class SecondChecker;
  friend class ThirdChecker;
  friend class IRGenerator;
public:
  EnumVariantsNode() = delete;
  EnumVariantsNode(const std::vector<Token> &, uint32_t &, const uint32_t &);
  void Accept(VisitorBase *) override;
private:
  std::vector<std::shared_ptr<EnumVariantNode>> enum_variant_s_;
  uint32_t comma_cnt_ = 0;
};

class EnumerationNode : public ASTNode {
  friend class Printer;
  friend class FirstChecker;
  friend class SecondChecker;
  friend class ThirdChecker;
  friend class IRGenerator;
public:
  EnumerationNode() = delete;
  EnumerationNode(const std::vector<Token> &, uint32_t &, const uint32_t &);
  void Accept(VisitorBase *) override;
private:
  std::shared_ptr<IdentifierNode> identifier_;
  std::shared_ptr<EnumVariantsNode> enum_variants_;

  std::map<std::string, uint32_t> enum_;
};

