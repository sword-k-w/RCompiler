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

void AssemblyGenerator::EmitMem(const std::string &type, const std::string &r, const std::string &rs1, int32_t imm) {
  PrintMem(os_, type, r, rs1, imm, &const_cache_);
}

void AssemblyGenerator::EmitIA(const std::string &type, const std::string &rd, const std::string &rs1, int32_t imm) {
  PrintIA(os_, type, rd, rs1, imm, &const_cache_);
}

void AssemblyGenerator::EmitIStar(const std::string &type, const std::string &rd, const std::string &rs1, int32_t imm) {
  PrintIStar(os_, type, rd, rs1, imm, &const_cache_);
}

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
  EmitMem( LoadStoreType(val_type).first, "t" + std::to_string(reg_id), "sp", current_stack_ - address);
}

std::string AssemblyGenerator::GetResultReg(StorageType storage_type, uint32_t address, uint32_t reg_id) {
  if (storage_type == kMemory) {
    return "t" + std::to_string(reg_id);
  }
  // Writing a result to a register makes it valid (overwrites any garbage);
  // RegToVariable marks it valid.  No t-reg routing needed — the hardware
  // register is always the canonical target with per-register tracking.
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
  // If the a-reg is invalid (has call garbage), restore from the save slot
  // first so subsequent accesses use the hardware register directly.
  if (address >= 10 && address <= 17) EnsureARegValid(address);
  return "x" + std::to_string(address);
}

void AssemblyGenerator::VariableForceToReg(const std::string &name, const std::string &reg, const std::string &val_type) {
  auto [type, address] = GetVariableAddress(name);
  if (type == kMemory) {
    EmitMem( LoadStoreType(val_type).first, reg, "sp", current_stack_ - address);
  } else if (type == kConst) {
    os_ << "\tli\t" << reg << ", " << name << '\n';
  } else {
    if (address >= 10 && address <= 17) EnsureARegValid(address);
    if (!SameRegister(address, reg)) {
      os_ << "\tmv\t" << reg << ", x" << address << '\n';
    }
  }
}

void AssemblyGenerator::RegToVariable(StorageType storage_type, uint32_t address, const std::string &reg, const std::string &val_type) {
  assert(storage_type != kConst);
  if (storage_type == kMemory) {
    EmitMem( LoadStoreType(val_type).second, reg, "sp", current_stack_ - address);
  } else if (!SameRegister(address, reg)) {
    os_ << "\tmv\tx" << address << ", " << reg << '\n';
  }
  // Writing to a register makes it valid: the hardware now holds the canonical
  // value.  No save-slot mirror needed — the next SaveRegister() will persist
  // it if it's still valid when a call occurs.
  if (address >= 10 && address <= 17) a_reg_valid_[address - 10] = true;
}

void AssemblyGenerator::SaveRegister() {
  // Save only valid a-regs — invalid ones already have the correct value in
  // their save slot (from an earlier save).  After the call all become invalid.
  for (uint32_t i = 0; i < current_a_reg_used_; ++i) {
    if (a_reg_valid_[i]) {
      EmitMem( "sd", "a" + std::to_string(i), "sp", current_stack_ - 8 - 8 * i);
      a_reg_valid_[i] = false;
    }
  }
  // ra is saved at most once per block.
  if (!ra_saved_) {
    EmitMem( "sd", "ra", "sp", current_stack_ - 8 - 8 * current_a_reg_used_);
    ra_saved_ = true;
  }
}

void AssemblyGenerator::EnsureARegValid(uint32_t addr) {
  uint32_t idx = addr - 10;
  if (!a_reg_valid_[idx]) {
    uint32_t save_off = 8 * (addr - 9);
    EmitMem( "ld", "x" + std::to_string(addr), "sp", current_stack_ - save_off);
    a_reg_valid_[idx] = true;
  }
}

void AssemblyGenerator::ReloadConstCache() {
  // t3/t4 are caller-saved and clobbered by every call; reload cached
  // constants so they stay valid between calls within the same block.
  for (auto &[val, reg] : const_cache_) {
    os_ << "\tli\t" << reg << ",\t" << val << '\n';
  }
}

