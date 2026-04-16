#pragma once
#include <string>
#include <vector>
#include <memory>
#include <deque>
#include <map>

enum StorageType {
  kRegister, kMemory, kConst
};

class IRVisitorBase;

class IRNode {
public:
  virtual void Accept(IRVisitorBase *) = 0;
private:

};

class IRArrayNode : public IRNode {
  friend class IRPrinter;
  friend class Preprocessor;
  friend class MemoryAllocator;
  friend class AssemblyGenerator;
public:
  IRArrayNode() = default;
  explicit IRArrayNode(const std::string &);
  void AddLength(uint32_t);
  void SetBaseType(const std::string &);
  void Accept(IRVisitorBase *) override;
  bool IsEmpty() const;
private:
  std::vector<uint32_t> length_;
  std::string base_type_; // empty means void

  uint32_t allocated_size_;
  uint32_t align_;
};

class IRStructNode : public IRNode {
  friend class IRPrinter;
  friend class Preprocessor;
  friend class MemoryAllocator;
  friend class AssemblyGenerator;
public:
  IRStructNode() = delete;
  explicit IRStructNode(const std::string &);
  void AddMember(std::shared_ptr<IRArrayNode>);
  void Accept(IRVisitorBase *) override;
private:
  std::string name_;
  std::vector<std::shared_ptr<IRArrayNode>> members_;

  uint32_t allocated_size_;
  uint32_t align_;
};

class IRInstructionNode : public IRNode {
protected:
  StorageType storage_type_;
  uint32_t address_;
};

class IRArithmeticInstructionNode : public IRInstructionNode {
  friend class IRPrinter;
  friend class Preprocessor;
  friend class MemoryAllocator;
  friend class AssemblyGenerator;
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
  friend class Preprocessor;
  friend class MemoryAllocator;
  friend class AssemblyGenerator;
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
  friend class Preprocessor;
  friend class MemoryAllocator;
  friend class AssemblyGenerator;
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
  friend class Preprocessor;
  friend class MemoryAllocator;
  friend class AssemblyGenerator;
public:
  IRJumpInstructionNode() = delete;
  explicit IRJumpInstructionNode(const uint32_t &);
  void Accept(IRVisitorBase *) override;
private:
  uint32_t destination_;
};

class IRReturnInstructionNode : public IRInstructionNode {
  friend class IRPrinter;
  friend class Preprocessor;
  friend class MemoryAllocator;
  friend class AssemblyGenerator;
public:
  IRReturnInstructionNode();
  IRReturnInstructionNode(std::shared_ptr<IRArrayNode>, const std::string &);
  void Accept(IRVisitorBase *) override;
private:
  std::shared_ptr<IRArrayNode> type_; // empty means void which also means name_ is also empty
  std::string name_;
};

class IRAllocateInstructionNode : public IRInstructionNode {
  friend class IRPrinter;
  friend class Preprocessor;
  friend class MemoryAllocator;
  friend class AssemblyGenerator;
public:
  IRAllocateInstructionNode() = delete;
  IRAllocateInstructionNode(const std::string &, std::shared_ptr<IRArrayNode>);
  void Accept(IRVisitorBase *) override;
private:
  std::string result_;
  std::shared_ptr<IRArrayNode> type_;

  StorageType inner_storage_type_;
  uint32_t inner_address_;
};

class IRLoadInstructionNode : public IRInstructionNode {
  friend class IRPrinter;
  friend class Preprocessor;
  friend class MemoryAllocator;
  friend class AssemblyGenerator;
public:
  IRLoadInstructionNode() = delete;
  IRLoadInstructionNode(const std::string &, std::shared_ptr<IRArrayNode>, const std::string &);
  void Accept(IRVisitorBase *) override;
private:
  std::string result_;
  std::shared_ptr<IRArrayNode> type_;
  std::string pointer_;
};

class IRStoreVariableInstructionNode : public IRInstructionNode {
  friend class IRPrinter;
  friend class Preprocessor;
  friend class MemoryAllocator;
  friend class AssemblyGenerator;
public:
  IRStoreVariableInstructionNode() = delete;
  IRStoreVariableInstructionNode(std::shared_ptr<IRArrayNode>, const std::string &, const std::string &);
  void Accept(IRVisitorBase *) override;
private:
  std::shared_ptr<IRArrayNode> type_;
  std::string value_;
  std::string pointer_;
};

