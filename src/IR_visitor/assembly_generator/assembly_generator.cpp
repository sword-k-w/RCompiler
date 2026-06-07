#include <iostream>
#include <cassert>
#include "IR_visitor/assembly_generator/assembly_generator.h"
#include "IR_visitor/memory_allocator/memory_allocator.h"
#include "IR/struct_map.h"
#include "IR/function_map.h"
#include "codegen/register.h"
#include "codegen/instruction.h"

AssemblyGenerator::AssemblyGenerator(const std::string &builtin_begin, std::ostream &os) :
  builtin_begin_(builtin_begin), os_(os) {}

std::pair<StorageType, uint32_t> AssemblyGenerator::GetVariableAddress(const std::string &name) {
  if (name[0] != '%') {
    return std::make_pair(kConst, 0);
  }
  auto it = variable_storage_->find(name);
  if (it == variable_storage_->end()) {
    std::cerr << "Error: variable " << name << " has no storage assigned\n";
    exit(-1);
  }
  return it->second;
}

void AssemblyGenerator::TransferToTreg(uint32_t address, uint32_t reg_id, const std::string &val_type) {
  PrintMem(os_, LoadStoreType(val_type).first, "t" + std::to_string(reg_id), "sp", current_stack_ - address);
}

std::string AssemblyGenerator::GetResultReg(StorageType storage_type, uint32_t address, uint32_t reg_id) {
  if (storage_type == kMemory) {
    return "t" + std::to_string(reg_id);
  }
  return "x" + std::to_string(address);
}

std::string AssemblyGenerator::VariableToReg(const std::string &name, uint32_t reg_id, const std::string &val_type) {
  auto [type, address] = GetVariableAddress(name);
  if (type == kMemory) {
    TransferToTreg(address, reg_id, val_type);
    return "t" + std::to_string(reg_id);
  }
  if (type == kConst) {
    os_ << "\tli\tt" << reg_id << ",\t" << name << '\n';
    return "t" + std::to_string(reg_id);
  }
  return "x" + std::to_string(address);
}

void AssemblyGenerator::VariableForceToReg(const std::string &name, const std::string &reg, const std::string &val_type) {
  auto [type, address] = GetVariableAddress(name);
  if (type == kMemory) {
    PrintMem(os_, LoadStoreType(val_type).first, reg, "sp", current_stack_ - address);
  } else if (type == kConst) {
    os_ << "\tli\t" << reg << ", " << name << '\n';
  } else {
    if (!SameRegister(address, reg)) {
      os_ << "\tmv\t" << reg << ", x" << address << '\n';
    }
  }
}

void AssemblyGenerator::RegToVariable(StorageType storage_type, uint32_t address, const std::string &reg, const std::string &val_type) {
  assert(storage_type != kConst);
  if (storage_type == kMemory) {
    PrintMem(os_, LoadStoreType(val_type).second, reg, "sp", current_stack_ - address);
  } else if (!SameRegister(address, reg)) {
    os_ << "\tmv\tx" << address << ", " << reg << '\n';
  }
}

void AssemblyGenerator::SaveRegister() {
  // Save a0~aN, ra, and t1 at the top of the frame (within the
  // 64-byte reserved area).  s-regs live at the bottom of the frame
  // and are handled by the prologue/epilogue.
  for (uint32_t i = 0; i < current_a_reg_used_; ++i) {
    PrintMem(os_, "sd", "a" + std::to_string(i), "sp", current_stack_ - 56 - 8 * i);
  }
  PrintMem(os_, "sd", "ra", "sp", current_stack_ - 56 - 64);
  PrintMem(os_, "sd", "t1", "sp", current_stack_ - 8);
}

void AssemblyGenerator::RestoreRegister() {
  for (uint32_t i = 0; i < current_a_reg_used_; ++i) {
    PrintMem(os_, "ld", "a" + std::to_string(i), "sp", current_stack_ - 56 - 8 * i);
  }
  PrintMem(os_, "ld", "ra", "sp", current_stack_ - 56 - 64);
  PrintMem(os_, "ld", "t1", "sp", current_stack_ - 8);
}

void AssemblyGenerator::DataMove(const std::string &from, StorageType to_type, uint32_t to_address, std::shared_ptr<IRArrayNode> type) {
  auto [from_type, from_address] = GetVariableAddress(from);
  if (from_type == kConst) {
    if (to_type == kRegister) {
      os_ << "\tli\tx" << to_address << ", " << from << '\n';
    } else {
      os_ << "\tli\tt0, " << from << '\n';
      auto [_, s_ins] = LoadStoreType(type->base_type_);
      PrintMem(os_, s_ins, "t0", "sp", current_stack_ - to_address);
    }
  } else if (from_type == kRegister) {
    DataMoveFromReg("x" + std::to_string(from_address), to_type, to_address, type);
  } else {
    if (to_type == kRegister) {
      auto [l_ins, _] = LoadStoreType(type->base_type_);
      PrintMem(os_, l_ins, "x" + std::to_string(to_address), "sp", current_stack_ - from_address);
    } else {
      SaveRegister();
      PrintIA(os_, "addi", "a0", "sp", current_stack_ - to_address);
      PrintIA(os_, "addi", "a1", "sp", current_stack_ - from_address);
      os_ << "\tli\ta2, " << type->allocated_size_ << '\n';
      os_ << "\tcall\tbuiltin_memcpy\n";
      RestoreRegister();
    }
  }
}

