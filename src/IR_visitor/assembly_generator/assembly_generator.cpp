#include <iostream>
#include <cassert>
#include "IR_visitor/assembly_generator/assembly_generator.h"
#include "IR_visitor/memory_allocator/memory_allocator.h"
#include "IR/struct_map.h"
#include "IR/function_map.h"
#include "codegen/register.h"
#include "codegen/instruction.h"

AssemblyGenerator::AssemblyGenerator(const std::string &builtin_begin, std::ostream &os,
                                   std::ostream *builtin_os) :
  builtin_begin_(builtin_begin), os_(os), builtin_os_(builtin_os) {}

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
    // Only use the cache for plain integer constants (avoid stoll on
    // empty/malformed names that may appear in partially-registered IR).
    if (!name.empty() && (name[0] == '-' || std::isdigit(name[0]))) {
      int64_t val = std::stoll(name);
      if (val == 0) return "x0";  // use zero register, avoid li
      auto it = const_cache_.find(val);
      if (it != const_cache_.end()) {
        return it->second;
      }
      os_ << "\tli\tt" << reg_id << ",\t" << name << '\n';
      return "t" + std::to_string(reg_id);
    }
    // Fallback: empty or non-integer name — emit li 0 to avoid asm errors.
    os_ << "\tli\tt" << reg_id << ",\t0\n";
    return "t" + std::to_string(reg_id);
  }
  // a-reg values are stale after a call; load from save slot
  if (address >= 10 && address <= 17 && registers_saved_) {
    uint32_t save_off = 56 + 8 * (address - 10);
    TransferToTreg(save_off, reg_id, val_type);
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
  if (registers_saved_) return;  // still valid in save slots from prior call
  // Save a0~aN, ra, and t1 at the top of the frame.
  // t3/t4 hold known constants; we reload them via li after the
  // call instead of saving/restoring to memory.
  for (uint32_t i = 0; i < current_a_reg_used_; ++i) {
    PrintMem(os_, "sd", "a" + std::to_string(i), "sp", current_stack_ - 56 - 8 * i);
  }
  PrintMem(os_, "sd", "ra", "sp", current_stack_ - 56 - 64);
  PrintMem(os_, "sd", "t1", "sp", current_stack_ - 8);
  registers_saved_ = true;
}

void AssemblyGenerator::RestoreRegister() {
  for (uint32_t i = 0; i < current_a_reg_used_; ++i) {
    PrintMem(os_, "ld", "a" + std::to_string(i), "sp", current_stack_ - 56 - 8 * i);
  }
  PrintMem(os_, "ld", "ra", "sp", current_stack_ - 56 - 64);
  PrintMem(os_, "ld", "t1", "sp", current_stack_ - 8);
  // Reload hoisted constants (caller-saved t3/t4 are clobbered by calls).
  for (auto &[val, reg] : const_cache_) {
    os_ << "\tli\t" << reg << ",\t" << val << '\n';
  }
  registers_saved_ = false;
}

void AssemblyGenerator::FlushSavedRegisters() {
  if (registers_saved_) RestoreRegister();
}