void AssemblyGenerator::FlushSavedRegisters() {
  // Only emit anything if this block actually had a call (some a-reg
  // is still invalid or ra is saved).  This avoids redundant
  // ReloadConstCache / restore at terminators in call-free blocks.
  bool any_invalid = false;
  for (uint32_t i = 0; i < current_a_reg_used_; ++i) {
    if (!a_reg_valid_[i]) { any_invalid = true; break; }
  }
  if (!any_invalid && !ra_saved_) return;

  // Reload const cache first — the EmitMem calls below may use cached offsets.
  ReloadConstCache();
  for (uint32_t i = 0; i < current_a_reg_used_; ++i) {
    if (!a_reg_valid_[i]) {
      EmitMem( "ld", "a" + std::to_string(i), "sp", current_stack_ - 8 - 8 * i);
      a_reg_valid_[i] = true;
    }
  }
  if (ra_saved_) {
    EmitMem( "ld", "ra", "sp", current_stack_ - 8 - 8 * current_a_reg_used_);
    ra_saved_ = false;
  }
}

void AssemblyGenerator::DataMove(const std::string &from, StorageType to_type, uint32_t to_address, std::shared_ptr<IRArrayNode> type) {
  auto [from_type, from_address] = GetVariableAddress(from);
  if (from_type == kRegister && from_address >= 10 && from_address <= 17) {
    EnsureARegValid(from_address);
  }
  if (from_type == kConst) {
    if (to_type == kRegister) {
      os_ << "\tli\tx" << to_address << ", " << from << '\n';
      if (to_address >= 10 && to_address <= 17) a_reg_valid_[to_address - 10] = true;
    } else {
      os_ << "\tli\tt0, " << from << '\n';
      auto [_, s_ins] = LoadStoreType(type->base_type_);
      EmitMem( s_ins, "t0", "sp", current_stack_ - to_address);
    }
  } else if (from_type == kRegister) {
    DataMoveFromReg("x" + std::to_string(from_address), to_type, to_address, type);
  } else {
    if (to_type == kRegister) {
      auto [l_ins, _] = LoadStoreType(type->base_type_);
      EmitMem( l_ins, "x" + std::to_string(to_address), "sp", current_stack_ - from_address);
      if (to_address >= 10 && to_address <= 17) a_reg_valid_[to_address - 10] = true;
    } else {
      SaveRegister();
      EmitIA( "addi", "a0", "sp", current_stack_ - to_address);
      EmitIA( "addi", "a1", "sp", current_stack_ - from_address);
      os_ << "\tli\ta2, " << type->allocated_size_ << '\n';
      os_ << "\tcall\tbuiltin_memcpy\n";
      // Defer a-reg restore to the block terminator: only invalid a-regs are
      // restored on-demand; consecutive calls only save valid a-regs.
      ReloadConstCache();
    }
  }
}

void AssemblyGenerator::DataMoveFromReg(const std::string &from, StorageType to_type, uint32_t to_address, std::shared_ptr<IRArrayNode> type) {
  auto [l_ins, s_ins] = LoadStoreType(type->base_type_);
  if (to_type == kRegister) {
    if (!SameRegister(to_address, from)) {
      os_ << "\tmv\tx" << to_address << ", " << from << '\n';
    }
    if (to_address >= 10 && to_address <= 17) a_reg_valid_[to_address - 10] = true;
  } else {
    EmitMem( s_ins, from, "sp", current_stack_ - to_address);
  }
}

void AssemblyGenerator::Visit(IRArrayNode *node) {}

void AssemblyGenerator::Visit(IRStructNode *node) {}

void AssemblyGenerator::Visit(IRArithmeticInstructionNode *node) {
  auto rd = GetResultReg(node->storage_type_, node->address_, 2);

  // Fold small constant operands into immediate instruction forms.
  // This turns "li tX, c; op rd, rs, tX" (2 ins) into "opi rd, rs, c" (1 ins).
  bool is_ptr = (node->type_ == "ptr");
  {
    auto [op2_type, _] = GetVariableAddress(node->operand2_);
    if (op2_type == kConst) {
      int32_t imm = std::stoi(node->operand2_);
      if (node->op_ == "+" && imm >= -2048 && imm <= 2047) {
        auto rs1 = VariableToReg(node->operand1_, 0, node->type_);
        os_ << "\t" << (is_ptr ? "addi" : "addiw") << "\t" << rd << ", " << rs1 << ", " << imm << '\n';
        RegToVariable(node->storage_type_, node->address_, rd, node->type_);
        return;
      }
      if (node->op_ == "-" && imm >= -2048 && imm <= 2047) {
        auto rs1 = VariableToReg(node->operand1_, 0, node->type_);
        os_ << "\t" << (is_ptr ? "addi" : "addiw") << "\t" << rd << ", " << rs1 << ", " << -imm << '\n';
        RegToVariable(node->storage_type_, node->address_, rd, node->type_);
        return;
      }
    }
  }

  auto rs1 = VariableToReg(node->operand1_, 0, node->type_);
  auto rs2 = VariableToReg(node->operand2_, 1, node->type_);
  os_ << "\t";
  if (node->op_ == "+") {
    os_ << (is_ptr ? "add" : "addw");
  } else if (node->op_ == "-") {
    os_ << (is_ptr ? "sub" : "subw");
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
      EmitIA( "xori", rd, rs, 1);
    } else {
      os_ << "\tnot\t" << rd << ", " << rs << '\n';
    }
  }
  RegToVariable(node->storage_type_, node->address_, rd, node->type_);
}

