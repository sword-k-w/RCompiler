#include <iostream>
#include <cassert>
#include "IR_visitor/assembly_generator/assembly_generator.h"
#include "IR_visitor/memory_allocator/memory_allocator.h"
#include "IR/struct_map.h"
#include "IR/function_map.h"
#include "codegen/register.h"
#include "codegen/instruction.h"

AssemblyGenerator::AssemblyGenerator(const std::string &builtin_begin, const std::string &builtin_end, std::ostream &os) :
  builtin_begin_(builtin_begin), builtin_end_(builtin_end),os_(os) {}

std::pair<StorageType, uint32_t> AssemblyGenerator::GetVariableAddress(const std::string &name) {
  if (name[0] != '%') { // name must be a number
    return std::make_pair(kConst, 0);
  }
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
  auto getep = dynamic_cast<IRGetElementPtrPrimeInstructionNode *>(instruction);
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
  auto para = dynamic_cast<IRParameterNode *>(instruction);
  if (para != nullptr) {
    return std::make_pair(para->storage_type_, para->address_);
  }
  std::cerr << "Error: unexpected variable!\n";
  exit(-1);
}

void AssemblyGenerator::TransferToTreg(uint32_t address, uint32_t reg_id) {
  PrintMem(os_, "lw", "t" + std::to_string(reg_id), "sp", current_stack_ - address);
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
  if (type == kConst) {
    os_ << "\tli\tt" << reg_id << "\t" << name << '\n';
    return "t" + std::to_string(reg_id);
  }
  return "x" + std::to_string(address);
}

void AssemblyGenerator::VariableForceToReg(const std::string &name, const std::string &reg) {
  auto [type, address] = GetVariableAddress(name);
  if (type == kMemory) {
    PrintMem(os_, "lw", reg, "sp", current_stack_ - address);
  } else if (type == kConst) {
    os_ << "\tli\t" << reg << ", " << name << '\n';
  } else {
    if (!SameRegister(address, reg)) {
      os_ << "\tmv\t" << address << ", " << reg << '\n';
    }
  }
}

void AssemblyGenerator::RegToVariable(StorageType storage_type, uint32_t address, const std::string &reg) {
  assert(storage_type != kConst);
  if (storage_type == kMemory) {
    PrintMem(os_, "sw", reg, "sp", current_stack_ - address);
  } else if (!SameRegister(address, reg)) {
    os_ << "\tmv\t" << address << ", " << reg << '\n';
  }
}

void AssemblyGenerator::SaveRegister() {
  // save a0~a7 and ra
  // Remember to think about the argument of call needs values in a0~a7, but they may be covered.
  // The correct way is to special judge a0~a7 and use the value in the memory.
  for (uint32_t i = 0; i < current_a_reg_used_; ++i) {
    PrintMem(os_, "sw", "a" + std::to_string(i), "sp", current_stack_ - 48 - 4 * i);
  }
  PrintMem(os_, "sw", "ra", "sp", current_stack_ - 48 - 32);
}

void AssemblyGenerator::RestoreRegister() {
  // restore a0~a7 and ra
  for (uint32_t i = 0; i < current_a_reg_used_; ++i) {
    PrintMem(os_, "lw", "a" + std::to_string(i), "sp", current_stack_ - 48 - 4 * i);
  }
  PrintMem(os_, "lw", "ra", "sp", current_stack_ - 48 - 32);
}

void AssemblyGenerator::Visit(IRArrayNode *node) {}

void AssemblyGenerator::Visit(IRStructNode *node) {}

