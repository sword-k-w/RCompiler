#pragma once
#include <string>
#include <vector>
#include <memory>
#include <deque>

class IRVisitorBase;

class IRNode {
public:
  virtual void Accept(IRVisitorBase *) = 0;
private:

};

class IRStructNode : public IRNode {
  friend class IRPrinter;
public:
  IRStructNode() = delete;
  explicit IRStructNode(const std::string &);
  void AddMember(const std::string &);
  void Accept(IRVisitorBase *) override;
private:
  std::string name_;
  std::vector<std::string> members_;
};

class IRInstructionNode : public IRNode {
};

class IRArithmeticInstructionNode : public IRInstructionNode {
  friend class IRPrinter;
public:
  IRArithmeticInstructionNode() = delete;
  IRArithmeticInstructionNode(const std::string &, const std::string &, const std::string &,
    const std::string &, const std::string &, const bool &);
  void Accept(IRVisitorBase *) override;
private:
  std::string result_;
  std::string op_;
  std::string type_;
  std::string operand1_;
  std::string operand2_;
  bool is_unsigned_;
};

class IRNegationInstructionNode : public IRInstructionNode {
  friend class IRPrinter;
public:
  IRNegationInstructionNode() = delete;
  IRNegationInstructionNode(const std::string &, const bool &, const std::string &, const std::string &);
  void Accept(IRVisitorBase *) override;
private:
  std::string result_;
  bool is_minus_;
  std::string type_;
  std::string operand_;
};

class IRBranchInstructionNode : public IRInstructionNode {
  friend class IRPrinter;
public:
  IRBranchInstructionNode() = delete;
  IRBranchInstructionNode(const std::string &, const uint32_t &, const uint32_t &);
  void Accept(IRVisitorBase *) override;
private:
  std::string condition_;
  uint32_t true_branch_;
  uint32_t false_branch_;
};

class IRJumpInstructionNode : public IRInstructionNode {
  friend class IRPrinter;
public:
  IRJumpInstructionNode() = delete;
  explicit IRJumpInstructionNode(const uint32_t &);
  void Accept(IRVisitorBase *) override;
private:
  uint32_t destination_;
};

class IRReturnInstructionNode : public IRInstructionNode {
  friend class IRPrinter;
public:
  IRReturnInstructionNode() = default;
  IRReturnInstructionNode(const std::string &, const std::string &);
  void Accept(IRVisitorBase *) override;
private:
  std::string type_; // empty means void which also means name_ is also empty
  std::string name_;
};

class IRAllocateInstructionNode : public IRInstructionNode {
  friend class IRPrinter;
public:
  IRAllocateInstructionNode() = delete;
  IRAllocateInstructionNode(const std::string &, const std::string &);
  void Accept(IRVisitorBase *) override;
private:
  std::string result_;
  std::string type_;
};

class IRLoadInstructionNode : public IRInstructionNode {
  friend class IRPrinter;
public:
  IRLoadInstructionNode() = delete;
  IRLoadInstructionNode(const std::string &, const std::string &, const std::string &);
  void Accept(IRVisitorBase *) override;
private:
  std::string result_;
  std::string type_;
  std::string pointer_;
};

class IRStoreVariableInstructionNode : public IRInstructionNode {
  friend class IRPrinter;
public:
  IRStoreVariableInstructionNode() = delete;
  IRStoreVariableInstructionNode(const std::string &, const std::string &, const std::string &);
  void Accept(IRVisitorBase *) override;
private:
  std::string type_;
  std::string value_;
  std::string pointer_;
};

class IRStoreConstInstructionNode : public IRInstructionNode {
  friend class IRPrinter;
public:
  IRStoreConstInstructionNode() = delete;
  IRStoreConstInstructionNode(const std::string &, const int32_t &, const std::string &);
  void Accept(IRVisitorBase *) override;
private:
  std::string type_;
  int32_t value_;
  std::string pointer_;
};

class IRGetElementPtrInstructionNode : public IRInstructionNode {
  friend class IRPrinter;
public:
  IRGetElementPtrInstructionNode() = delete;
  IRGetElementPtrInstructionNode(const std::string &, const std::string &, const std::string &, const uint32_t &);
  void Accept(IRVisitorBase *) override;
private:
  std::string result_;
  std::string type_;
  std::string ptrval_;
  uint32_t index_;
};

