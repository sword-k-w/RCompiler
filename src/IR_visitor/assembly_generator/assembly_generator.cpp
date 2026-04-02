#include "IR_visitor/assembly_generator/assembly_generator.h"
#include <iostream>

AssemblyGenerator::AssemblyGenerator(const std::string &builtin, std::ostream &os) : builtin_(builtin), os_(os) {}

std::pair<StorageType, uint32_t> AssemblyGenerator::GetVariableAddress(const std::string &name) {
  auto instruction = (*current_variables_)[name];
  auto arith = dynamic_cast<IRArithmeticInstructionNode *>(instruction);
  if (arith != nullptr) {
    return std::make_pair(arith->storage_type_, arith->address_);
  }
  auto neg = dynamic_cast<IRNegationInstructionNode *>(instruction);
  if (neg != nullptr) {
    return std::make_pair(neg->storage_type_, neg->address_);
  }
  auto alloc = dynamic_cast<IRAllocateInstructionNode *>(instruction);
  if (alloc != nullptr) {
    return std::make_pair(alloc->storage_type_, alloc->address_);
  }
  auto load = dynamic_cast<IRLoadInstructionNode *>(instruction);
  if (load != nullptr) {
    return std::make_pair(load->storage_type_, load->address_);
  }
  auto gete = dynamic_cast<IRGetElementPtrInstructionNode *>(instruction);
  if (gete != nullptr) {
    return std::make_pair(gete->storage_type_, gete->address_);
  }
  auto getep = dynamic_cast<IRGetElementPtrInstructionNode *>(instruction);
  if (getep != nullptr) {
    return std::make_pair(getep->storage_type_, getep->address_);
  }
  auto comp = dynamic_cast<IRCompareInstructionNode *>(instruction);
  if (comp != nullptr) {
    return std::make_pair(comp->storage_type_, comp->address_);
  }
  auto call = dynamic_cast<IRCallInstructionNode *>(instruction);
  if (call != nullptr) {
    return std::make_pair(call->storage_type_, call->address_);
  }
  auto sele = dynamic_cast<IRSelectInstructionNode *>(instruction);
  if (sele != nullptr) {
    return std::make_pair(sele->storage_type_, sele->address_);
  }
  std::cerr << "Error: unexpected variable!\n";
  exit(-1);
}

void AssemblyGenerator::TransferToTreg(uint32_t address, uint32_t reg_id) {
  os_ << "\tlw\tt" << reg_id << ", " << current_stack_ - address << "(sp)\n";
}

std::string AssemblyGenerator::GetResultReg(StorageType storage_type, uint32_t address, uint32_t reg_id) {
  if (storage_type == kMemory) {
    return "t" + std::to_string(reg_id);
  }
  return "x" + std::to_string(address);
}

std::string AssemblyGenerator::VariableToReg(const std::string &name, uint32_t reg_id) {
  auto [type, address] = GetVariableAddress(name);
  if (type == kMemory) {
    TransferToTreg(address, reg_id);
    return "t" + std::to_string(reg_id);
  }
  return "x" + std::to_string(address);
}

void AssemblyGenerator::RegToVariable(StorageType storage_type, uint32_t address, const std::string &reg) {
  if (storage_type == kMemory) {
    os_ << "\tsw\t" << reg << ", " << current_stack_ - address << "(sp)\n";
  }
}

void AssemblyGenerator::Visit(IRArrayNode *node) {}

void AssemblyGenerator::Visit(IRStructNode *node) {}

void AssemblyGenerator::Visit(IRArithmeticInstructionNode *node) {
  auto rs1 = VariableToReg(node->operand1_, 0);
  auto rs2 = VariableToReg(node->operand2_, 1);
  std::string rd = GetResultReg(node->storage_type_, node->address_, 2);
  os_ << "\t";
  if (node->op_ == "+") {
    os_ << "add";
  } else if (node->op_ == "-") {
    os_ << "sub";
  } else if (node->op_ == "*") {
    os_ << "mul";
  } else if (node->op_ == "/") {
    os_ << "div";
    if (node->is_unsigned_) {
      os_ << "u";
    }
  } else if (node->op_ == "%") {
    os_ << "rem";
    if (node->is_unsigned_) {
      os_ << "u";
    }
  } else if (node->op_ == "<<") {
    os_ << "sll";
  } else if (node->op_ == ">>") {
    os_ << "sra";
  } else if (node->op_ == "&") {
    os_ << "and";
  } else if (node->op_ == "|") {
    os_ << "or";
  } else if (node->op_ == "^") {
    os_ << "xor";
  } else {
    std::cerr << "Error! : unexpected op " << node->op_ << '\n';
    exit(-1);
  }
  os_ << "\t" << rd << ", " << rs1 << ", " << rs2 << '\n';
  RegToVariable(node->storage_type_, node->address_, rd);
}

void AssemblyGenerator::Visit(IRBlockNode *node) {
  if (node->id_ != 0) {
    os_ << ".L" << current_func_name_ << "_" << node->id_ << ":\n";
  }
  for (auto &instruction : node->instructions_) {
    instruction->Accept(this);
  }
}

void AssemblyGenerator::Visit(IRParameterNode *node) {}

void AssemblyGenerator::Visit(IRFunctionNode *node) {
  os_ << "\t.globl " << node->name_ << "        # -- Begin function " << node->name_ << '\n';
  os_ << "\t.p2align 2\n";
  os_ << "\t.type " << node->name_ << ",@function\n";
  os_ << node->name_ << ":        # @" << node->name_ << '\n';

  os_ << "\taddi\tsp, sp, " << -node->stack_size_ << '\n'; // reserve stack space

  current_stack_ = node->stack_size_;
  current_func_name_ = node->name_;
  current_variables_ = &node->variables_;

  // save regs
  for (uint32_t i = 0; i < 12; ++i) {
    os_ << "\tsw\ts" << i << ", " << current_stack_ - 4 * (i + 1) << "(sp)\n";
  }

  // blocks
  for (auto &block : node->blocks_) {
    block->Accept(this);
  }
}

void AssemblyGenerator::Visit(IRRootNode *node) {
  for (auto &function_node : node->functions_) {
    function_node->Accept(this);
  }
}