void AssemblyGenerator::DataMoveFromReg(const std::string &from, StorageType to_type, uint32_t to_address, std::shared_ptr<IRArrayNode> type) {
  auto [l_ins, s_ins] = LoadStoreType(type->base_type_);
  if (to_type == kRegister) {
    if (!SameRegister(to_address, from)) {
      os_ << "\tmv\tx" << to_address << ", " << from << '\n';
    }
  } else {
    PrintMem(os_, s_ins, from, "sp", current_stack_ - to_address);
  }
}

void AssemblyGenerator::Visit(IRArrayNode *node) {}

void AssemblyGenerator::Visit(IRStructNode *node) {}

void AssemblyGenerator::Visit(IRArithmeticInstructionNode *node) {
  os_ << "\t# Arithmetic Instruction " << node->result_ << '\n';
  auto rs1 = VariableToReg(node->operand1_, 0, node->type_);
  auto rs2 = VariableToReg(node->operand2_, 1, node->type_);
  auto rd = GetResultReg(node->storage_type_, node->address_, 2);
  os_ << "\t";
  if (node->op_ == "+") {
    os_ << "addw";
  } else if (node->op_ == "-") {
    os_ << "subw";
  } else if (node->op_ == "*") {
    os_ << "mulw";
  } else if (node->op_ == "/") {
    os_ << "div";
    if (node->is_unsigned_) {
      os_ << "u";
    }
    os_ << "w";
  } else if (node->op_ == "%") {
    os_ << "rem";
    if (node->is_unsigned_) {
      os_ << "u";
    }
    os_ << "w";
  } else if (node->op_ == "<<") {
    os_ << "sllw";
  } else if (node->op_ == ">>") {
    os_ << "sraw";
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
  RegToVariable(node->storage_type_, node->address_, rd, node->type_);
}

void AssemblyGenerator::Visit(IRNegationInstructionNode *node) {
  os_ << "\t# Negation Instruction " << node->result_ << '\n';
  auto rs = VariableToReg(node->operand_, 0, node->type_);
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
  RegToVariable(node->storage_type_, node->address_, rd, node->type_);
}

void AssemblyGenerator::Visit(IRBranchInstructionNode *node) {
  os_ << "\t# Branch Instruction\n";
  auto rs = VariableToReg(node->condition_, 0, "i1");

  std::string tmp_label = ".Lbranch..tmp." + std::to_string(branch_cnt_);
  os_ << "\tbeqz\t" << rs << ", " << tmp_label << '\n';
  os_ << "\tj " << ".L" << current_func_name_ << "_" << node->true_branch_ << '\n';
  os_ << tmp_label << ":\n";
  os_ << "\tj " << ".L" << current_func_name_ << "_" << node->false_branch_ << '\n';

  ++branch_cnt_;
}

void AssemblyGenerator::Visit(IRJumpInstructionNode *node) {
  os_ << "\t# Jump Instruction\n";
  os_ << "\tj " << ".L" << current_func_name_ << "_" << node->destination_ << '\n';
}

void AssemblyGenerator::Visit(IRReturnInstructionNode *node) {
  os_ << "\t# Return Instruction\n";
  if (!node->type_->IsEmpty()) {
    VariableForceToReg(node->name_, "a0", node->type_->base_type_);
  }
  // restore used s-registers from the bottom of the frame
  {
    uint32_t s_off = 0;
    for (auto reg_id : cur_func_->used_s_regs_) {
      PrintMem(os_, "ld", kRegisterName[reg_id], "sp", s_off);
      s_off += 8;
    }
  }
  PrintIA(os_, "addi", "sp", "sp", current_stack_);
  os_ << "\tret\n";
}

void AssemblyGenerator::Visit(IRAllocateInstructionNode *node) {
  os_ << "\t# Allocate Instruction " << node->result_ << '\n';
  auto rd = GetResultReg(node->storage_type_, node->address_, 1);
  PrintIA(os_, "addi", rd, "sp", current_stack_ - node->inner_address_);
  RegToVariable(node->storage_type_, node->address_, rd, "ptr");
}

void AssemblyGenerator::Visit(IRLoadInstructionNode *node) {
  os_ << "\t# Load Instruction " << node->result_ << '\n';
  auto ptr_reg = VariableToReg(node->pointer_, 0, "ptr");
  if (node->storage_type_ == kRegister) {
    auto [l_ins, _] = LoadStoreType(node->type_->base_type_);
    PrintMem(os_, l_ins, "x" + std::to_string(node->address_), ptr_reg, 0);
  } else {
    SaveRegister();
    PrintIA(os_, "addi", "a0", "sp", current_stack_ - node->address_);
    bool flag = true;
    for (uint32_t i = 10; i < 18; ++i) {
      if (SameRegister(i, ptr_reg)) {
        PrintMem(os_, "ld", "a1", "sp", current_stack_ - 56 - 8 * (i - 10));
        flag = false;
      }
    }
    if (flag) {
      os_ << "\tmv\ta1, " << ptr_reg << '\n';
    }
    os_ << "\tli\ta2, " << node->type_->allocated_size_ << '\n';
    os_ << "\tcall\tbuiltin_memcpy\n";
    RestoreRegister();
  }
}

void AssemblyGenerator::Visit(IRStoreVariableInstructionNode *node) {
  // After mem2reg, this type of instruction may store const
  os_ << "\t# Store Instruction\n";
  auto ptr_reg = VariableToReg(node->pointer_, 0, "ptr");
  auto [type, address] = GetVariableAddress(node->value_);
  if (type == kConst) {
    os_ << "\tli\tt1, " << node->value_ << '\n';
    auto [_, s_ins] = LoadStoreType(node->type_->base_type_);
    PrintMem(os_, s_ins, "t1", ptr_reg, 0);
  } else if (type == kRegister) {
    auto [_, s_ins] = LoadStoreType(node->type_->base_type_);
    PrintMem(os_, s_ins, "x" + std::to_string(address), ptr_reg, 0);
  } else {
    SaveRegister();
    bool flag = true;
    for (uint32_t i = 10; i < 18; ++i) {
      if (SameRegister(i, ptr_reg)) {
        PrintMem(os_, "ld", "a0", "sp", current_stack_ - 56 - 8 * (i - 10));
        flag = false;
      }
    }
    if (flag) {
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
  auto ptr_reg = VariableToReg(node->pointer_, 0, "ptr");
  os_ << "\tli\tt1, " << node->value_ << '\n';
  auto [_, s_ins] = LoadStoreType(node->type_);
  PrintMem(os_, s_ins, "t1", ptr_reg, 0);
}

void AssemblyGenerator::Visit(IRGetElementPtrInstructionNode *node) {
  os_ << "\t# GetElementPtr Instruction " << node->result_ << '\n';
  auto ptr_reg = VariableToReg(node->ptrval_, 0, "ptr");
  auto rd = GetResultReg(node->storage_type_, node->address_, 1);
  uint32_t offset = 0;
  if (node->type_->length_.empty()) {
    uint32_t align = 1;
    auto struct_node = StructMap::Instance().Query(node->type_->base_type_);
    for (uint32_t i = 0; i < node->index_; ++i) {
      if (align == 1 && struct_node->members_[i]->align_ == 8) {
        align = 8;
        offset = Align8(offset);
      }
      offset += struct_node->members_[i]->allocated_size_;
    }
    if (struct_node->members_[node->index_]->align_ == 8) {
      offset = Align8(offset);
    }
  } else {
    offset = node->type_->allocated_size_ / node->type_->length_[0] * node->index_;
  }
  PrintIA(os_, "addi", rd, ptr_reg, offset);
  RegToVariable(node->storage_type_, node->address_, rd, "ptr");
}

void AssemblyGenerator::Visit(IRGetElementPtrPrimeInstructionNode *node) {
  os_ << "\t# GetElementPtr Instruction " << node->result_ << '\n';
  auto ptr_reg = VariableToReg(node->ptrval_, 0, "ptr");
  auto index_reg = VariableToReg(node->index_, 2, "i32");
  auto rd = GetResultReg(node->storage_type_, node->address_, 1);
  assert(!node->type_->length_.empty());
  os_ << "\tli\tt1, " << node->type_->allocated_size_ / node->type_->length_[0] << '\n';
  os_ << "\tmul\tt2, t1, " << index_reg << '\n';
  os_ << "\tadd " << rd << ", " << ptr_reg << ", t2\n";
  RegToVariable(node->storage_type_, node->address_, rd, "ptr");
}

void AssemblyGenerator::Visit(IRCompareInstructionNode *node) {
  os_ << "\t# Compare Instruction " << node->result_ << '\n';
  auto rs1 = VariableToReg(node->operand1_, 0, node->type_);
  auto rs2 = VariableToReg(node->operand2_, 1, node->type_);
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
  RegToVariable(node->storage_type_, node->address_, rd, "i1");
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
      PrintIA(os_, "addi", "a0", "sp", -static_cast<int32_t>(para->address_));
      PrintIA(os_, "addi", "a1", "sp", current_stack_ - address);
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
        auto [_, s_ins] = LoadStoreType(para->type_->base_type_);
        PrintMem(os_, s_ins, "t0", "sp", -static_cast<int32_t>(para->address_));
      }
    } else if (type == kRegister) {
      auto [l_ins, s_ins] = LoadStoreType(para->type_->base_type_);
      if (address >= 10 && address < 18) { // The correct values are in the memory.
        if (para->storage_type_ == kRegister) {
          PrintMem(os_, l_ins, "x" + std::to_string(para->address_), "sp", current_stack_ - 56 - 8 * (address - 10));
        } else {
          PrintMem(os_, l_ins, "t0", "sp", current_stack_ - 56 - 8 * (address - 10));
          PrintMem(os_, s_ins, "t0", "sp", -static_cast<int32_t>(para->address_));
        }
      } else {
        if (para->storage_type_ == kRegister) {
          if (para->address_ != address) {
            os_ << "\tmv\tx" << para->address_ << ", x" << address << '\n';
          }
        } else {
          PrintMem(os_, s_ins, "x" + std::to_string(address), "sp", -static_cast<int32_t>(para->address_));
        }
      }
    } else {
      if (para->storage_type_ == kRegister) {
        auto [l_ins, _] = LoadStoreType(para->type_->base_type_);
        PrintMem(os_, l_ins, "x" + std::to_string(para->address_), "sp", current_stack_ - address);
      }
    }
  }
  os_ << "\tcall\t" << node->function_name_ << '\n';
  if (!node->result_type_->IsEmpty()) {
    RegToVariable(node->storage_type_, node->address_, "a0", node->result_type_->base_type_);
  }

  RestoreRegister();
}

