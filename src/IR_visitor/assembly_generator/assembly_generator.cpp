#include <iostream>
#include <cassert>
#include "IR_visitor/assembly_generator/assembly_generator.h"
#include "IR_visitor/memory_allocator/memory_allocator.h"
#include "IR/struct_map.h"
#include "IR/function_map.h"
#include "codegen/register.h"

AssemblyGenerator::AssemblyGenerator(const std::string &builtin, std::ostream &os) : builtin_(builtin), os_(os) {}

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
  if (type == kConst) {
    os_ << "\tli\tt" << reg_id << "\t" << name << '\n';
    return "t" + std::to_string(reg_id);
  }
  return "x" + std::to_string(address);
}

void AssemblyGenerator::VariableForceToReg(const std::string &name, const std::string &reg) {
  auto [type, address] = GetVariableAddress(name);
  if (type == kMemory) {
    os_ << "\tlw\t" << reg << ", " << current_stack_ - address << "(sp)\n";
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
    os_ << "\tsw\t" << reg << ", " << current_stack_ - address << "(sp)\n";
  } else if (!SameRegister(address, reg)) {
    os_ << "\tmv\t" << address << ", " << reg << '\n';
  }
}

void AssemblyGenerator::Visit(IRArrayNode *node) {}

void AssemblyGenerator::Visit(IRStructNode *node) {}

void AssemblyGenerator::Visit(IRArithmeticInstructionNode *node) {
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
  auto rs = VariableToReg(node->operand_, 0);
  auto rd = GetResultReg(node->storage_type_, node->address_, 1);
  if (node->is_minus_) {
    os_ << "\tneg\t" << rd << ", " << rs << '\n';
  } else {
    if (node->type_ == "i1") {
      os_ << "\txori\t" << rd << ", " << rs << ", 1\n";
    } else {
      os_ << "\tnot\t" << rd << ", " << rs << '\n';
    }
  }
  RegToVariable(node->storage_type_, node->address_, rd);
}

void AssemblyGenerator::Visit(IRBranchInstructionNode *node) {
  auto rs = VariableToReg(node->condition_, 0);
  os_ << "\tbnez\t" << rs << ", " << ".L" << current_func_name_ << "_" << node->true_branch_ << '\n';
  os_ << "\tj " << ".L" << current_func_name_ << "_" << node->false_branch_ << '\n';
}

void AssemblyGenerator::Visit(IRJumpInstructionNode *node) {
  os_ << "\tj " << ".L" << current_func_name_ << "_" << node->destination_ << '\n';
}

void AssemblyGenerator::Visit(IRReturnInstructionNode *node) {
  if (!node->type_->IsEmpty()) {
    VariableForceToReg(node->name_, "a0");
  }
  for (uint32_t i = 0; i < 12; ++i) {
    os_ << "\tlw\ts" << i << ", " << current_stack_ - 4 * (i + 1) << "(sp)\n";
  }
  os_ << "\taddi\tsp, sp, " << current_stack_ << '\n';
  os_ << "\tret\n";
}

void AssemblyGenerator::Visit(IRAllocateInstructionNode *node) {
  auto rd = GetResultReg(node->storage_type_, node->address_, 1);
  os_ << "\taddi, " << rd << ", sp, " << current_stack_ - node->inner_address_ << '\n';
  RegToVariable(node->storage_type_, node->address_, rd);
}

void AssemblyGenerator::Visit(IRLoadInstructionNode *node) {
  auto ptr_reg = VariableToReg(node->pointer_, 0);
  if (node->storage_type_ == kRegister) {
    os_ << "\tlw\tx" << node->address_ << ", " << "0(" << ptr_reg << ")\n";
  } else {
    uint32_t size = (node->type_->allocated_size_ + 3) / 4;
    for (uint32_t i = 0; i < size; ++i) {
      os_ << "\tlw\tt1, " << i * 4 << "(" << ptr_reg << ")\n";
      os_ << "\tsw\tt1, " << current_stack_ - node->address_ + i * 4 << "(sp)\n";
    }
  }
}

void AssemblyGenerator::Visit(IRStoreVariableInstructionNode *node) {
  auto ptr_reg = VariableToReg(node->pointer_, 0);
  auto [type, address] = GetVariableAddress(node->value_);
  if (type == kRegister) {
    os_ << "\tsw\tx" << address << ", 0(" << ptr_reg << ")\n";
  } else {
    uint32_t size = (node->type_->allocated_size_ + 3) / 4;
    for (uint32_t i = 0; i < size; ++i) {
      os_ << "\tlw\tt1, " << current_stack_ - node->address_ + i * 4 << "(sp)\n";
      os_ << "\tsw\tt1, " << i * 4 << "(" << ptr_reg << ")\n";
    }
  }
}

void AssemblyGenerator::Visit(IRStoreConstInstructionNode *node) {
  auto ptr_reg = VariableToReg(node->pointer_, 0);
  os_ << "\tli\tt1, " << node->value_ << '\n';
  os_ << "\tsw\tt1, " << "0(" << ptr_reg << ")\n";
}

void AssemblyGenerator::Visit(IRGetElementPtrInstructionNode *node) {
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
  os_ << "\taddi\t" << rd << ", " << ptr_reg << ", " << offset << '\n';
  RegToVariable(node->storage_type_, node->address_, rd);
}