void AssemblyGenerator::Visit(IRArithmeticInstructionNode *node) {
  os_ << "\t # Arithmetic Instruction " << node->result_ << '\n';
  auto rs1 = VariableToReg(node->operand1_, 0);
  auto rs2 = VariableToReg(node->operand2_, 1);
  auto rd = GetResultReg(node->storage_type_, node->address_, 2);
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

void AssemblyGenerator::Visit(IRNegationInstructionNode *node) {
  os_ << "\t# Negation Instruction " << node->result_ << '\n';
  auto rs = VariableToReg(node->operand_, 0);
  auto rd = GetResultReg(node->storage_type_, node->address_, 1);
  if (node->is_minus_) {
    os_ << "\tneg\t" << rd << ", " << rs << '\n';
  } else {
    if (node->type_ == "i1") {
      PrintIA(os_, "xori", rd, rs, 1);
    } else {
      os_ << "\tnot\t" << rd << ", " << rs << '\n';
    }
  }
  RegToVariable(node->storage_type_, node->address_, rd);
}

void AssemblyGenerator::Visit(IRBranchInstructionNode *node) {
  os_ << "\t# Branch Instruction\n";
  auto rs = VariableToReg(node->condition_, 0);
  os_ << "\tbnez\t" << rs << ", " << ".L" << current_func_name_ << "_" << node->true_branch_ << '\n';
  os_ << "\tj " << ".L" << current_func_name_ << "_" << node->false_branch_ << '\n';
}

void AssemblyGenerator::Visit(IRJumpInstructionNode *node) {
  os_ << "\t# Jump Instruction\n";
  os_ << "\tj " << ".L" << current_func_name_ << "_" << node->destination_ << '\n';
}

void AssemblyGenerator::Visit(IRReturnInstructionNode *node) {
  os_ << "\t# Return Instruction\n";
  if (!node->type_->IsEmpty()) {
    VariableForceToReg(node->name_, "a0");
  }
  // not activated yet. If s registers are used, [remember to enable it]
  // for (uint32_t i = 0; i < 12; ++i) {
  //   PrintMem(os_, "lw", "s" + std::to_string(i), "sp", current_stack_ - 4 * (i + 1));
  // }
  PrintIA(os_, "addi", "sp", "sp", current_stack_);
  os_ << "\tret\n";
}

void AssemblyGenerator::Visit(IRAllocateInstructionNode *node) {
  os_ << "\t# Allocate Instruction " << node->result_ << '\n';
  auto rd = GetResultReg(node->storage_type_, node->address_, 1);
  PrintIA(os_, "addi", rd, "sp", current_stack_ - node->inner_address_);
  RegToVariable(node->storage_type_, node->address_, rd);
}

void AssemblyGenerator::Visit(IRLoadInstructionNode *node) {
  os_ << "\t# Load Instruction " << node->result_ << '\n';
  auto ptr_reg = VariableToReg(node->pointer_, 0);
  if (node->storage_type_ == kRegister) {
    PrintMem(os_, "lw", "x" + std::to_string(node->address_), ptr_reg, 0);
  } else {
    SaveRegister();
    PrintIA(os_, "addi", "a0", "sp", current_stack_ - node->address_);
    if (!SameRegister(11, ptr_reg)) {
      os_ << "\tmv\ta1, " << ptr_reg << '\n';
    }
    os_ << "\tli\ta2, " << node->type_->allocated_size_ << '\n';
    os_ << "\tcall\tbuiltin_memcpy\n";
    RestoreRegister();
  }
}

void AssemblyGenerator::Visit(IRStoreVariableInstructionNode *node) {
  os_ << "\t# Store Instruction\n";
  auto ptr_reg = VariableToReg(node->pointer_, 0);
  auto [type, address] = GetVariableAddress(node->value_);
  if (type == kRegister) {
    PrintMem(os_, "sw", "x" + std::to_string(address), ptr_reg, 0);
  } else {
    SaveRegister();
    if (!SameRegister(10, ptr_reg)) {
      os_ << "\tmv\ta0, " << ptr_reg << '\n';
    }
    PrintIA(os_, "addi", "a1", "sp", current_stack_ - address);
    os_ << "\tli\ta2, " << node->type_->allocated_size_ << '\n';
    os_ << "\tcall\tbuiltin_memcpy\n";
    RestoreRegister();
  }
}

void AssemblyGenerator::Visit(IRStoreConstInstructionNode *node) {
  os_ << "\t# Store Instruction\n";
  auto ptr_reg = VariableToReg(node->pointer_, 0);
  os_ << "\tli\tt1, " << node->value_ << '\n';
  PrintMem(os_, "sw", "t1", ptr_reg, 0);
}

void AssemblyGenerator::Visit(IRGetElementPtrInstructionNode *node) {
  os_ << "\t# GetElementPtr Instruction " << node->result_ << '\n';
  auto ptr_reg = VariableToReg(node->ptrval_, 0);
  auto rd = GetResultReg(node->storage_type_, node->address_, 1);
  uint32_t offset = 0;
  if (node->type_->length_.empty()) {
    uint32_t align = 1;
    auto struct_node = StructMap::Instance().Query(node->type_->base_type_);
    for (uint32_t i = 0; i < node->index_; ++i) {
      if (align == 1 && struct_node->members_[i]->align_ == 4) {
        align = 4;
        offset = Align4(offset);
      }
      offset += struct_node->members_[i]->allocated_size_;
    }
  } else {
    offset = node->type_->allocated_size_ / node->type_->length_[0] * node->index_;
  }
  PrintIA(os_, "addi", rd, ptr_reg, offset);
  RegToVariable(node->storage_type_, node->address_, rd);
}

void AssemblyGenerator::Visit(IRGetElementPtrPrimeInstructionNode *node) {
  os_ << "\t# GetElementPtr Instruction " << node->result_ << '\n';
  auto ptr_reg = VariableToReg(node->ptrval_, 0);
  auto index_reg = VariableToReg(node->index_, 2);
  auto rd = GetResultReg(node->storage_type_, node->address_, 1);
  assert(!node->type_->length_.empty());
  os_ << "\tli\tt1, " << node->type_->allocated_size_ / node->type_->length_[0] << '\n';
  os_ << "\tmul\tt2, t1, t2\n";
  os_ << "\tadd " << rd << ", " << ptr_reg << ", t2\n";
  RegToVariable(node->storage_type_, node->address_, rd);
}

void AssemblyGenerator::Visit(IRCompareInstructionNode *node) {
  os_ << "\t# Compare Instruction " << node->result_ << '\n';
  auto rs1 = VariableToReg(node->operand1_, 0);
  auto rs2 = VariableToReg(node->operand2_, 1);
  auto rd = GetResultReg(node->storage_type_, node->address_, 2);
  if (node->op_ == IRCompareInstructionNode::kEq) {
    os_ << "\tslt t3, " << rs1 << ", " << rs2 << '\n';
    os_ << "\tslt t4, " << rs2 << ", " << rs1 << '\n';
    os_ << "\tor\tt3, t3, t4\n";
    PrintIA(os_, "xori", rd, "t3", 1);
  } else if (node->op_ == IRCompareInstructionNode::kNe) {
    os_ << "\tslt t3, " << rs1 << ", " << rs2 << '\n';
    os_ << "\tslt t4, " << rs2 << ", " << rs1 << '\n';
    os_ << "\tor\t" << rd << ", t3, t4\n";
  } else if (node->op_ == IRCompareInstructionNode::kUgt) {
    os_ << "\tsltu\t" << rd << ", " << rs2 << ", " << rs1 << '\n';
  } else if (node->op_ == IRCompareInstructionNode::kUge) {
    os_ << "\tsltu\tt3, " << rs1 << ", " << rs2 << '\n';
    PrintIA(os_, "xori", rd, "t3", 1);
  } else if (node->op_ == IRCompareInstructionNode::kUlt) {
    os_ << "\tsltu\t" << rd << ", " << rs1 << ", " << rs2 << '\n';
  } else if (node->op_ == IRCompareInstructionNode::kUle) {
    os_ << "\tsltu\tt3, " << rs2 << ", " << rs1 << '\n';
    PrintIA(os_, "xori", rd, "t3", 1);
  } else if (node->op_ == IRCompareInstructionNode::kSgt) {
    os_ << "\tslt\t" << rd << ", " << rs2 << ", " << rs1 << '\n';
  } else if (node->op_ == IRCompareInstructionNode::kSge) {
    os_ << "\tslt\tt3, " << rs1 << ", " << rs2 << '\n';
    PrintIA(os_, "xori", rd, "t3", 1);
  } else if (node->op_ == IRCompareInstructionNode::kSlt) {
    os_ << "\tslt\t" << rd << ", " << rs1 << ", " << rs2 << '\n';
  } else {
    os_ << "\tslt\tt3, " << rs2 << ", " << rs1 << '\n';
    PrintIA(os_, "xori", rd, "t3", 1);
  }
  RegToVariable(node->storage_type_, node->address_, rd);
}

void AssemblyGenerator::Visit(IRArgumentNode *node) {}

void AssemblyGenerator::Visit(IRCallInstructionNode *node) {
  os_ << "\t# Call Instruction " << node->function_name_ << '\n';
  auto function_node = FunctionMap::Instance().Query(node->function_name_);
  auto size = node->arguments_.size();

  SaveRegister();

  // operate the memory to memory in advance, using builtin_memcpy
  for (uint32_t i = 0; i < size; ++i) {
    auto para = function_node->parameters_[i];
    auto [type, address] = GetVariableAddress(node->arguments_[i]->value_);
    if (type == kMemory && para->storage_type_ == kMemory) {
      PrintIA(os_, "addi", "a0", "sp", current_stack_ - address);
      PrintIA(os_, "addi", "a1", "sp", -static_cast<int32_t>(para->address_));
      os_ << "\tli\ta2, " << para->type_->allocated_size_ << '\n';
      os_ << "\tcall\tbuiltin_memcpy\n";
    }
  }

  for (uint32_t i = 0; i < size; ++i) {
    auto para = function_node->parameters_[i];
    auto [type, address] = GetVariableAddress(node->arguments_[i]->value_);
    if (type == kConst) {
      if (para->storage_type_ == kRegister) {
        os_ << "\tli\tx" << para->address_ << ", " << node->arguments_[i]->value_ << '\n';
      } else {
        os_ << "\tli\tt0, " << node->arguments_[i]->value_ << '\n';
        PrintMem(os_, "sw", "t0", "sp", -static_cast<int32_t>(para->address_));
      }
    } else if (type == kRegister) {
      if (address >= 10 && address < 18) { // The correct values are in the memory.
        if (para->storage_type_ == kRegister) {
          PrintMem(os_, "lw", "x" + std::to_string(para->address_), "sp", current_stack_ - 48 - 4 * (address - 10));
        } else {
          PrintMem(os_, "lw", "t0", "sp", current_stack_ - 48 - 4 * (address - 10));
          PrintMem(os_, "sw", "t0", "sp", -static_cast<int32_t>(para->address_));
        }
      } else {
        if (para->storage_type_ == kRegister) {
          if (para->address_ != address) {
            os_ << "\tmv\tx" << address << ", x" << para->address_ << '\n';
          }
        } else {
          PrintMem(os_, "sw", "x" + std::to_string(address), "sp", -static_cast<int32_t>(para->address_));
        }
      }
    } else {
      if (para->storage_type_ == kRegister) {
        PrintMem(os_, "lw", "x" + std::to_string(para->address_), "sp", current_stack_ - address);
      }
    }
  }
  os_ << "\tcall\t" << node->function_name_ << '\n';
  if (!node->result_type_->IsEmpty()) {
    RegToVariable(node->storage_type_, node->address_, "a0");
  }

  RestoreRegister();
}

void AssemblyGenerator::Visit(IRSelectInstructionNode *node) {
  os_ << "\t# Select Instruction " << node->result_ << '\n';
  auto rs = VariableToReg(node->cond_, 0);
  auto rd = GetResultReg(node->storage_type_, node->address_, 1);
  PrintIA(os_, "slti", rd, rs, 1);
  PrintIA(os_, "xori", rd, rd, 1);
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

  PrintIA(os_, "addi", "sp", "sp", -static_cast<int32_t>(node->stack_size_)); // reserve stack space

  current_stack_ = node->stack_size_;
  current_func_name_ = node->name_;
  current_a_reg_used_ = node->a_reg_used_cnt_;
  current_variables_ = &node->variables_;

  // save regs
  // not activated yet. If s registers are used, [remember to enable it]
  // for (uint32_t i = 0; i < 12; ++i) {
  //   PrintMem(os_, "sw", "s" + std::to_string(i), "sp", current_stack_ - 4 * (i + 1));
  // }

  // blocks
  for (auto &block : node->blocks_) {
    block->Accept(this);
  }
}

void AssemblyGenerator::Visit(IRRootNode *node) {
  os_ << builtin_begin_ << '\n';
  for (auto &function_node : node->functions_) {
    function_node->Accept(this);
  }
  os_ << builtin_end_ << '\n';
}