void AssemblyGenerator::Visit(IRBranchInstructionNode *node) {
  // Leaving the block: hardware a-regs/ra must be valid for successors.
  FlushSavedRegisters();
  auto rs = VariableToReg(node->condition_, 0, "i1");

  auto label = [&](uint32_t id) {
    return ".L" + std::to_string(block_label_map_[id]);
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
  // Leaving the block: hardware a-regs/ra must be valid for successors.
  FlushSavedRegisters();
  auto lbl = ".L" + std::to_string(block_label_map_[node->destination_]);
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
  // Leaving the function: hardware a-regs/ra must be valid — ra is needed by ret.
  FlushSavedRegisters();
  if (!node->type_->IsEmpty()) {
    VariableForceToReg(node->name_, "a0", node->type_->base_type_);
  }
  // restore used s-registers from the bottom of the frame
  {
    uint32_t s_off = 0;
    for (auto reg_id : cur_func_->used_s_regs_) {
      EmitMem( "ld", kRegisterName[reg_id], "sp", s_off);
      s_off += 8;
    }
  }
  if (current_stack_ != 0) {
    EmitIA( "addi", "sp", "sp", current_stack_);
  }
  os_ << "\tret\n";
}

void AssemblyGenerator::Visit(IRAllocateInstructionNode *node) {
  auto rd = GetResultReg(node->storage_type_, node->address_, 1);
  EmitIA( "addi", rd, "sp", current_stack_ - node->inner_address_);
  RegToVariable(node->storage_type_, node->address_, rd, "ptr");
}

void AssemblyGenerator::Visit(IRLoadInstructionNode *node) {
  auto ptr_reg = VariableToReg(node->pointer_, 0, "ptr");
  if (node->storage_type_ == kRegister) {
    auto [l_ins, _] = LoadStoreType(node->type_->base_type_);
    EmitMem( l_ins, "x" + std::to_string(node->address_), ptr_reg, 0);
    if (node->address_ >= 10 && node->address_ <= 17) a_reg_valid_[node->address_ - 10] = true;
  } else {
    SaveRegister();
    EmitIA( "addi", "a0", "sp", current_stack_ - node->address_);
    bool flag = true;
    for (uint32_t i = 10; i < 18; ++i) {
      if (SameRegister(i, ptr_reg)) {
        EnsureARegValid(i);
        os_ << "\tmv\ta1, x" << i << '\n';
        flag = false;
      }
    }
    if (flag) {
      os_ << "\tmv\ta1, " << ptr_reg << '\n';
    }
    os_ << "\tli\ta2, " << node->type_->allocated_size_ << '\n';
    os_ << "\tcall\tbuiltin_memcpy\n";
    // Defer restore to the block terminator (see DataMove).
    ReloadConstCache();
  }
}

void AssemblyGenerator::Visit(IRStoreVariableInstructionNode *node) {
  // After mem2reg, this type of instruction may store const
  auto ptr_reg = VariableToReg(node->pointer_, 0, "ptr");
  auto [type, address] = GetVariableAddress(node->value_);
  if (type == kConst) {
    os_ << "\tli\tt1, " << node->value_ << '\n';
    auto [_, s_ins] = LoadStoreType(node->type_->base_type_);
    EmitMem( s_ins, "t1", ptr_reg, 0);
  } else if (type == kRegister) {
    if (address >= 10 && address <= 17) EnsureARegValid(address);
    auto [_, s_ins] = LoadStoreType(node->type_->base_type_);
    EmitMem( s_ins, "x" + std::to_string(address), ptr_reg, 0);
  } else {
    SaveRegister();
    bool flag = true;
    for (uint32_t i = 10; i < 18; ++i) {
      if (SameRegister(i, ptr_reg)) {
        EnsureARegValid(i);
        os_ << "\tmv\ta0, x" << i << '\n';
        flag = false;
      }
    }
    if (flag) {
      os_ << "\tmv\ta0, " << ptr_reg << '\n';
    }
    EmitIA( "addi", "a1", "sp", current_stack_ - address);
    os_ << "\tli\ta2, " << node->type_->allocated_size_ << '\n';
    os_ << "\tcall\tbuiltin_memcpy\n";
    // Defer restore to the block terminator (see DataMove).
    ReloadConstCache();
  }
}

void AssemblyGenerator::Visit(IRStoreConstInstructionNode *node) {
  auto ptr_reg = VariableToReg(node->pointer_, 0, "ptr");
  os_ << "\tli\tt1, " << node->value_ << '\n';
  auto [_, s_ins] = LoadStoreType(node->type_);
  EmitMem( s_ins, "t1", ptr_reg, 0);
}

void AssemblyGenerator::Visit(IRGetElementPtrInstructionNode *node) {
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
      auto member_size = struct_node->members_[i]->allocated_size_;
      if (align == 8 && struct_node->members_[i]->align_ == 1) {
        member_size = Align8(member_size);
      }
      offset += member_size;
    }
    if (struct_node->members_[node->index_]->align_ == 8) {
      offset = Align8(offset);
    }
  } else {
    offset = node->type_->allocated_size_ / node->type_->length_[0] * node->index_;
  }
  if (offset == 0)
    os_ << "\tmv\t" << rd << ", " << ptr_reg << '\n';
  else
    EmitIA( "addi", rd, ptr_reg, offset);
  RegToVariable(node->storage_type_, node->address_, rd, "ptr");
}

