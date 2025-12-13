#include "IR/IR_node.h"

#include <IR_visitor/base/IR_visitor_base.h>

IRStructNode::IRStructNode(const std::string &name) : name_(name) {}

IRArithmeticInstructionNode::IRArithmeticInstructionNode(const std::string &result, const std::string &op,
  const std::string &type, const std::string &operand1, const std::string &operand2, const bool &is_unsigned) :
  result_(result), op_(op), type_(type), operand1_(operand1), operand2_(operand2), is_unsigned_(is_unsigned) {}

IRNegationInstructionNode::IRNegationInstructionNode(const std::string &result, const bool &is_minus,
  const std::string &type, const std::string &operand) :
  result_(result), is_minus_(is_minus), type_(type), operand_(operand) {}

IRBranchInstructionNode::IRBranchInstructionNode(const std::string &condition, const uint32_t &true_branch,
  const uint32_t &false_branch) : condition_(condition), true_branch_(true_branch), false_branch_(false_branch) {}

IRJumpInstructionNode::IRJumpInstructionNode(const uint32_t &destination) : destination_(destination) {}

IRReturnInstructionNode::IRReturnInstructionNode(const std::string &type, const std::string &name) :
  type_(type), name_(name) {}

IRAllocateInstructionNode::IRAllocateInstructionNode(const std::string &result, const std::string &type) :
  result_(result), type_(type) {}

IRLoadInstructionNode::IRLoadInstructionNode(const std::string &result, const std::string &type,
  const std::string &pointer) : result_(result), type_(type), pointer_(pointer) {}

IRStoreVariableInstructionNode::IRStoreVariableInstructionNode(const std::string &type, const std::string &value,
  const std::string &pointer) : type_(type), value_(value), pointer_(pointer) {}

IRStoreConstInstructionNode::IRStoreConstInstructionNode(const std::string &type, const int32_t &value,
  const std::string &pointer) : type_(type), value_(value), pointer_(pointer) {}

IRGetElementPtrInstructionNode::IRGetElementPtrInstructionNode(const std::string &result, const std::string &type,
  const std::string &ptrval, const uint32_t &index) :
  result_(result), type_(type), ptrval_(ptrval), index_(index) {}

IRGetElementPtrPrimeInstructionNode::IRGetElementPtrPrimeInstructionNode(const std::string &result, const std::string &type,
  const std::string &ptrval, const std::string &index) :
  result_(result), type_(type), ptrval_(ptrval), index_(index) {}

IRCompareInstructionNode::IRCompareInstructionNode(const std::string &result, const Operator &op,
  const std::string &type, const std::string &operand1, const std::string &operand2) :
  result_(result), op_(op), type_(type), operand1_(operand1), operand2_(operand2) {}

IRArgumentNode::IRArgumentNode(const std::string &type, const std::string &value) : type_(type), value_(value) {}

IRCallInstructionNode::IRCallInstructionNode(const std::string &result, const std::string &result_type,
  const std::string &function_name) : result_(result), result_type_(result_type), function_name_(function_name) {}

IRSelectInstructionNode::IRSelectInstructionNode(const std::string &result, const std::string &cond) :
  result_(result), cond_(cond) {}

void IRCallInstructionNode::AddArgument(std::shared_ptr<IRArgumentNode> argument) {
  arguments_.emplace_back(argument);
}

void IRStructNode::AddMember(const std::string &member) {
  members_.emplace_back(member);
}

IRBlockNode::IRBlockNode(const uint32_t &id) : id_(id) {}

void IRBlockNode::AddInstruction(std::shared_ptr<IRInstructionNode> instruction) {
  if (dynamic_cast<IRAllocateInstructionNode *>(instruction.get()) != nullptr) {
    instructions_.emplace_front(instruction);
  } else if (!end_) {
    instructions_.emplace_back(instruction);
    if (dynamic_cast<IRJumpInstructionNode *>(instruction.get()) != nullptr
      || dynamic_cast<IRReturnInstructionNode *>(instruction.get()) != nullptr
      || dynamic_cast<IRBranchInstructionNode *>(instruction.get()) != nullptr) {
      end_ = true;
    }
  }
}

uint32_t IRBlockNode::GetID() const { return id_; }

IRParameterNode::IRParameterNode(const std::string &type, const std::string &name) : type_(type), name_(name) {}

IRFunctionNode::IRFunctionNode(const std::string &type, const std::string &name) : type_(type), name_(name) {}

void IRRootNode::AddStruct(std::shared_ptr<IRStructNode> struct_node) {
  structs_.emplace_back(struct_node);
}

void IRRootNode::AddFunction(std::shared_ptr<IRFunctionNode> function_node) {
  functions_.emplace_back(function_node);
}

void IRStructNode::Accept(IRVisitorBase *visitor) {
  visitor->Visit(this);
}

void IRArithmeticInstructionNode::Accept(IRVisitorBase *visitor) {
  visitor->Visit(this);
}

void IRNegationInstructionNode::Accept(IRVisitorBase *visitor) {
  visitor->Visit(this);
}

void IRBranchInstructionNode::Accept(IRVisitorBase *visitor) {
  visitor->Visit(this);
}

void IRJumpInstructionNode::Accept(IRVisitorBase *visitor) {
  visitor->Visit(this);
}

void IRReturnInstructionNode::Accept(IRVisitorBase *visitor) {
  visitor->Visit(this);
}

void IRAllocateInstructionNode::Accept(IRVisitorBase *visitor) {
  visitor->Visit(this);
}

void IRLoadInstructionNode::Accept(IRVisitorBase *visitor) {
  visitor->Visit(this);
}

void IRStoreVariableInstructionNode::Accept(IRVisitorBase *visitor) {
  visitor->Visit(this);
}

void IRStoreConstInstructionNode::Accept(IRVisitorBase *visitor) {
  visitor->Visit(this);
}

void IRGetElementPtrInstructionNode::Accept(IRVisitorBase *visitor) {
  visitor->Visit(this);
}

void IRGetElementPtrPrimeInstructionNode::Accept(IRVisitorBase *visitor) {
  visitor->Visit(this);
}

void IRCompareInstructionNode::Accept(IRVisitorBase *visitor) {
  visitor->Visit(this);
}

void IRArgumentNode::Accept(IRVisitorBase *visitor) {
  visitor->Visit(this);
}

void IRCallInstructionNode::Accept(IRVisitorBase *visitor) {
  visitor->Visit(this);
}

void IRSelectInstructionNode::Accept(IRVisitorBase *visitor) {
  visitor->Visit(this);
}


void IRBlockNode::Accept(IRVisitorBase *visitor) {
  visitor->Visit(this);
}

void IRParameterNode::Accept(IRVisitorBase *visitor) {
  visitor->Visit(this);
}

void IRFunctionNode::AddParameter(std::shared_ptr<IRParameterNode> parameter) {
  parameters_.emplace_back(parameter);
}

void IRFunctionNode::AddBlock(std::shared_ptr<IRBlockNode> block) {
  blocks_.emplace_back(block);
}


void IRFunctionNode::Accept(IRVisitorBase *visitor) {
  visitor->Visit(this);
}

void IRRootNode::Accept(IRVisitorBase *visitor) {
  visitor->Visit(this);
}