// the index is variable now
class IRGetElementPtrPrimeInstructionNode : public IRInstructionNode {
  friend class IRPrinter;
public:
  IRGetElementPtrPrimeInstructionNode() = delete;
  IRGetElementPtrPrimeInstructionNode(const std::string &, const std::string &, const std::string &, const std::string &);
  void Accept(IRVisitorBase *) override;
private:
  std::string result_;
  std::string type_;
  std::string ptrval_;
  std::string index_;
};

class IRCompareInstructionNode : public IRInstructionNode {
  friend class IRPrinter;
public:
  enum Operator {
    kEq, kNe, kUgt, kUge, kUlt, kUle, kSgt, kSge, kSlt, kSle
  };
  IRCompareInstructionNode() = delete;
  IRCompareInstructionNode(const std::string &, const Operator &, const std::string &,
    const std::string &, const std::string &);
  void Accept(IRVisitorBase *) override;
private:
  std::string result_;
  Operator op_;
  std::string type_;
  std::string operand1_;
  std::string operand2_;
};

class IRArgumentNode : public IRInstructionNode {
  friend class IRPrinter;
public:
  IRArgumentNode() = delete;
  IRArgumentNode(const std::string &, const std::string &);
  void Accept(IRVisitorBase *) override;
private:
  std::string type_;
  std::string value_;
};

class IRCallInstructionNode : public IRInstructionNode {
  friend class IRPrinter;
public:
  IRCallInstructionNode() = delete;
  IRCallInstructionNode(const std::string &, const std::string &, const std::string &);
  void AddArgument(std::shared_ptr<IRArgumentNode>);
  void Accept(IRVisitorBase *) override;
private:
  std::string result_;
  std::string result_type_; // empty means void which also means result_ is also empty
  std::string function_name_;
  std::vector<std::shared_ptr<IRArgumentNode>> arguments_;
};

class IRSelectInstructionNode : public IRInstructionNode {
  friend class IRPrinter;
public:
  IRSelectInstructionNode() = delete;
  IRSelectInstructionNode(const std::string &, const std::string &);
  void Accept(IRVisitorBase *) override;
private:
  std::string result_;
  std::string cond_;
};

class IRBlockNode : public IRNode {
  friend class IRPrinter;
public:
  IRBlockNode() = delete;
  explicit IRBlockNode(const uint32_t &);
  void AddInstruction(std::shared_ptr<IRInstructionNode>);
  void Accept(IRVisitorBase *) override;
  uint32_t GetID() const;
private:
  uint32_t id_;
  std::deque<std::shared_ptr<IRInstructionNode>> instructions_;
  bool end_{false};
};

class IRParameterNode : public IRNode {
  friend class IRPrinter;
public:
  IRParameterNode() = delete;
  IRParameterNode(const std::string &, const std::string &);
  void Accept(IRVisitorBase *) override;
private:
  std::string type_;
  std::string name_;
};

class IRFunctionNode : public IRNode {
  friend class IRPrinter;
public:
  IRFunctionNode() = delete;
  IRFunctionNode(const std::string &, const std::string &);
  void AddParameter(std::shared_ptr<IRParameterNode>);
  void AddBlock(std::shared_ptr<IRBlockNode>);
  void Accept(IRVisitorBase *) override;
private:
  std::string type_;
  std::string name_;
  std::vector<std::shared_ptr<IRParameterNode>> parameters_;
  std::vector<std::shared_ptr<IRBlockNode>> blocks_;
};

class IRRootNode : public IRNode {
  friend class IRPrinter;
public:
  IRRootNode() = default;
  void AddStruct(std::shared_ptr<IRStructNode>);
  void AddFunction(std::shared_ptr<IRFunctionNode>);
  void Accept(IRVisitorBase *) override;
private:
  std::vector<std::shared_ptr<IRStructNode>> structs_;
  std::vector<std::shared_ptr<IRFunctionNode>> functions_;
};