void AssemblyGenerator::Visit(IRGetElementPtrPrimeInstructionNode *node) {
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
  auto rs1 = VariableToReg(node->operand1_, 0);
  auto rs2 = VariableToReg(node->operand2_, 1);
  auto rd = GetResultReg(node->storage_type_, node->address_, 2);
  if (node->op_ == IRCompareInstructionNode::kEq) {
    os_ << "\tslt t3, " << rs1 << ", " << rs2 << '\n';
    os_ << "\tslt t4, " << rs2 << ", " << rs1 << '\n';
    os_ << "\tor\tt3, t3, t4\n";
    os_ << "\txori\t" << rd << ", t3, 1\n";
  } else if (node->op_ == IRCompareInstructionNode::kNe) {
    os_ << "\tslt t3, " << rs1 << ", " << rs2 << '\n';
    os_ << "\tslt t4, " << rs2 << ", " << rs1 << '\n';
    os_ << "\tor\t" << rd << ", t3, t4\n";
  } else if (node->op_ == IRCompareInstructionNode::kUgt) {
    os_ << "\tsltu\t" << rd << ", " << rs2 << ", " << rs1 << '\n';
  } else if (node->op_ == IRCompareInstructionNode::kUge) {
    os_ << "\tsltu\tt3, " << rs1 << ", " << rs2 << '\n';
    os_ << "\txori\t" << rd << ", t3, 1\n";
  } else if (node->op_ == IRCompareInstructionNode::kUlt) {
    os_ << "\tsltu\t" << rd << ", " << rs1 << ", " << rs2 << '\n';
  } else if (node->op_ == IRCompareInstructionNode::kUle) {
    os_ << "\tsltu\tt3, " << rs2 << ", " << rs1 << '\n';
    os_ << "\txori\t" << rd << ", t3, 1\n";
  } else if (node->op_ == IRCompareInstructionNode::kSgt) {
    os_ << "\tslt\t" << rd << ", " << rs2 << ", " << rs1 << '\n';
  } else if (node->op_ == IRCompareInstructionNode::kSge) {
    os_ << "\tslt\tt3, " << rs1 << ", " << rs2 << '\n';
    os_ << "\txori\t" << rd << ", t3, 1\n";
  } else if (node->op_ == IRCompareInstructionNode::kSlt) {
    os_ << "\tslt\t" << rd << ", " << rs1 << ", " << rs2 << '\n';
  } else {
    os_ << "\tslt\tt3, " << rs2 << ", " << rs1 << '\n';
    os_ << "\txori\t" << rd << ", t3, 1\n";
  }
  RegToVariable(node->storage_type_, node->address_, rd);
}

void AssemblyGenerator::Visit(IRArgumentNode *node) {}

void AssemblyGenerator::Visit(IRCallInstructionNode *node) {
  auto function_node = FunctionMap::Instance().Query(node->function_name_);
  auto size = node->arguments_.size();

  // save a0~a7 and ra
  // Remember to think about the argument of call needs values in a0~a7, but they may be covered.
  // The correct way is to special judge a0~a7 and use the value in the memory.
  for (uint32_t i = 0; i < 8; ++i) {
    os_ << "\tsw\ta" << i << ", " << current_stack_ - 48 - 4 * i << "(sp)\n";
  }
  for (uint32_t i = 0; i < size; ++i) {
    auto para = function_node->parameters_[i];
    auto [type, address] = GetVariableAddress(node->arguments_[i]->value_);
    if (type == kConst) {
      if (para->storage_type_ == kRegister) {
        os_ << "\tli\tx" << para->address_ << ", " << node->arguments_[i]->value_ << '\n';
      } else {
        os_ << "\tli\tt0, " << node->arguments_[i]->value_ << '\n';
        os_ << "\tsw\tt0, -" << para->address_ << "(sp)" << '\n';
      }
    } else if (type == kRegister) {
      if (address >= 10 && address < 18) { // The correct values are in the memory.
        if (para->storage_type_ == kRegister) {
          os_ << "\tlw\tx" << para->address_ << ", " << current_stack_ - 48 - 4 * (address - 10) << "(sp)\n";
        } else {
          os_ << "\tlw\tt0, " << current_stack_ - 48 - 4 * (address - 10) << "(sp)\n";
          os_ << "\tsw\tt0, -" << para->address_ << "(sp)\n";
        }
      } else {
        if (para->storage_type_ == kRegister) {
          if (para->address_ != address) {
            os_ << "\tmv\tx" << address << ", x" << para->address_ << '\n';
          }
        } else {
          os_ << "\tsw\tx" << address << ", -" << para->address_ << "(sp)\n";
        }
      }
    } else {
      if (para->storage_type_ == kRegister) {
        os_ << "\tlw\tx" << para->address_ << ", " << current_stack_ - address << "(sp)\n";
      } else {
        uint32_t chunk_size = (para->type_->allocated_size_ + 3) / 4;
        for (uint32_t i = 0; i < chunk_size; ++i) {
          os_ << "\tlw\tt0, " << current_stack_ - address + i * 4 << "(sp)\n";
          os_ << "\tsw\tt0, -" << para->address_ - 4 * i << "(sp)\n";
        }
      }
    }
  }
  os_ << "\tcall\t" << node->function_name_ << '\n';
  if (!node->result_type_->IsEmpty()) {
    RegToVariable(node->storage_type_, node->address_, "a0");
  }
}

void AssemblyGenerator::Visit(IRSelectInstructionNode *node) {
  auto rs = VariableToReg(node->cond_, 0);
  auto rd = GetResultReg(node->storage_type_, node->address_, 1);
  os_ << "\tslti\t" << rd << ", " << rs << ", 1\n";
  os_ << "\txori\t" << rd << ", " << rd << ", 1\n";
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

  os_ << "\taddi\tsp, sp, -" << node->stack_size_ << '\n'; // reserve stack space

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
  os_ << builtin_ << '\n';
  for (auto &function_node : node->functions_) {
    function_node->Accept(this);
  }
}