void AssemblyGenerator::DataMove(const std::string &from, StorageType to_type, uint32_t to_address, std::shared_ptr<IRArrayNode> type) {
  auto [from_type, from_address] = GetVariableAddress(from);
  // If the source is an a-reg and registers are saved (stale), restore first.
  if (from_type == kRegister && from_address >= 10 && from_address <= 17) {
    FlushSavedRegisters();
  }
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
  auto rd = GetResultReg(node->storage_type_, node->address_, 2);

  // Fold small constant operands into immediate instruction forms.
  // This turns "li tX, c; op rd, rs, tX" (2 ins) into "opi rd, rs, c" (1 ins).
  {
    auto [op2_type, _] = GetVariableAddress(node->operand2_);
    if (op2_type == kConst) {
      int32_t imm = std::stoi(node->operand2_);
      if (node->op_ == "+" && imm >= -2048 && imm <= 2047) {
        auto rs1 = VariableToReg(node->operand1_, 0, node->type_);
        os_ << "\taddiw\t" << rd << ", " << rs1 << ", " << imm << '\n';
        RegToVariable(node->storage_type_, node->address_, rd, node->type_);
        return;
      }
      if (node->op_ == "-" && imm >= -2048 && imm <= 2047) {
        auto rs1 = VariableToReg(node->operand1_, 0, node->type_);
        os_ << "\taddiw\t" << rd << ", " << rs1 << ", " << -imm << '\n';
        RegToVariable(node->storage_type_, node->address_, rd, node->type_);
        return;
      }
    }
  }

  auto rs1 = VariableToReg(node->operand1_, 0, node->type_);
  auto rs2 = VariableToReg(node->operand2_, 1, node->type_);
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
  auto [op_type, op_addr] = GetVariableAddress(node->operand_);
  if (node->is_minus_ && op_type == kConst) {
    auto rd = GetResultReg(node->storage_type_, node->address_, 1);
    int32_t val = std::stoi(node->operand_);
    os_ << "\tli\t" << rd << ", " << -val << '\n';
    RegToVariable(node->storage_type_, node->address_, rd, node->type_);
    return;
  }
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

  auto label = [&](uint32_t id) {
    return ".L" + current_func_name_ + "_" + std::to_string(id);
  };

  if (!large_function_) {
    // Elide redundant fall-through jumps.
    auto get_next = [&]() -> uint32_t {
      auto it = next_block_map_.find(cur_block_);
      return (it != next_block_map_.end()) ? it->second : UINT32_MAX;
    };
    uint32_t nid = get_next();

    if (node->false_branch_ == nid) {
      // false target is next block; just branch to true.
      os_ << "\tbnez\t" << rs << ", " << label(node->true_branch_) << '\n';
      return;
    }
    if (node->true_branch_ == nid) {
      // true target is next block; invert and branch to false.
      os_ << "\tbeqz\t" << rs << ", " << label(node->false_branch_) << '\n';
      return;
    }
    os_ << "\tbnez\t" << rs << ", " << label(node->true_branch_) << '\n';
    os_ << "\tj " << label(node->false_branch_) << '\n';
    return;
  }

  auto true_lbl  = label(node->true_branch_);
  auto false_lbl = label(node->false_branch_);

  // Long-branch pattern for huge functions.
  // bnez jumps to a local forward label (always within +/-4KB),
  // then lui+addi+jalr handles the arbitrary-distance jump.
  os_ << "\tbnez\t" << rs << ", 1f\n";
  os_ << "\tlui\tt5, %hi(" << false_lbl << ")\n";
  os_ << "\taddi\tt5, t5, %lo(" << false_lbl << ")\n";
  os_ << "\tjalr\tx0, t5, 0\n";
  os_ << "1:\n";
  os_ << "\tlui\tt5, %hi(" << true_lbl << ")\n";
  os_ << "\taddi\tt5, t5, %lo(" << true_lbl << ")\n";
  os_ << "\tjalr\tx0, t5, 0\n";
}