void AssemblyGenerator::Visit(IRGetElementPtrPrimeInstructionNode *node) {
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
    EmitIA( "addi", "t0", rs, neg);
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
    EmitIA( "xori", rd, "t0", 1);
  } else if (node->op_ == IRCompareInstructionNode::kUlt) {
    os_ << "\tsltu\t" << rd << ", " << rs1 << ", " << rs2 << '\n';
  } else if (node->op_ == IRCompareInstructionNode::kUle) {
    os_ << "\tsltu\tt0, " << rs2 << ", " << rs1 << '\n';
    EmitIA( "xori", rd, "t0", 1);
  } else if (node->op_ == IRCompareInstructionNode::kSgt) {
    os_ << "\tslt\t" << rd << ", " << rs2 << ", " << rs1 << '\n';
  } else if (node->op_ == IRCompareInstructionNode::kSge) {
    os_ << "\tslt\tt0, " << rs1 << ", " << rs2 << '\n';
    EmitIA( "xori", rd, "t0", 1);
  } else if (node->op_ == IRCompareInstructionNode::kSlt) {
    os_ << "\tslt\t" << rd << ", " << rs1 << ", " << rs2 << '\n';
  } else {
    os_ << "\tslt\tt0, " << rs2 << ", " << rs1 << '\n';
    EmitIA( "xori", rd, "t0", 1);
  }
  RegToVariable(node->storage_type_, node->address_, rd, "i1");
}

void AssemblyGenerator::Visit(IRArgumentNode *node) {}

