#pragma once

#include "parser/class_declaration.h"
#include "lexer/lexer.h"
#include "parser/node/AST_node.h"
#include <cstdint>

class ShorthandSelfNode : public ASTNode {
  friend class Printer;
  friend class FirstChecker;
  friend class SecondChecker;
  friend class ThirdChecker;
  friend class IRGenerator;
public:
  ShorthandSelfNode() = delete;
  ShorthandSelfNode(const std::vector<Token> &, uint32_t &, const uint32_t &);
  void Accept(VisitorBase *) override;
private:
  bool quote_ = false;
  bool mut_ = false;
  std::shared_ptr<SelfLowerNode> self_lower_;
};

class SelfParamNode : public ASTNode {
  friend class Printer;
  friend class FirstChecker;
  friend class SecondChecker;
  friend class ThirdChecker;
  friend class IRGenerator;
public:
  SelfParamNode() = delete;
  SelfParamNode(const std::vector<Token> &, uint32_t &, const uint32_t &);
  void Accept(VisitorBase *) override;
private:
  std::shared_ptr<ShorthandSelfNode> shorthand_self_;
};

class FunctionParamNode : public ASTNode {
  friend class Printer;
  friend class FirstChecker;
  friend class SecondChecker;
  friend class ThirdChecker;
  friend class IRGenerator;
public:
  FunctionParamNode() = delete;
  FunctionParamNode(const std::vector<Token> &, uint32_t &, const uint32_t &);
  void Accept(VisitorBase *) override;
private:
  std::shared_ptr<PatternNoTopAltNode> pattern_no_top_alt_;
  std::shared_ptr<TypeNode> type_;
};

class FunctionParametersNode : public ASTNode {
  friend class Printer;
  friend class FirstChecker;
  friend class SecondChecker;
  friend class ThirdChecker;
  friend class IRGenerator;
public:
  FunctionParametersNode() = delete;
  FunctionParametersNode(const std::vector<Token> &, uint32_t &, const uint32_t &);
  void Accept(VisitorBase *) override;
private:
  std::shared_ptr<SelfParamNode> self_param_;
  uint32_t comma_cnt_ = 0;
  std::vector<std::shared_ptr<FunctionParamNode>> function_params_;
};

class FunctionReturnTypeNode : public ASTNode {
  friend class Printer;
  friend class FirstChecker;
  friend class SecondChecker;
  friend class ThirdChecker;
  friend class IRGenerator;
public:
  FunctionReturnTypeNode() = delete;
  FunctionReturnTypeNode(const std::vector<Token> &, uint32_t &, const uint32_t &);
  void Accept(VisitorBase *) override;
private:
  std::shared_ptr<TypeNode> type_;
};

class FunctionNode : public ASTNode {
  friend class Printer;
  friend class FirstChecker;
  friend class SecondChecker;
  friend class ThirdChecker;
  friend class IRGenerator;
public:
  FunctionNode() = delete;
  FunctionNode(const std::vector<Token> &, uint32_t &, const uint32_t &);
  void Accept(VisitorBase *) override;
private:
  bool const_ = false;
  std::shared_ptr<IdentifierNode> identifier_;
  std::shared_ptr<FunctionParametersNode> function_parameters_;
  std::shared_ptr<FunctionReturnTypeNode> function_return_type_;
  bool semicolon_ = false;
  std::shared_ptr<BlockExpressionNode> block_expr_;

  bool in_trait_ = false;
  bool in_implement_ = false;
};