void AssemblyGenerator::Visit(IRJumpInstructionNode *node) {
  os_ << "\t# Jump Instruction\n";
  auto lbl = ".L" + current_func_name_ + "_" + std::to_string(node->destination_);
  if (!large_function_) {
    auto it = next_block_map_.find(cur_block_);
    if (it != next_block_map_.end() && it->second == node->destination_) {
      // destination is the next block, no jump needed.
      return;
    }
    os_ << "\tj " << lbl << '\n';
    return;
  }
  os_ << "\tlui\tt5, %hi(" << lbl << ")\n";
  os_ << "\taddi\tt5, t5, %lo(" << lbl << ")\n";
  os_ << "\tjalr\tx0, t5, 0\n";
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
    const_cache_.clear();
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
    const_cache_.clear();
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
  uint32_t elem_size = node->type_->allocated_size_ / node->type_->length_[0];
  if (elem_size == 1) {
    os_ << "\tadd " << rd << ", " << ptr_reg << ", " << index_reg << '\n';
  } else if ((elem_size & (elem_size - 1)) == 0) {
    // Power of 2: use shift
    uint32_t shift = __builtin_ctz(elem_size);
    os_ << "\tslli\tt1, " << index_reg << ", " << shift << '\n';
    os_ << "\tadd " << rd << ", " << ptr_reg << ", t1\n";
  } else {
    os_ << "\tli\tt1, " << elem_size << '\n';
    os_ << "\tmul\tt2, t1, " << index_reg << '\n';
    os_ << "\tadd " << rd << ", " << ptr_reg << ", t2\n";
  }
  RegToVariable(node->storage_type_, node->address_, rd, "ptr");
}