void AssemblyGenerator::Visit(IRMoveInstructionNode *node) {
  os_ << "\t# Move " << node->result_ << " <- " << node->source_ << '\n';
  DataMove(node->source_, node->storage_type_, node->address_, node->type_);
}

void AssemblyGenerator::Visit(IRSelectInstructionNode *node) {
  os_ << "\t# Select Instruction " << node->result_ << '\n';
  auto rs = VariableToReg(node->cond_, 0, "i1");
  auto rd = GetResultReg(node->storage_type_, node->address_, 1);
  PrintIA(os_, "slti", rd, rs, 1);
  PrintIA(os_, "xori", rd, rd, 1);
  RegToVariable(node->storage_type_, node->address_, rd, "i32");
}

void AssemblyGenerator::Visit(IRBlockNode *node) {
  cur_block_ = node->id_;
  if (node->id_ != 0) {
    os_ << ".L" << current_func_name_ << "_" << node->id_ << ":\n";
  }
  for (auto &instruction : node->instructions_) {
    if (instruction->removed_) {
      continue;
    }
    instruction->Accept(this);
  }
}

void AssemblyGenerator::Visit(IRParameterNode *node) {}

void AssemblyGenerator::Visit(IRFunctionNode *node) {
  os_ << "\t.text\n";
  os_ << "\t.globl " << node->name_ << "        # -- Begin function " << node->name_ << '\n';
  os_ << "\t.p2align 2\n";
  os_ << "\t.type " << node->name_ << ",@function\n";
  os_ << node->name_ << ":        # @" << node->name_ << '\n';

  // Extend the frame by s_save bytes to hold s-reg saves at the bottom
  // (sp + 0, sp + 8, ...).  a-reg/ra/t1 saves stay at the top within the
  // MemoryAllocator's 64-byte reserved area.  The two areas never overlap.
  uint32_t s_save = 8 * node->used_s_regs_.size();
  uint32_t total_stack = node->stack_size_ + s_save;

  PrintIA(os_, "addi", "sp", "sp", -static_cast<int32_t>(total_stack));

  current_stack_ = total_stack;
  current_func_name_ = node->name_;
  current_a_reg_used_ = node->a_reg_used_cnt_;
  current_variables_ = &node->variables_;
  variable_storage_ = &node->variable_storage_;

  cur_func_ = node;

  // save used s-registers at the bottom of the frame
  {
    uint32_t s_off = 0;
    for (auto reg_id : node->used_s_regs_) {
      PrintMem(os_, "sd", kRegisterName[reg_id], "sp", s_off);
      s_off += 8;
    }
  }

  // blocks
  for (auto &block : node->blocks_) {
    block->Accept(this);
  }
}

void AssemblyGenerator::Visit(IRRootNode *node) {
  std::cerr << builtin_begin_ << '\n';
  for (auto &function_node : node->functions_) {
    function_node->Accept(this);
  }
}