void AssemblyGenerator::Visit(IRCallInstructionNode *node) {
  auto function_node = FunctionMap::Instance().Query(node->function_name_);
  auto size = node->arguments_.size();

  SaveRegister();

  // operate the memory to memory in advance, using builtin_memcpy
  for (uint32_t i = 0; i < size; ++i) {
    auto para = function_node->parameters_[i];
    auto [type, address] = GetVariableAddress(node->arguments_[i]->value_);
    if (type == kMemory && para->storage_type_ == kMemory) {
      EmitIA( "addi", "a0", "sp", -static_cast<int32_t>(para->address_));
      EmitIA( "addi", "a1", "sp", current_stack_ - address);
      os_ << "\tli\ta2, " << para->type_->allocated_size_ << '\n';
      os_ << "\tcall\tbuiltin_memcpy\n";
      // The call clobbered t3/t4; reload the cached constants (don't clear the
      // map — SaveRegister/FlushSavedRegisters EmitMem may rely on the cached
      // offsets, which must stay valid across the loop body and beyond).
      ReloadConstCache();
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
        EmitMem( s_ins, "t0", "sp", -static_cast<int32_t>(para->address_));
      }
    } else if (type == kRegister) {
      auto [l_ins, s_ins] = LoadStoreType(para->type_->base_type_);
      if (address >= 10 && address < 18) {
        // For a-reg sources, the correct value may be in the save slot
        // (if invalid).  Load directly into the parameter register (or a
        // temp for memory params) — don't go through EnsureARegValid,
        // which would restore to the source a-reg hardware and risk
        // clobbering a previously-set parameter register.
        if (para->storage_type_ == kRegister) {
          EmitMem( l_ins, "x" + std::to_string(para->address_), "sp",
                  current_stack_ - 8 * (address - 9));
        } else {
          EmitMem( l_ins, "t0", "sp", current_stack_ - 8 * (address - 9));
          EmitMem( s_ins, "t0", "sp", -static_cast<int32_t>(para->address_));
        }
      } else {
        if (para->storage_type_ == kRegister) {
          if (para->address_ != address) {
            os_ << "\tmv\tx" << para->address_ << ", x" << address << '\n';
          }
        } else {
          EmitMem( s_ins, "x" + std::to_string(address), "sp", -static_cast<int32_t>(para->address_));
        }
      }
    } else {
      if (para->storage_type_ == kRegister) {
        auto [l_ins, _] = LoadStoreType(para->type_->base_type_);
        EmitMem( l_ins, "x" + std::to_string(para->address_), "sp", current_stack_ - address);
      }
    }
  }
  os_ << "\tcall\t" << node->function_name_ << '\n';

  // With per-register tracking, a0 hardware still holds the return value
  // after the call (no RestoreRegister runs).  Use a0 directly instead of
  // stashing to t0 — saves one mv per non-void call.
  bool has_result = !node->result_type_->IsEmpty();

  // Defer a-reg restore to the block terminator or on-demand via
  // EnsureARegValid.  Only a-regs that are actually accessed between
  // calls pay the restore cost; consecutive calls only save valid a-regs.
  ReloadConstCache();

  if (has_result) {
    RegToVariable(node->storage_type_, node->address_, "a0", node->result_type_->base_type_);
  }
}

void AssemblyGenerator::Visit(IRMoveInstructionNode *node) {
  DataMove(node->source_, node->storage_type_, node->address_, node->type_);
}

void AssemblyGenerator::Visit(IRSelectInstructionNode *node) {
  auto rs = VariableToReg(node->cond_, 0, "i1");
  auto rd = GetResultReg(node->storage_type_, node->address_, 1);
  EmitIA( "slti", rd, rs, 1);
  EmitIA( "xori", rd, rd, 1);
  RegToVariable(node->storage_type_, node->address_, rd, "i32");
}