void AssemblyGenerator::Visit(IRCompareInstructionNode *node) {
  os_ << "\t# Compare Instruction " << node->result_ << '\n';

  auto [type1, addr1] = GetVariableAddress(node->operand1_);
  auto [type2, addr2] = GetVariableAddress(node->operand2_);
  bool op1c = (type1 == kConst);
  bool op2c = (type2 == kConst);
  auto rd = GetResultReg(node->storage_type_, node->address_, 2);

  // Helper: fold small constant into addi for ==/!=.  Returns true if folded.
  auto foldConstCmp = [&](bool is_eq, const std::string &const_op,
                          const std::string &var_op, uint32_t var_reg_id) -> bool {
    if (const_op.empty() || (const_op[0] != '-' && !std::isdigit(const_op[0])))
      return false;
    int64_t val = std::stoll(const_op);
    if (val == 0 || val < -2048 || val > 2047) return false;
    int64_t neg = -val;
    if (neg < -2048 || neg > 2047) return false;
    auto rs = VariableToReg(var_op, var_reg_id, node->type_);
    PrintIA(os_, "addi", "t0", rs, neg);
    if (is_eq)
      os_ << "\tsltiu\t" << rd << ", t0, 1\n";
    else
      os_ << "\tsltu\t" << rd << ", x0, t0\n";
    return true;
  };

  // Peephole: ==0, !=0 — load only the non-const operand.
  if (node->op_ == IRCompareInstructionNode::kEq) {
    if (op1c && node->operand1_ == "0") {
      auto rs2 = VariableToReg(node->operand2_, 1, node->type_);
      os_ << "\tsltiu\t" << rd << ", " << rs2 << ", 1\n";
      RegToVariable(node->storage_type_, node->address_, rd, "i1");
      return;
    }
    if (op2c && node->operand2_ == "0") {
      auto rs1 = VariableToReg(node->operand1_, 0, node->type_);
      os_ << "\tsltiu\t" << rd << ", " << rs1 << ", 1\n";
      RegToVariable(node->storage_type_, node->address_, rd, "i1");
      return;
    }
    // Peephole: ==small_const — fold into addi (avoids li).
    if (op2c && foldConstCmp(true, node->operand2_, node->operand1_, 0)) {
      RegToVariable(node->storage_type_, node->address_, rd, "i1");
      return;
    }
    if (op1c && foldConstCmp(true, node->operand1_, node->operand2_, 1)) {
      RegToVariable(node->storage_type_, node->address_, rd, "i1");
      return;
    }
  }
  if (node->op_ == IRCompareInstructionNode::kNe) {
    if (op1c && node->operand1_ == "0") {
      auto rs2 = VariableToReg(node->operand2_, 1, node->type_);
      os_ << "\tsltu\t" << rd << ", x0, " << rs2 << '\n';
      RegToVariable(node->storage_type_, node->address_, rd, "i1");
      return;
    }
    if (op2c && node->operand2_ == "0") {
      auto rs1 = VariableToReg(node->operand1_, 0, node->type_);
      os_ << "\tsltu\t" << rd << ", x0, " << rs1 << '\n';
      RegToVariable(node->storage_type_, node->address_, rd, "i1");
      return;
    }
    // Peephole: !=small_const — fold into addi (avoids li).
    if (op2c && foldConstCmp(false, node->operand2_, node->operand1_, 0)) {
      RegToVariable(node->storage_type_, node->address_, rd, "i1");
      return;
    }
    if (op1c && foldConstCmp(false, node->operand1_, node->operand2_, 1)) {
      RegToVariable(node->storage_type_, node->address_, rd, "i1");
      return;
    }
  }

  // General case: load both operands.
  auto rs1 = VariableToReg(node->operand1_, 0, node->type_);
  auto rs2 = VariableToReg(node->operand2_, 1, node->type_);

  // Use t0 as the only aux register (was t0+t3, now just t0 frees t3).
  if (node->op_ == IRCompareInstructionNode::kEq) {
    os_ << "\tsub\tt0, " << rs1 << ", " << rs2 << '\n';
    os_ << "\tsltiu\t" << rd << ", t0, 1\n";
  } else if (node->op_ == IRCompareInstructionNode::kNe) {
    os_ << "\tsub\tt0, " << rs1 << ", " << rs2 << '\n';
    os_ << "\tsltu\t" << rd << ", x0, t0\n";
  } else if (node->op_ == IRCompareInstructionNode::kUgt) {
    os_ << "\tsltu\t" << rd << ", " << rs2 << ", " << rs1 << '\n';
  } else if (node->op_ == IRCompareInstructionNode::kUge) {
    os_ << "\tsltu\tt0, " << rs1 << ", " << rs2 << '\n';
    PrintIA(os_, "xori", rd, "t0", 1);
  } else if (node->op_ == IRCompareInstructionNode::kUlt) {
    os_ << "\tsltu\t" << rd << ", " << rs1 << ", " << rs2 << '\n';
  } else if (node->op_ == IRCompareInstructionNode::kUle) {
    os_ << "\tsltu\tt0, " << rs2 << ", " << rs1 << '\n';
    PrintIA(os_, "xori", rd, "t0", 1);
  } else if (node->op_ == IRCompareInstructionNode::kSgt) {
    os_ << "\tslt\t" << rd << ", " << rs2 << ", " << rs1 << '\n';
  } else if (node->op_ == IRCompareInstructionNode::kSge) {
    os_ << "\tslt\tt0, " << rs1 << ", " << rs2 << '\n';
    PrintIA(os_, "xori", rd, "t0", 1);
  } else if (node->op_ == IRCompareInstructionNode::kSlt) {
    os_ << "\tslt\t" << rd << ", " << rs1 << ", " << rs2 << '\n';
  } else {
    os_ << "\tslt\tt0, " << rs2 << ", " << rs1 << '\n';
    PrintIA(os_, "xori", rd, "t0", 1);
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
      const_cache_.clear();
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

  // Stash the return value in t0 before RestoreRegister, which would
  // otherwise restore a0 from the save slot and clobber the result.
  // t0 survives RestoreRegister (it only touches a0-aN, ra, and t1).
  bool has_result = !node->result_type_->IsEmpty();
  if (has_result) {
    os_ << "\tmv\tt0, a0\n";
  }

  // If the next instruction is also a call, keep the saved state so the
  // next SaveRegister/RestoreRegister pair can be skipped entirely.
  if (NextInstructionIsCall()) {
    // registers_saved_ stays true → next call skips SaveRegister.
    // But reload hoisted constants: the call clobbered t3/t4 and the
    // cache entries still point to them.
    for (auto &[val, reg] : const_cache_) {
      os_ << "\tli\t" << reg << ",\t" << val << '\n';
    }
  } else {
    RestoreRegister();
  }

  if (has_result) {
    RegToVariable(node->storage_type_, node->address_, "t0", node->result_type_->base_type_);
  }
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
  cur_instructions_ = &node->instructions_;
  for (cur_ins_index_ = 0; cur_ins_index_ < node->instructions_.size(); ++cur_ins_index_) {
    auto &instruction = node->instructions_[cur_ins_index_];
    if (instruction->removed_) continue;
    instruction->Accept(this);
  }
  cur_instructions_ = nullptr;
}

bool AssemblyGenerator::NextInstructionIsCall() {
  if (!cur_instructions_) return false;
  for (size_t i = cur_ins_index_ + 1; i < cur_instructions_->size(); ++i) {
    if ((*cur_instructions_)[i]->removed_) continue;
    return dynamic_cast<IRCallInstructionNode *>((*cur_instructions_)[i].get()) != nullptr;
  }
  return false;
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
  total_stack = (total_stack + 15) / 16 * 16;  // 16-byte alignment for RISC-V ABI

  PrintIA(os_, "addi", "sp", "sp", -static_cast<int32_t>(total_stack));

  current_stack_ = total_stack;
  current_func_name_ = node->name_;
  current_a_reg_used_ = node->a_reg_used_cnt_;
  current_variables_ = &node->variables_;
  variable_storage_ = &node->variable_storage_;

  cur_func_ = node;
  const_cache_.clear();

  // Estimate function code size: count non-removed instructions.
  // If the function is very large (e.g. a huge expression producing
  // thousands of basic blocks), intra-function j/bnez may exceed the
  // RISC-V range limits (jal: +/-1MB, B-type: +/-4KB).  We switch to
  // a long-jump pattern (lui+addi+jalr) only when needed.
  {
    uint32_t total_ins = 0;
    for (auto &block : node->blocks_) {
      for (auto &ins : block->instructions_) {
        if (!ins->removed_) ++total_ins;
      }
    }
    // ~5 bytes per instruction on average; 40000 ins ≈ 200KB, or
    // 800+ blocks, both safely under the 1MB jal limit.  Either
    // condition triggers the long-jump pattern.
    large_function_ = (total_ins > 40000 || node->blocks_.size() > 800);

    // Map each block to its successor in layout order, for
    // eliminating redundant fall-through jumps.
    next_block_map_.clear();
    for (size_t i = 0; i + 1 < node->blocks_.size(); ++i) {
      next_block_map_[node->blocks_[i]->id_] = node->blocks_[i + 1]->id_;
    }

    // Pre-scan: hoist large constants (>12-bit) into t3 and t4.
    {
      std::set<int64_t> cs;
      for (auto &block : node->blocks_) {
        for (auto &ins : block->instructions_) {
          if (ins->removed_) continue;
          auto scan = [&](const std::string &s) {
            if (!s.empty() && (s[0] == '-' || std::isdigit(s[0])))
              cs.insert(std::stoll(s));
          };
          if (auto *p = dynamic_cast<IRArithmeticInstructionNode *>(ins.get())) {
            scan(p->operand1_); scan(p->operand2_);
          } else if (auto *p = dynamic_cast<IRCompareInstructionNode *>(ins.get())) {
            scan(p->operand1_); scan(p->operand2_);
          } else if (auto *p = dynamic_cast<IRStoreConstInstructionNode *>(ins.get())) {
            cs.insert(p->value_);
          }
        }
      }
      uint32_t treg = 3;
      for (auto v : cs) {
        if (v >= -2048 && v <= 2047) continue;
        if (treg > 4) break;
        std::string r = "t" + std::to_string(treg);
        os_ << "\tli\t" << r << ",\t" << v << '\n';
        const_cache_[v] = r;
        ++treg;
      }
    }
  }

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
  auto &builtin_out = builtin_os_ ? *builtin_os_ : os_;
  builtin_out << builtin_begin_ << '\n';
  for (auto &function_node : node->functions_) {
    function_node->Accept(this);
  }
}