class IRStoreConstInstructionNode : public IRInstructionNode {
  friend class IRPrinter;
  friend class Preprocessor;
  friend class MemoryAllocator;
  friend class AssemblyGenerator;
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
  friend class Preprocessor;
  friend class MemoryAllocator;
  friend class AssemblyGenerator;
public:
  IRGetElementPtrInstructionNode() = delete;
  IRGetElementPtrInstructionNode(const std::string &, std::shared_ptr<IRArrayNode>, const std::string &, const uint32_t &);
  void Accept(IRVisitorBase *) override;
private:
  std::string result_;
  std::shared_ptr<IRArrayNode> type_;
  std::string ptrval_;
  uint32_t index_;
};

// the index is variable now
class IRGetElementPtrPrimeInstructionNode : public IRInstructionNode {
  friend class IRPrinter;
  friend class Preprocessor;
  friend class MemoryAllocator;
  friend class AssemblyGenerator;
public:
  IRGetElementPtrPrimeInstructionNode() = delete;
  IRGetElementPtrPrimeInstructionNode(const std::string &, std::shared_ptr<IRArrayNode>, const std::string &, const std::string &);
  void Accept(IRVisitorBase *) override;
private:
  std::string result_;
  std::shared_ptr<IRArrayNode> type_;
  std::string ptrval_;
  std::string index_;
};

class IRCompareInstructionNode : public IRInstructionNode {
  friend class IRPrinter;
  friend class Preprocessor;
  friend class MemoryAllocator;
  friend class AssemblyGenerator;
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
  friend class Preprocessor;
  friend class MemoryAllocator;
  friend class AssemblyGenerator;
public:
  IRArgumentNode() = delete;
  IRArgumentNode(std::shared_ptr<IRArrayNode>, const std::string &);
  void Accept(IRVisitorBase *) override;
private:
  std::shared_ptr<IRArrayNode> type_;
  std::string value_;
};

class IRCallInstructionNode : public IRInstructionNode {
  friend class IRPrinter;
  friend class Preprocessor;
  friend class MemoryAllocator;
  friend class AssemblyGenerator;
public:
  IRCallInstructionNode() = delete;
  IRCallInstructionNode(const std::string &, std::shared_ptr<IRArrayNode>, const std::string &);
  void AddArgument(std::shared_ptr<IRArgumentNode>);
  void Accept(IRVisitorBase *) override;
private:
  std::string result_;
  std::shared_ptr<IRArrayNode> result_type_; // empty means void which also means result_ is also empty
  std::string function_name_;
  std::vector<std::shared_ptr<IRArgumentNode>> arguments_;

  StorageType storage_type_;
  uint32_t address_;
};

class IRSelectInstructionNode : public IRInstructionNode {
  friend class IRPrinter;
  friend class Preprocessor;
  friend class MemoryAllocator;
  friend class AssemblyGenerator;
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
  friend class Preprocessor;
  friend class MemoryAllocator;
  friend class AssemblyGenerator;
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
  friend class Preprocessor;
  friend class MemoryAllocator;
  friend class AssemblyGenerator;
public:
  IRParameterNode() = delete;
  IRParameterNode(std::shared_ptr<IRArrayNode>, const std::string &);
  void Accept(IRVisitorBase *) override;
private:
  std::shared_ptr<IRArrayNode> type_;
  std::string name_;

  StorageType storage_type_ = kMemory;
  uint32_t address_ = 0;
};

class IRFunctionNode : public IRNode {
  friend class IRPrinter;
  friend class Preprocessor;
  friend class MemoryAllocator;
  friend class AssemblyGenerator;
public:
  IRFunctionNode() = delete;
  IRFunctionNode(std::shared_ptr<IRArrayNode>, const std::string &);
  void AddParameter(std::shared_ptr<IRParameterNode>);
  void AddBlock(std::shared_ptr<IRBlockNode>);
  void Accept(IRVisitorBase *) override;
private:
  std::shared_ptr<IRArrayNode> type_;
  std::string name_;
  std::vector<std::shared_ptr<IRParameterNode>> parameters_;
  std::vector<std::shared_ptr<IRBlockNode>> blocks_;

  uint32_t stack_size_ = 0;
  std::map<std::string, IRNode *> variables_;
};

class IRRootNode : public IRNode {
  friend class IRPrinter;
  friend class Preprocessor;
  friend class MemoryAllocator;
  friend class AssemblyGenerator;
public:
  IRRootNode() = default;
  void AddStruct(std::shared_ptr<IRStructNode>);
  void AddFunction(std::shared_ptr<IRFunctionNode>);
  void Accept(IRVisitorBase *) override;
private:
  std::vector<std::shared_ptr<IRStructNode>> structs_;
  std::vector<std::shared_ptr<IRFunctionNode>> functions_;
};