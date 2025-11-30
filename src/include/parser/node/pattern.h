#pragma once

#include "parser/class_declaration.h"
#include "lexer/lexer.h"
#include "parser/node/AST_node.h"
#include <cstdint>
#include <memory>

class IdentifierPatternNode : public ASTNode {
  friend class Printer;
  friend class FirstChecker;
  friend class SecondChecker;
  friend class ThirdChecker;
  friend class IRGenerator;
public:
  IdentifierPatternNode() = delete;
  IdentifierPatternNode(const std::vector<Token> &, uint32_t &, const uint32_t &);
  void Accept(VisitorBase *) override;
private:
  bool ref_ = false;
  bool mut_ = false;
  std::shared_ptr<IdentifierNode> identifier_;
};

class ReferencePatternNode : public ASTNode {
  friend class Printer;
  friend class FirstChecker;
  friend class SecondChecker;
  friend class ThirdChecker;
  friend class IRGenerator;
public:
  ReferencePatternNode() = delete;
  ReferencePatternNode(const std::vector<Token> &, uint32_t &, const uint32_t &);
  void Accept(VisitorBase *) override;
private:
  bool single_ = false;
  bool mut_ = false;
  std::shared_ptr<PatternWithoutRangeNode> pattern_without_range_;
};

class PatternWithoutRangeNode : public ASTNode {
  friend class Printer;
  friend class FirstChecker;
  friend class SecondChecker;
  friend class ThirdChecker;
  friend class IRGenerator;
public:
  PatternWithoutRangeNode() = delete;
  PatternWithoutRangeNode(const std::vector<Token> &, uint32_t &, const uint32_t &);
  void Accept(VisitorBase *) override;
private:
  std::shared_ptr<IdentifierPatternNode> identifier_pattern_;
  std::shared_ptr<ReferencePatternNode> reference_pattern_;
};