void AssemblyGenerator::Visit(IRBlockNode *node) {
  cur_block_ = node->id_;
  if (node->id_ != 0 && referenced_blocks_.count(node->id_)) {
    os_ << ".L" << block_label_map_[node->id_] << ":\n";
  }
  for (auto &instruction : node->instructions_) {
    if (instruction->removed_) continue;
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
  total_stack = (total_stack + 15) / 16 * 16;  // 16-byte alignment for RISC-V ABI

  if (total_stack != 0) {
    EmitIA( "addi", "sp", "sp", -static_cast<int32_t>(total_stack));
  }

  current_stack_ = total_stack;
  current_func_name_ = node->name_;
  current_a_reg_used_ = node->a_reg_used_cnt_;
  for (bool &v : a_reg_valid_) v = true;
  ra_saved_ = false;
  current_variables_ = &node->variables_;
  variable_storage_ = &node->variable_storage_;

  cur_func_ = node;
  const_cache_.clear();

  // Estimate function code size: count non-removed IR instructions
  // plus const-cache overhead (ReloadConstCache emits ~2 li per call,
  // FlushSavedRegisters emits ~2 li per terminating block).  If the
  // function is very large, intra-function j/bnez may exceed the
  // RISC-V range limits (B-type: +/-4KB ≈ 1000 insns, jal: +/-1MB).
  // We switch to a long-jump pattern (lui+addi+jalr) when needed.
  {
    uint32_t total_ins = 0;
    uint32_t call_cnt = 0;
    for (auto &block : node->blocks_) {
      for (auto &ins : block->instructions_) {
        if (ins->removed_) continue;
        ++total_ins;
        if (dynamic_cast<IRCallInstructionNode *>(ins.get())) ++call_cnt;
      }
    }
    // For the long-jump threshold, use the pre-CSE instruction count
    // (if set).  CSE removes instructions, which can reduce the estimate
    // enough to disable long jumps when they're still needed.
    uint32_t threshold_ins = (node->pre_cse_ins_count_ > 0)
                                 ? node->pre_cse_ins_count_
                                 : total_ins;
    // Each IR insn becomes ~1-3 asm insns; const cache adds ~2 li per
    // call + ~2 li per block terminator.  The B-type branch limit is
    // only ±4KB (~1024 insns), so even moderate functions can exceed it
    // if blocks are far apart.  Use a generous threshold to be safe.
    uint32_t est_asm = threshold_ins * 2 + call_cnt * 2 + node->blocks_.size() * 2;
    large_function_ = (est_asm > 8000 || node->blocks_.size() > 100);

    // Map each block to its successor in layout order, for
    // eliminating redundant fall-through jumps.
    next_block_map_.clear();
    for (size_t i = 0; i + 1 < node->blocks_.size(); ++i) {
      next_block_map_[node->blocks_[i]->id_] = node->blocks_[i + 1]->id_;
    }

    // Pre-scan: collect referenced block IDs for label elision
    // and assign short global label IDs.
    referenced_blocks_.clear();
    block_label_map_.clear();
    for (auto &block : node->blocks_) {
      for (auto &ins : block->instructions_) {
        if (ins->removed_) continue;
        if (auto *b = dynamic_cast<IRBranchInstructionNode *>(ins.get())) {
          referenced_blocks_.insert(b->true_branch_);
          referenced_blocks_.insert(b->false_branch_);
        } else if (auto *j = dynamic_cast<IRJumpInstructionNode *>(ins.get())) {
          referenced_blocks_.insert(j->destination_);
        }
      }
    }
    for (auto id : referenced_blocks_) {
      block_label_map_[id] = next_label_id_++;
    }

    // Pre-scan: hoist large constants (>12-bit) into t3 and t4.
    // Count frequency of each constant to pick the most beneficial ones.
    {
      std::unordered_map<int64_t, uint32_t> const_freq;
      auto add_const = [&](int64_t v) {
        if (v < -2048 || v > 2047) const_freq[v]++;
      };
      auto scan = [&](const std::string &s) {
        if (!s.empty() && (s[0] == '-' || std::isdigit(s[0]))) {
          int64_t v = std::stoll(s);
          // Interpret as signed 32-bit if it looks like an unsigned 32-bit value
          // (e.g., "4294967256" is actually -40 as signed 32-bit).
          if (v > 0x7FFFFFFF && v <= 0xFFFFFFFF) {
            v = static_cast<int64_t>(static_cast<int32_t>(static_cast<uint32_t>(v)));
          }
          add_const(v);
        }
      };
      for (auto &block : node->blocks_) {
        for (auto &ins : block->instructions_) {
          if (ins->removed_) continue;
          if (auto *p = dynamic_cast<IRArithmeticInstructionNode *>(ins.get())) {
            scan(p->operand1_); scan(p->operand2_);
          } else if (auto *p = dynamic_cast<IRCompareInstructionNode *>(ins.get())) {
            scan(p->operand1_); scan(p->operand2_);
          } else if (auto *p = dynamic_cast<IRStoreConstInstructionNode *>(ins.get())) {
            add_const(p->value_);
          }
        }
      }
      // Also count stack offsets used by SaveRegister/FlushSavedRegisters.
      // These are used every time a call is made, so they can be very frequent.
      // Only add if the function actually has a stack frame and makes calls.
      if (current_stack_ > 0 && node->a_reg_used_cnt_ > 0) {
        for (uint32_t i = 0; i < node->a_reg_used_cnt_; ++i) {
          add_const(static_cast<int32_t>(current_stack_ - 8 - 8 * i));
        }
        add_const(static_cast<int32_t>(current_stack_ - 8 - 8 * node->a_reg_used_cnt_));  // ra offset
      }

      // Sort by frequency (descending) and pick top 2.
      std::vector<std::pair<int64_t, uint32_t>> sorted_consts(const_freq.begin(), const_freq.end());
      std::sort(sorted_consts.begin(), sorted_consts.end(),
                [](const auto &a, const auto &b) { return a.second > b.second; });

      // Pre-load the 2 most frequent large constants into t3/t4.
      uint32_t treg = 3;
      for (auto &[v, freq] : sorted_consts) {
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
      EmitMem( "sd", kRegisterName[reg_id], "sp", s_off);
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