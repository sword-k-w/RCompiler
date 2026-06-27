#include <iostream>
#include <cassert>
#include <set>
#include <functional>
#include "IR_visitor/assembly_generator/assembly_generator.h"
#include "IR_visitor/memory_allocator/memory_allocator.h"
#include "IR/struct_map.h"
#include "IR/function_map.h"
#include "codegen/register.h"
#include "codegen/instruction.h"

AssemblyGenerator::AssemblyGenerator(const std::string &builtin_begin, std::ostream &os,
                                   std::ostream *builtin_os) :
  builtin_begin_(builtin_begin), os_(os), builtin_os_(builtin_os) {}

void AssemblyGenerator::FlushDeferredStore() {
  // Deferred-store optimization removed — no-op retained as a safety
  // hook for future re-implementation.
}

void AssemblyGenerator::EmitMem(const std::string &type, const std::string &r, const std::string &rs1, int32_t imm) {
  if (!type.empty() && type[0] == 'l') BeforeWrite(r);
  PrintMem(os_, type, r, rs1, imm, &const_cache_);
}

void AssemblyGenerator::EmitIA(const std::string &type, const std::string &rd, const std::string &rs1, int32_t imm) {
  BeforeWrite(rd);
  PrintIA(os_, type, rd, rs1, imm, &const_cache_);
}

void AssemblyGenerator::EmitIStar(const std::string &type, const std::string &rd, const std::string &rs1, int32_t imm) {
  BeforeWrite(rd);
  PrintIStar(os_, type, rd, rs1, imm, &const_cache_);
}

void AssemblyGenerator::EmitR(const std::string &op, const std::string &rd,
                              const std::string &rs1, const std::string &rs2) {
  BeforeWrite(rd);
  os_ << "\t" << op << "\t" << rd << ", " << rs1 << ", " << rs2 << '\n';
}

void AssemblyGenerator::EmitUnary(const std::string &op, const std::string &rd,
                                  const std::string &rs) {
  BeforeWrite(rd);
  os_ << "\t" << op << "\t" << rd << ", " << rs << '\n';
}

void AssemblyGenerator::EmitLI(const std::string &rd, const std::string &val) {
  BeforeWrite(rd);
  os_ << "\tli\t" << rd << ", " << val << '\n';
}

void AssemblyGenerator::EmitLI(const std::string &rd, int64_t val) {
  BeforeWrite(rd);
  os_ << "\tli\t" << rd << ", " << val << '\n';
}

void AssemblyGenerator::EmitMV(const std::string &rd, const std::string &rs) {
  BeforeWrite(rd);
  os_ << "\tmv\t" << rd << ", " << rs << '\n';
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
    if (!name.empty() && (name[0] == '-' || std::isdigit(name[0]))) {
      int64_t val = std::stoll(name);
      if (val == 0) return "x0";
      auto it = const_cache_.find(val);
      if (it != const_cache_.end()) return it->second;
      EmitLI("t" + std::to_string(reg_id), name);
      return "t" + std::to_string(reg_id);
    }
    EmitLI("t" + std::to_string(reg_id), 0);
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
    EmitLI(reg, name);
  } else {
    if (address >= 10 && address <= 17) EnsureARegValid(address);
    if (!SameRegister(address, reg)) {
      EmitMV(reg, "x" + std::to_string(address));
    }
  }
}

void AssemblyGenerator::RegToVariable(StorageType storage_type, uint32_t address, const std::string &reg, const std::string &val_type) {
  assert(storage_type != kConst);
  if (storage_type == kMemory) {
    EmitMem( LoadStoreType(val_type).second, reg, "sp", current_stack_ - address);
  } else if (!SameRegister(address, reg)) {
    EmitMV("x" + std::to_string(address), reg);
  }
  if (address >= 10 && address <= 17) a_reg_valid_[address - 10] = true;
}

void AssemblyGenerator::SaveRegister() {
  // Save only valid a-regs — invalid ones already have the correct value in
  // their save slot (from an earlier save).  After the call all become invalid.
  for (uint32_t i = 0; i < current_a_reg_used_; ++i) {
    if (a_reg_valid_[i]) {
      EmitMem( "sd", "a" + std::to_string(i), "sp", save_area_base_ + 8 * i);
      a_reg_valid_[i] = false;
      a_slot_valid_[i] = true;  // Save slot now holds a valid value.
    }
  }
  // ra is saved at most once per block.
  if (!ra_saved_) {
    EmitMem( "sd", "ra", "sp", save_area_base_ + 8 * current_a_reg_used_);
    ra_saved_ = true;
    ra_slot_valid_ = true;
  }
}

void AssemblyGenerator::EnsureARegValid(uint32_t addr) {
  uint32_t idx = addr - 10;
  if (!a_reg_valid_[idx] && a_slot_valid_[idx]) {
    uint32_t save_off = save_area_base_ + 8 * idx;
    EmitMem( "ld", "x" + std::to_string(addr), "sp", save_off);
    a_reg_valid_[idx] = true;
  } else if (!a_reg_valid_[idx] && !a_slot_valid_[idx]) {
    // Trying to read an a-reg whose value was never saved — this is a bug
    // in the dead-register tracking.  Emit a load from the stale slot as
    // a fallback (the slot contains whatever was on the stack).
    uint32_t save_off = save_area_base_ + 8 * idx;
    EmitMem( "ld", "x" + std::to_string(addr), "sp", save_off);
    a_reg_valid_[idx] = true;
  }
}

void AssemblyGenerator::ReloadConstCache() {
  for (auto &[val, reg] : const_cache_) {
    EmitLI(reg, std::to_string(val));
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
    if (!a_reg_valid_[i] && a_slot_valid_[i]) {
      EmitMem( "ld", "a" + std::to_string(i), "sp", save_area_base_ + 8 * i);
      a_reg_valid_[i] = true;
    }
  }
  if (ra_saved_ && ra_slot_valid_) {
    EmitMem( "ld", "ra", "sp", save_area_base_ + 8 * current_a_reg_used_);
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
      EmitLI("x" + std::to_string(to_address), from);
      if (to_address >= 10 && to_address <= 17) a_reg_valid_[to_address - 10] = true;
    } else {
      EmitLI("t0", from);
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
      uint32_t sz = type->allocated_size_;
      if (sz > 0) {
        std::string dst_ptr = "t1";
        std::string src_ptr = "t2";
        EmitIA("addi", dst_ptr, "sp", current_stack_ - to_address);
        EmitIA("addi", src_ptr, "sp", current_stack_ - from_address);
        if (!EmitInlineCopy(dst_ptr, src_ptr, sz)) {
          SaveRegister();
          EmitIA( "addi", "a0", "sp", current_stack_ - to_address);
          EmitIA( "addi", "a1", "sp", current_stack_ - from_address);
          EmitLI("a2", sz);
          os_ << "\tcall\tbuiltin_memcpy\n";
          ReloadConstCache();
        }
      }
    }
  }
}

void AssemblyGenerator::DataMoveFromReg(const std::string &from, StorageType to_type, uint32_t to_address, std::shared_ptr<IRArrayNode> type) {
  auto [l_ins, s_ins] = LoadStoreType(type->base_type_);
  if (to_type == kRegister) {
    if (!SameRegister(to_address, from)) {
      EmitMV("x" + std::to_string(to_address), from);
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
        EmitIA(is_ptr ? "addi" : "addiw", rd, rs1, imm);
        RegToVariable(node->storage_type_, node->address_, rd, node->type_);
        return;
      }
      if (node->op_ == "-" && imm >= -2048 && imm <= 2047) {
        auto rs1 = VariableToReg(node->operand1_, 0, node->type_);
        EmitIA(is_ptr ? "addi" : "addiw", rd, rs1, -imm);
        RegToVariable(node->storage_type_, node->address_, rd, node->type_);
        return;
      }
    }

    // Also check operand1 for commutative operations (e.g., SCCP may
    // propagate a constant into the first operand position).
    {
      auto [op1_type, _] = GetVariableAddress(node->operand1_);
      if (op1_type == kConst) {
        int32_t imm = std::stoi(node->operand1_);
        // c + x == x + c (commutative): fold to addi rd, x, c.
        if (node->op_ == "+" && imm >= -2048 && imm <= 2047) {
          auto rs2 = VariableToReg(node->operand2_, 1, node->type_);
          EmitIA(is_ptr ? "addi" : "addiw", rd, rs2, imm);
          RegToVariable(node->storage_type_, node->address_, rd, node->type_);
          return;
        }
        // c - x has no single-instruction RISC-V immediate form (no subi),
        // so intentionally not folded here.
      }
    }

    // Strength reduction: replace mul/div/rem by power-of-2 constants with shifts.
    // x * (2^k) → slliw rd, x, k   (1 ins instead of li + mulw = 2 ins)
    // x / (2^k) → srliw rd, x, k   (unsigned only; 1 ins instead of li + divuw = 2 ins)
    // x % (2^k) → andi rd, x, (2^k - 1)  (unsigned only)
    {
      auto try_pow2 = [&](const std::string &const_op, const std::string &var_op,
                          uint32_t var_reg, bool commutative) {
        auto [op_type, _] = GetVariableAddress(const_op);
        if (op_type != kConst) return false;
        int32_t imm = std::stoi(const_op);
        if (imm < 1) return false;
        // Check power-of-2: (imm & (imm - 1)) == 0
        if ((imm & (imm - 1)) != 0) return false;
        int k = __builtin_ctz((unsigned)imm);  // log2(imm), k ≥ 0

        auto rs = VariableToReg(var_op, var_reg, node->type_);

        if (node->op_ == "*") {
          // x * (2^k) → slliw (or mv for k=0)
          if (k == 0) {
            EmitMV(rd, rs);
          } else {
            EmitIA("slliw", rd, rs, k);
          }
          RegToVariable(node->storage_type_, node->address_, rd, node->type_);
          return true;
        }
        if (node->op_ == "/" && !commutative) {
          // divisor is 2^k; only valid when const is operand2 (not commutative).
          if (node->is_unsigned_ && k > 0) {
            // x /u (2^k) → srliw rd, x, k
            EmitIA("srliw", rd, rs, k);
            RegToVariable(node->storage_type_, node->address_, rd, node->type_);
            return true;
          }
          // Signed division by power-of-2: use shift with sign correction.
          // For k ≥ 1:  sraiw t, rs, 31; srliw t, t, (32-k); addw t, rs, t; sraiw rd, t, k
          if (!node->is_unsigned_ && k > 0 && node->type_ == "i32") {
            std::string t = "t6";  // scratch
            EmitIA("sraiw", t, rs, 31);
            EmitIA("srliw", t, t, 32 - k);
            EmitR("addw", t, rs, t);
            EmitIA("sraiw", rd, t, k);
            RegToVariable(node->storage_type_, node->address_, rd, node->type_);
            return true;
          }
        }
        if (node->op_ == "%" && !commutative) {
          // divisor is 2^k; unsigned only.
          if (node->is_unsigned_ && k > 0 && imm <= 2047) {
            // x %u (2^k) → andi rd, x, (2^k - 1)
            EmitIA("andi", rd, rs, imm - 1);
            RegToVariable(node->storage_type_, node->address_, rd, node->type_);
            return true;
          }
          // For larger 2^k where (2^k - 1) exceeds 12-bit signed immediate,
          // fall through to the general remuw path (still correct, just not
          // strength-reduced).
        }
        return false;
      };

      // Try operand2 as constant (the divisor/multiplier).
      if (try_pow2(node->operand2_, node->operand1_, 0, false)) return;
      // For commutative ops (*), also try operand1 as constant.
      if (node->op_ == "*") {
        if (try_pow2(node->operand1_, node->operand2_, 1, true)) return;
      }
    }
  }

  auto rs1 = VariableToReg(node->operand1_, 0, node->type_);
  auto rs2 = VariableToReg(node->operand2_, 1, node->type_);
  std::string op;
  if (node->op_ == "+") {
    op = is_ptr ? "add" : "addw";
  } else if (node->op_ == "-") {
    op = is_ptr ? "sub" : "subw";
  } else if (node->op_ == "*") {
    op = "mulw";
  } else if (node->op_ == "/") {
    op = "div";
    if (node->is_unsigned_) op += "u";
    op += "w";
  } else if (node->op_ == "%") {
    op = "rem";
    if (node->is_unsigned_) op += "u";
    op += "w";
  } else if (node->op_ == "<<") {
    op = "sllw";
  } else if (node->op_ == ">>") {
    op = "sraw";
  } else if (node->op_ == "&") {
    op = "and";
  } else if (node->op_ == "|") {
    op = "or";
  } else if (node->op_ == "^") {
    op = "xor";
  } else {
    std::cerr << "Error! : unexpected op " << node->op_ << '\n';
    exit(-1);
  }
  EmitR(op, rd, rs1, rs2);
  RegToVariable(node->storage_type_, node->address_, rd, node->type_);
}

void AssemblyGenerator::Visit(IRNegationInstructionNode *node) {
  auto [op_type, op_addr] = GetVariableAddress(node->operand_);
  if (node->is_minus_ && op_type == kConst) {
    auto rd = GetResultReg(node->storage_type_, node->address_, 1);
    int32_t val = std::stoi(node->operand_);
    EmitLI(rd, -val);
    RegToVariable(node->storage_type_, node->address_, rd, node->type_);
    return;
  }
  auto rs = VariableToReg(node->operand_, 0, node->type_);
  auto rd = GetResultReg(node->storage_type_, node->address_, 1);
  if (node->is_minus_) {
    EmitUnary("neg", rd, rs);
  } else {
    if (node->type_ == "i1") {
      EmitIA("xori", rd, rs, 1);
    } else {
      EmitUnary("not", rd, rs);
    }
  }
  RegToVariable(node->storage_type_, node->address_, rd, node->type_);
}

void AssemblyGenerator::Visit(IRBranchInstructionNode *node) {
  // Leaving the block: hardware a-regs/ra must be valid for successors.
  FlushSavedRegisters();

  auto label = [&](uint32_t id) {
    return ".L" + std::to_string(block_label_map_[id]);
  };

  // Compare→branch fusion path: emit one of beq/bne/blt/bge/bltu/bgeu
  // directly from the (skipped) compare's operands, saving the slt+xori+bnez
  // sequence (1-2 instructions per compare).
  if (pending_fused_cmp_ != nullptr && pending_fused_cmp_->result_ == node->condition_) {
    auto *cmp = pending_fused_cmp_;
    pending_fused_cmp_ = nullptr;

    // Map IR compare op → (branch op, swap operands).  The condition we want
    // is "branch to true_label iff (operand1 OP operand2)".
    //   kSgt: a > b  ⇔  b < a   → blt swapped
    //   kSle: a <= b ⇔  b >= a  → bge swapped
    //   kUgt / kUle: analogous for unsigned.
    std::string br_op;
    bool swap = false;
    switch (cmp->op_) {
      case IRCompareInstructionNode::kEq:  br_op = "beq";  break;
      case IRCompareInstructionNode::kNe:  br_op = "bne";  break;
      case IRCompareInstructionNode::kSlt: br_op = "blt";  break;
      case IRCompareInstructionNode::kSge: br_op = "bge";  break;
      case IRCompareInstructionNode::kSgt: br_op = "blt";  swap = true; break;
      case IRCompareInstructionNode::kSle: br_op = "bge";  swap = true; break;
      case IRCompareInstructionNode::kUlt: br_op = "bltu"; break;
      case IRCompareInstructionNode::kUge: br_op = "bgeu"; break;
      case IRCompareInstructionNode::kUgt: br_op = "bltu"; swap = true; break;
      case IRCompareInstructionNode::kUle: br_op = "bgeu"; swap = true; break;
    }

    // Inversion (taken-on-false) used when true_branch is the next block in
    // layout order — we emit the inverted branch to false_branch and let
    // true_branch fall through.
    auto invert = [](const std::string &op) -> std::string {
      if (op == "beq")  return "bne";
      if (op == "bne")  return "beq";
      if (op == "blt")  return "bge";
      if (op == "bge")  return "blt";
      if (op == "bltu") return "bgeu";
      if (op == "bgeu") return "bltu";
      return op;
    };

    std::string op1_name = cmp->operand1_;
    std::string op2_name = cmp->operand2_;
    if (swap) std::swap(op1_name, op2_name);
    auto rs1 = VariableToReg(op1_name, 0, cmp->type_);
    auto rs2 = VariableToReg(op2_name, 1, cmp->type_);

    if (!large_function_) {
      auto get_next = [&]() -> uint32_t {
        auto it = next_block_map_.find(cur_block_);
        return (it != next_block_map_.end()) ? it->second : UINT32_MAX;
      };
      uint32_t nid = get_next();
      if (node->false_branch_ == nid) {
        os_ << "\t" << br_op << "\t" << rs1 << ", " << rs2 << ", " << label(node->true_branch_) << '\n';
        return;
      }
      if (node->true_branch_ == nid) {
        os_ << "\t" << invert(br_op) << "\t" << rs1 << ", " << rs2 << ", " << label(node->false_branch_) << '\n';
        return;
      }
      os_ << "\t" << br_op << "\t" << rs1 << ", " << rs2 << ", " << label(node->true_branch_) << '\n';
      os_ << "\tj " << label(node->false_branch_) << '\n';
      return;
    }

    // Large-function: trampoline via local label 1f.
    auto true_lbl  = label(node->true_branch_);
    auto false_lbl = label(node->false_branch_);
    os_ << "\t" << br_op << "\t" << rs1 << ", " << rs2 << ", 1f\n";
    os_ << "\tlui\tt5, %hi(" << false_lbl << ")\n";
    os_ << "\taddi\tt5, t5, %lo(" << false_lbl << ")\n";
    os_ << "\tjalr\tx0, t5, 0\n";
    os_ << "1:\n";
    os_ << "\tlui\tt5, %hi(" << true_lbl << ")\n";
    os_ << "\taddi\tt5, t5, %lo(" << true_lbl << ")\n";
    os_ << "\tjalr\tx0, t5, 0\n";
    return;
  }

  // No fused compare: load the i1 condition and branch on zero/non-zero.
  auto rs = VariableToReg(node->condition_, 0, "i1");

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

static constexpr uint32_t kInlineCopyThreshold = 128;

bool AssemblyGenerator::EmitInlineCopy(const std::string &dst_ptr,
                                        const std::string &src_ptr,
                                        uint32_t size) {
  if (size > kInlineCopyThreshold) return false;
  if (size == 0) return true;
  // Pick a data register that does not conflict with either pointer.
  // VariableToReg may return "t0" for kMemory pointers; t0/t1/t2 are pure
  // scratch and never hold promoted variables across instructions.
  std::string data_reg = "t0";
  if (dst_ptr == "t0" || src_ptr == "t0") data_reg = "t2";
  if (dst_ptr == data_reg || src_ptr == data_reg) data_reg = "t1";

  uint32_t offset = 0;
  while (offset + 8 <= size) {
    EmitMem("ld", data_reg, src_ptr, static_cast<int32_t>(offset));
    EmitMem("sd", data_reg, dst_ptr, static_cast<int32_t>(offset));
    offset += 8;
  }
  if (offset + 4 <= size) {
    EmitMem("lwu", data_reg, src_ptr, static_cast<int32_t>(offset));
    EmitMem("sw", data_reg, dst_ptr, static_cast<int32_t>(offset));
    offset += 4;
  }
  if (offset + 2 <= size) {
    EmitMem("lhu", data_reg, src_ptr, static_cast<int32_t>(offset));
    EmitMem("sh", data_reg, dst_ptr, static_cast<int32_t>(offset));
    offset += 2;
  }
  if (offset + 1 <= size) {
    EmitMem("lbu", data_reg, src_ptr, static_cast<int32_t>(offset));
    EmitMem("sb", data_reg, dst_ptr, static_cast<int32_t>(offset));
    offset += 1;
  }
  return true;
}

void AssemblyGenerator::Visit(IRLoadInstructionNode *node) {
  auto ptr_reg = VariableToReg(node->pointer_, 0, "ptr");
  if (node->storage_type_ == kRegister) {
    auto [l_ins, _] = LoadStoreType(node->type_->base_type_);
    EmitMem( l_ins, "x" + std::to_string(node->address_), ptr_reg, 0);
    if (node->address_ >= 10 && node->address_ <= 17) a_reg_valid_[node->address_ - 10] = true;
  } else {
    uint32_t size = node->type_->allocated_size_;
    std::string dst_ptr = "t1";
    EmitIA("addi", dst_ptr, "sp", current_stack_ - node->address_);
    if (!EmitInlineCopy(dst_ptr, ptr_reg, size)) {
      SaveRegister();
      EmitIA( "addi", "a0", "sp", current_stack_ - node->address_);
      bool flag = true;
      for (uint32_t i = 10; i < 18; ++i) {
        if (SameRegister(i, ptr_reg)) {
          EnsureARegValid(i);
          EmitMV("a1", "x" + std::to_string(i));
          flag = false;
        }
      }
      if (flag) {
        EmitMV("a1", ptr_reg);
      }
      EmitLI("a2", size);
      os_ << "\tcall\tbuiltin_memcpy\n";
      ReloadConstCache();
    }
  }
}

void AssemblyGenerator::Visit(IRStoreVariableInstructionNode *node) {
  // After mem2reg, this type of instruction may store const
  auto ptr_reg = VariableToReg(node->pointer_, 0, "ptr");
  auto [type, address] = GetVariableAddress(node->value_);
  if (type == kConst) {
    std::string val_reg = (ptr_reg == "t1") ? "t0" : "t1";
    EmitLI(val_reg, node->value_);
    auto [_, s_ins] = LoadStoreType(node->type_->base_type_);
    EmitMem( s_ins, val_reg, ptr_reg, 0);
  } else if (type == kRegister) {
    if (address >= 10 && address <= 17) EnsureARegValid(address);
    auto [_, s_ins] = LoadStoreType(node->type_->base_type_);
    EmitMem( s_ins, "x" + std::to_string(address), ptr_reg, 0);
  } else {
    uint32_t size = node->type_->allocated_size_;
    if (size == 0) return;
    std::string src_ptr = "t1";
    EmitIA("addi", src_ptr, "sp", current_stack_ - address);
    if (!EmitInlineCopy(ptr_reg, src_ptr, size)) {
      SaveRegister();
      bool flag = true;
      for (uint32_t i = 10; i < 18; ++i) {
        if (SameRegister(i, ptr_reg)) {
          EnsureARegValid(i);
          EmitMV("a0", "x" + std::to_string(i));
          flag = false;
        }
      }
      if (flag) {
        EmitMV("a0", ptr_reg);
      }
      EmitIA( "addi", "a1", "sp", current_stack_ - address);
      EmitLI("a2", size);
      os_ << "\tcall\tbuiltin_memcpy\n";
      ReloadConstCache();
    }
  }
}

void AssemblyGenerator::Visit(IRStoreConstInstructionNode *node) {
  auto ptr_reg = VariableToReg(node->pointer_, 0, "ptr");
  std::string val_reg = (ptr_reg == "t1") ? "t0" : "t1";
  EmitLI(val_reg, node->value_);
  auto [_, s_ins] = LoadStoreType(node->type_);
  EmitMem( s_ins, val_reg, ptr_reg, 0);
}

uint32_t AssemblyGenerator::ComputeGEPOffset(IRGetElementPtrInstructionNode *node) {
  uint32_t offset = 0;
  if (node->type_->length_.empty()) {
    uint32_t align = 1;
    auto struct_node = StructMap::Instance().Query(node->type_->base_type_);
    for (uint32_t i = 0; i < node->index_; ++i) {
      auto memb_align = struct_node->members_[i]->align_;
      if (memb_align > align) align = memb_align;
      offset = (offset + memb_align - 1) / memb_align * memb_align;
      offset += struct_node->members_[i]->allocated_size_;
    }
    offset = (offset + struct_node->members_[node->index_]->align_ - 1)
           / struct_node->members_[node->index_]->align_
           * struct_node->members_[node->index_]->align_;
  } else {
    offset = node->type_->allocated_size_ / node->type_->length_[0] * node->index_;
  }
  return offset;
}

void AssemblyGenerator::Visit(IRGetElementPtrInstructionNode *node) {
  auto ptr_reg = VariableToReg(node->ptrval_, 0, "ptr");
  auto rd = GetResultReg(node->storage_type_, node->address_, 1);
  uint32_t offset = ComputeGEPOffset(node);
  if (offset == 0) {
    EmitMV(rd, ptr_reg);
  } else
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
    EmitR("add", rd, ptr_reg, index_reg);
  } else if ((elem_size & (elem_size - 1)) == 0) {
    // Power of 2: use shift
    uint32_t shift = __builtin_ctz(elem_size);
    EmitIStar("slli", "t1", index_reg, shift);
    EmitR("add", rd, ptr_reg, "t1");
  } else {
    EmitLI("t1", static_cast<int32_t>(elem_size));
    EmitR("mul", "t2", "t1", index_reg);
    EmitR("add", rd, ptr_reg, "t2");
  }
  RegToVariable(node->storage_type_, node->address_, rd, "ptr");
}

void AssemblyGenerator::Visit(IRCompareInstructionNode *node) {
  // Compare→branch fusion: skip emission entirely.  The immediately
  // following branch will emit a single `blt/bge/beq/bne/bltu/bgeu`
  // straight from this compare's operands.
  if (fused_compares_.count(node) > 0) {
    pending_fused_cmp_ = node;
    return;
  }

  auto [type1, addr1] = GetVariableAddress(node->operand1_);
  auto [type2, addr2] = GetVariableAddress(node->operand2_);
  bool op1c = (type1 == kConst);
  bool op2c = (type2 == kConst);
  auto rd = GetResultReg(node->storage_type_, node->address_, 2);
  auto store_result = [&]() {
    RegToVariable(node->storage_type_, node->address_, rd, "i1");
  };

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
    EmitIA("addi", "t0", rs, neg);
    if (is_eq)
      EmitIA("sltiu", rd, "t0", 1);
    else
      EmitR("sltu", rd, "x0", "t0");
    return true;
  };

  // Peephole: ==0, !=0 — load only the non-const operand.
  if (node->op_ == IRCompareInstructionNode::kEq) {
    if (op1c && node->operand1_ == "0") {
      auto rs2 = VariableToReg(node->operand2_, 1, node->type_);
      EmitIA("sltiu", rd, rs2, 1);
      store_result();
      return;
    }
    if (op2c && node->operand2_ == "0") {
      auto rs1 = VariableToReg(node->operand1_, 0, node->type_);
      EmitIA("sltiu", rd, rs1, 1);
      store_result();
      return;
    }
    // Peephole: ==small_const — fold into addi (avoids li).
    if (op2c && foldConstCmp(true, node->operand2_, node->operand1_, 0)) {
      store_result();
      return;
    }
    if (op1c && foldConstCmp(true, node->operand1_, node->operand2_, 1)) {
      store_result();
      return;
    }
  }
  if (node->op_ == IRCompareInstructionNode::kNe) {
    if (op1c && node->operand1_ == "0") {
      auto rs2 = VariableToReg(node->operand2_, 1, node->type_);
      EmitR("sltu", rd, "x0", rs2);
      store_result();
      return;
    }
    if (op2c && node->operand2_ == "0") {
      auto rs1 = VariableToReg(node->operand1_, 0, node->type_);
      EmitR("sltu", rd, "x0", rs1);
      store_result();
      return;
    }
    // Peephole: !=small_const — fold into addi (avoids li).
    if (op2c && foldConstCmp(false, node->operand2_, node->operand1_, 0)) {
      store_result();
      return;
    }
    if (op1c && foldConstCmp(false, node->operand1_, node->operand2_, 1)) {
      store_result();
      return;
    }
  }

  // General case: load both operands.
  auto rs1 = VariableToReg(node->operand1_, 0, node->type_);
  auto rs2 = VariableToReg(node->operand2_, 1, node->type_);

  // Use t0 as the only aux register (was t0+t3, now just t0 frees t3).
  if (node->op_ == IRCompareInstructionNode::kEq) {
    EmitR("sub", "t0", rs1, rs2);
    EmitIA("sltiu", rd, "t0", 1);
  } else if (node->op_ == IRCompareInstructionNode::kNe) {
    EmitR("sub", "t0", rs1, rs2);
    EmitR("sltu", rd, "x0", "t0");
  } else if (node->op_ == IRCompareInstructionNode::kUgt) {
    EmitR("sltu", rd, rs2, rs1);
  } else if (node->op_ == IRCompareInstructionNode::kUge) {
    EmitR("sltu", "t0", rs1, rs2);
    EmitIA("xori", rd, "t0", 1);
  } else if (node->op_ == IRCompareInstructionNode::kUlt) {
    EmitR("sltu", rd, rs1, rs2);
  } else if (node->op_ == IRCompareInstructionNode::kUle) {
    EmitR("sltu", "t0", rs2, rs1);
    EmitIA("xori", rd, "t0", 1);
  } else if (node->op_ == IRCompareInstructionNode::kSgt) {
    EmitR("slt", rd, rs2, rs1);
  } else if (node->op_ == IRCompareInstructionNode::kSge) {
    EmitR("slt", "t0", rs1, rs2);
    EmitIA("xori", rd, "t0", 1);
  } else if (node->op_ == IRCompareInstructionNode::kSlt) {
    EmitR("slt", rd, rs1, rs2);
  } else {
    EmitR("slt", "t0", rs2, rs1);
    EmitIA("xori", rd, "t0", 1);
  }
  store_result();
}

void AssemblyGenerator::Visit(IRArgumentNode *node) {}

void AssemblyGenerator::Visit(IRCallInstructionNode *node) {
  auto function_node = FunctionMap::Instance().Query(node->function_name_);
  auto size = node->arguments_.size();

  // Inline small builtin_memcpy calls: extract dest, src, size and emit
  // ld/sd pairs directly, avoiding the call and register save/restore.
  if (node->function_name_ == "builtin_memcpy" && size == 3) {
    auto [sz_type, sz_addr] = GetVariableAddress(node->arguments_[2]->value_);
    if (sz_type == kConst) {
      uint32_t copy_sz = static_cast<uint32_t>(std::stoul(node->arguments_[2]->value_));
      if (copy_sz > 0 && copy_sz <= kInlineCopyThreshold) {
        // Helper to resolve a pointer argument to a register string.
        auto resolve_ptr = [&](const std::string &val, const std::string &tmp_reg) -> std::string {
          auto [ty, addr] = GetVariableAddress(val);
          if (ty == kRegister) {
            if (addr >= 10 && addr <= 17) EnsureARegValid(addr);
            return "x" + std::to_string(addr);
          } else if (ty == kConst) {
            // Constant address: load it into the temp register
            EmitLI(tmp_reg, val);
            return tmp_reg;
          } else {
            // kMemory: the pointer value is stored on the stack — load it
            EmitMem("ld", tmp_reg, "sp", current_stack_ - addr);
            return tmp_reg;
          }
        };
        std::string dst_ptr = resolve_ptr(node->arguments_[0]->value_, "t1");
        std::string src_ptr = resolve_ptr(node->arguments_[1]->value_, "t2");
        EmitInlineCopy(dst_ptr, src_ptr, copy_sz);
        return;
      }
    }
  }

  // Dead a-reg optimization: a-regs that hold no live values at this call
  // don't need saving.  We only skip a-regs that are *already* invalid
  // (previously saved): if an a-reg is valid (fresh value), we must save it
  // to establish a valid save slot before the call clobbers the hardware register.
  // Subsequent calls in the same block can then skip the re-save for dead a-regs.
  // For the first call in a block, all a-regs start valid so all are saved.
  // The dead_mask was computed by RegAlloc from instruction-level backwards
  // liveness: bit i is set iff no variable assigned to phys_reg(10+i)
  // is in liveOut(call).
  uint32_t dead_mask = node->dead_a_regs_mask_;
  for (uint32_t i = 0; i < current_a_reg_used_ && i < 8; ++i) {
    if ((dead_mask & (1u << i)) && !a_reg_valid_[i]) {
      // Already saved by a previous call — keep it invalid so SaveRegister
      // skips it, avoiding a redundant save.
      // a_reg_valid_[i] stays false.
    }
  }

  SaveRegister();

  // operate the memory to memory in advance, using builtin_memcpy
  for (uint32_t i = 0; i < size; ++i) {
    auto para = function_node->parameters_[i];
    auto [type, address] = GetVariableAddress(node->arguments_[i]->value_);
    if (type == kMemory && para->storage_type_ == kMemory) {
      uint32_t sz = para->type_->allocated_size_;
      if (sz == 0) continue;
      std::string dst_ptr = "t1";
      std::string src_ptr = "t2";
      EmitIA("addi", dst_ptr, "sp", -static_cast<int32_t>(para->address_));
      EmitIA("addi", src_ptr, "sp", current_stack_ - address);
      if (!EmitInlineCopy(dst_ptr, src_ptr, sz)) {
        EmitIA( "addi", "a0", "sp", -static_cast<int32_t>(para->address_));
        EmitIA( "addi", "a1", "sp", current_stack_ - address);
        EmitLI("a2", sz);
        os_ << "\tcall\tbuiltin_memcpy\n";
        ReloadConstCache();
      }
    }
  }

  for (uint32_t i = 0; i < size; ++i) {
    auto para = function_node->parameters_[i];
    auto [type, address] = GetVariableAddress(node->arguments_[i]->value_);
    if (type == kConst) {
      if (para->storage_type_ == kRegister) {
        EmitLI("x" + std::to_string(para->address_), node->arguments_[i]->value_);
      } else {
        EmitLI("t0", node->arguments_[i]->value_);
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
        uint32_t save_off = save_area_base_ + 8 * (address - 10);
        if (para->storage_type_ == kRegister) {
          EmitMem( l_ins, "x" + std::to_string(para->address_), "sp", save_off);
        } else {
          EmitMem( l_ins, "t0", "sp", save_off);
          EmitMem( s_ins, "t0", "sp", -static_cast<int32_t>(para->address_));
        }
      } else {
        if (para->storage_type_ == kRegister) {
          if (para->address_ != address) {
            EmitMV("x" + std::to_string(para->address_), "x" + std::to_string(address));
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
  pending_fused_cmp_ = nullptr;
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

  // Frame layout (low addresses ↑):
  //   sp + 0..s_save                              s-reg saves
  //   sp + s_save..s_save + a_save                a-reg/ra save area (non-leaf)
  //   sp + s_save + a_save..total_stack           variables and allocas
  //
  // The 72-byte top-of-frame reservation (where saves used to live) is still
  // counted inside the variable address space, so parameter addresses are
  // unchanged.  Adding a-reg saves at the bottom costs an extra `a_save`
  // bytes per non-leaf function, but every save/restore drops from 3
  // instructions (`li t6, off; add t6, t6, sp; sd 0(t6)`) to 1 (`sd off(sp)`).
  uint32_t s_save = 8 * node->used_s_regs_.size();
  uint32_t a_save = node->has_calls_ ? 8 * (node->a_reg_used_cnt_ + 1) : 0;
  uint32_t total_stack = node->stack_size_ + s_save + a_save;
  total_stack = (total_stack + 15) / 16 * 16;  // 16-byte alignment for RISC-V ABI

  if (total_stack != 0) {
    EmitIA( "addi", "sp", "sp", -static_cast<int32_t>(total_stack));
  }

  current_stack_ = total_stack;
  current_func_name_ = node->name_;
  current_a_reg_used_ = node->a_reg_used_cnt_;
  // Save slots live in the [s_save, s_save + a_save] window — cheap to
  // access via `sd/ld off(sp)` since these offsets fit in a 12-bit immediate.
  save_area_base_ = s_save;
  for (bool &v : a_reg_valid_) v = true;
  for (bool &v : a_slot_valid_) v = false;
  ra_saved_ = false;
  ra_slot_valid_ = false;
  current_variables_ = &node->variables_;
  variable_storage_ = &node->variable_storage_;

  cur_func_ = node;
  const_cache_.clear();
  fused_compares_.clear();
  pending_fused_cmp_ = nullptr;

  // Compare→branch fusion pre-scan: for each block whose terminator is a
  // branch, check whether the preceding non-removed instruction is a compare
  // that defines the branch's condition.  If so — and the compare's result
  // is in kMemory (otherwise regalloc already gave it a real register) and
  // has exactly one use in the entire function — we can skip the store/load
  // pair and let the compare's t-reg result flow directly into the branch.
  {
    std::unordered_map<std::string, uint32_t> use_count;
    auto bump = [&](const std::string &n) {
      if (!n.empty() && n[0] == '%') ++use_count[n];
    };
    for (auto &block : node->blocks_) {
      for (auto &ins : block->instructions_) {
        if (ins->removed_) continue;
        IRInstructionNode *i = ins.get();
        if (auto *p = dynamic_cast<IRArithmeticInstructionNode *>(i)) {
          bump(p->operand1_); bump(p->operand2_);
        } else if (auto *p = dynamic_cast<IRNegationInstructionNode *>(i)) {
          bump(p->operand_);
        } else if (auto *p = dynamic_cast<IRBranchInstructionNode *>(i)) {
          bump(p->condition_);
        } else if (auto *p = dynamic_cast<IRReturnInstructionNode *>(i)) {
          if (!p->type_->IsEmpty()) bump(p->name_);
        } else if (auto *p = dynamic_cast<IRLoadInstructionNode *>(i)) {
          bump(p->pointer_);
        } else if (auto *p = dynamic_cast<IRStoreVariableInstructionNode *>(i)) {
          bump(p->pointer_); bump(p->value_);
        } else if (auto *p = dynamic_cast<IRStoreConstInstructionNode *>(i)) {
          bump(p->pointer_);
        } else if (auto *p = dynamic_cast<IRGetElementPtrInstructionNode *>(i)) {
          bump(p->ptrval_);
        } else if (auto *p = dynamic_cast<IRGetElementPtrPrimeInstructionNode *>(i)) {
          bump(p->ptrval_); bump(p->index_);
        } else if (auto *p = dynamic_cast<IRCompareInstructionNode *>(i)) {
          bump(p->operand1_); bump(p->operand2_);
        } else if (auto *p = dynamic_cast<IRCallInstructionNode *>(i)) {
          for (auto &arg : p->arguments_) bump(arg->value_);
        } else if (auto *p = dynamic_cast<IRMoveInstructionNode *>(i)) {
          bump(p->source_);
        } else if (auto *p = dynamic_cast<IRSelectInstructionNode *>(i)) {
          bump(p->cond_);
        }
      }
    }
    // Helper: look up the post-RegAlloc storage of a variable name.
    auto var_storage = [&](const std::string &name) -> std::pair<StorageType, uint32_t> {
      auto it = node->variable_storage_.find(name);
      if (it == node->variable_storage_.end()) return {kConst, 0};
      return it->second;
    };

    for (auto &block : node->blocks_) {
      // Collect all non-removed instructions in this block.
      std::vector<IRInstructionNode *> live_ins;
      for (auto &ins : block->instructions_) {
        if (!ins->removed_) live_ins.push_back(ins.get());
      }
      if (live_ins.size() < 2) continue;

      // The last instruction must be a branch.
      auto *br = dynamic_cast<IRBranchInstructionNode *>(live_ins.back());
      if (!br) continue;

      // Walk backward past any phi-elimination moves looking for the compare
      // that defines the branch's condition.
      IRCompareInstructionNode *cmp = nullptr;
      int cmp_idx = -1;
      for (int idx = static_cast<int>(live_ins.size()) - 2; idx >= 0; --idx) {
        auto *i = live_ins[idx];
        if (auto *c = dynamic_cast<IRCompareInstructionNode *>(i)) {
          cmp = c;
          cmp_idx = idx;
          break;
        }
        if (dynamic_cast<IRMoveInstructionNode *>(i)) continue;
        // Any other instruction blocks fusion.
        break;
      }
      if (!cmp) continue;
      if (cmp->result_ != br->condition_) continue;
      auto it = use_count.find(cmp->result_);
      if (it == use_count.end() || it->second != 1) continue;

      // Safety check for intervening moves: when we defer the compare to the
      // branch site, its operands must still hold their pre-compare values.
      // A phi move with destination = operand's name is obviously bad, but
      // after RegAlloc/coalescing two distinct SSA names can share the same
      // physical register — so we check at the physical-register level too.
      auto op1_store = var_storage(cmp->operand1_);
      auto op2_store = var_storage(cmp->operand2_);
      bool intervening_safe = true;
      for (int idx = cmp_idx + 1; idx < static_cast<int>(live_ins.size()) - 1; ++idx) {
        auto *mv = dynamic_cast<IRMoveInstructionNode *>(live_ins[idx]);
        if (!mv) { intervening_safe = false; break; }
        // Direct name match.
        if (mv->result_ == cmp->operand1_ || mv->result_ == cmp->operand2_) {
          intervening_safe = false; break;
        }
        // Physical-register aliasing.
        auto mv_store = var_storage(mv->result_);
        if (mv_store.first == kRegister) {
          if (op1_store.first == kRegister && op1_store.second == mv_store.second) {
            intervening_safe = false; break;
          }
          if (op2_store.first == kRegister && op2_store.second == mv_store.second) {
            intervening_safe = false; break;
          }
        } else if (mv_store.first == kMemory) {
          if (op1_store.first == kMemory && op1_store.second == mv_store.second) {
            intervening_safe = false; break;
          }
          if (op2_store.first == kMemory && op2_store.second == mv_store.second) {
            intervening_safe = false; break;
          }
        }
      }
      if (!intervening_safe) continue;

      fused_compares_.insert(cmp);
    }
  }

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
    // Each IR insn becomes ~1-3 asm insns; const cache adds ~2 li per
    // call + ~2 li per block terminator.  The B-type branch limit is
    // only ±4KB (~1024 insns), so even moderate functions can exceed it
    // if blocks are far apart.  Use total_ins (post-optimization) with
    // a multiplier of 3 to be conservative.  (Previously used pre-CSE
    // count which inflated the estimate for functions with unrolled
    // array initialization that later becomes memset loops.)
    uint32_t est_asm = total_ins * 3 + call_cnt * 3 + node->blocks_.size() * 3;
    large_function_ = (est_asm > 8000 || node->blocks_.size() > 500);

    // Reorder blocks to maximize fall-through, minimizing explicit
    // jumps.  A DFS traversal from the entry block places each block
    // immediately after its predecessor when the predecessor's
    // terminator targets it.  For branches, the false target is
    // preferred as next (so bnez falls through when the condition
    // is false).  The jump-elimination logic in next_block_map_
    // and Visit(IRJumpInstructionNode)/Visit(IRBranchInstructionNode)
    // then removes the now-redundant jumps.
    {
      // Index blocks by id for O(1) lookup.
      std::unordered_map<uint32_t, std::shared_ptr<IRBlockNode>> block_by_id;
      for (auto &b : node->blocks_) block_by_id[b->id_] = b;

      std::vector<std::shared_ptr<IRBlockNode>> reordered;
      std::set<uint32_t> placed;

      // DFS: place `id` and then recurse on its preferred successor.
      std::function<void(uint32_t)> place = [&](uint32_t id) {
        if (placed.count(id)) return;
        placed.insert(id);
        if (block_by_id.count(id))
          reordered.push_back(block_by_id[id]);

        auto &blk = block_by_id[id];
        if (!blk) return;

        // Find the terminator to determine successors.
        IRJumpInstructionNode    *jmp = nullptr;
        IRBranchInstructionNode  *br  = nullptr;
        for (auto &ins : blk->instructions_) {
          if (ins->removed_) continue;
          if (!jmp) jmp = dynamic_cast<IRJumpInstructionNode *>(ins.get());
          if (!br)  br  = dynamic_cast<IRBranchInstructionNode *>(ins.get());
          if (dynamic_cast<IRReturnInstructionNode *>(ins.get())) return;
        }

        if (jmp) {
          // Jump: place the target next → fall-through.
          place(jmp->destination_);
        } else if (br) {
          // Branch: place false target next so bnez/beqz falls through
          // when the condition is false (the common case for if-then).
          place(br->false_branch_);
          place(br->true_branch_);
        }
      };

      place(0);

      // Append any remaining blocks not reached by the DFS
      // (e.g. unreachable blocks, or blocks only reachable via
      //  backwards edges that the DFS placed out of order).
      for (auto &b : node->blocks_) {
        if (!placed.count(b->id_)) {
          reordered.push_back(b);
        }
      }

      node->blocks_ = std::move(reordered);
    }

    // Map each block to its successor in layout order, for
    // eliminating redundant fall-through jumps.
    // Skip blocks whose instructions have all been removed
    // (merged by EliminateEmptyBlocks).
    auto empty_block = [](const std::shared_ptr<IRBlockNode> &b) {
      for (auto &ins : b->instructions_)
        if (!ins->removed_) return false;
      return true;
    };
    next_block_map_.clear();
    for (size_t i = 0; i < node->blocks_.size(); ++i) {
      if (empty_block(node->blocks_[i])) continue;
      size_t j = i + 1;
      while (j < node->blocks_.size() && empty_block(node->blocks_[j])) ++j;
      if (j < node->blocks_.size())
        next_block_map_[node->blocks_[i]->id_] = node->blocks_[j]->id_;
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
      // For a name that resolves to a kMemory variable, count the stack
      // offset (current_stack_ - addr) that any ld/sd against that slot
      // will emit.  This is what makes hoisting these offsets into t3/t4
      // pay off: each cached offset avoids `li t6, X; add t6, t6, sp`
      // (2 ins) at every load and store of the slot, replaced by a single
      // `add t6, t3, sp`.
      auto scan_var = [&](const std::string &s) {
        if (s.empty() || s[0] != '%') return;
        auto it = node->variable_storage_.find(s);
        if (it == node->variable_storage_.end()) return;
        if (it->second.first != kMemory) return;
        int64_t off = static_cast<int64_t>(current_stack_) -
                      static_cast<int64_t>(it->second.second);
        add_const(off);
      };
      for (auto &block : node->blocks_) {
        for (auto &ins : block->instructions_) {
          if (ins->removed_) continue;
          IRInstructionNode *i = ins.get();
          if (auto *p = dynamic_cast<IRArithmeticInstructionNode *>(i)) {
            scan(p->operand1_); scan(p->operand2_);
            scan_var(p->operand1_); scan_var(p->operand2_); scan_var(p->result_);
          } else if (auto *p = dynamic_cast<IRCompareInstructionNode *>(i)) {
            scan(p->operand1_); scan(p->operand2_);
            scan_var(p->operand1_); scan_var(p->operand2_); scan_var(p->result_);
          } else if (auto *p = dynamic_cast<IRStoreConstInstructionNode *>(i)) {
            add_const(p->value_);
            scan_var(p->pointer_);
          } else if (auto *p = dynamic_cast<IRStoreVariableInstructionNode *>(i)) {
            scan_var(p->pointer_); scan_var(p->value_);
          } else if (auto *p = dynamic_cast<IRLoadInstructionNode *>(i)) {
            scan_var(p->pointer_); scan_var(p->result_);
          } else if (auto *p = dynamic_cast<IRGetElementPtrInstructionNode *>(i)) {
            scan_var(p->ptrval_); scan_var(p->result_);
          } else if (auto *p = dynamic_cast<IRGetElementPtrPrimeInstructionNode *>(i)) {
            scan_var(p->ptrval_); scan(p->index_); scan_var(p->index_); scan_var(p->result_);
          } else if (auto *p = dynamic_cast<IRNegationInstructionNode *>(i)) {
            scan_var(p->operand_); scan_var(p->result_);
          } else if (auto *p = dynamic_cast<IRMoveInstructionNode *>(i)) {
            scan_var(p->source_); scan_var(p->result_);
          } else if (auto *p = dynamic_cast<IRCallInstructionNode *>(i)) {
            for (auto &arg : p->arguments_) {
              scan(arg->value_);
              scan_var(arg->value_);
            }
            if (!p->result_type_->IsEmpty()) scan_var(p->result_);
          } else if (auto *p = dynamic_cast<IRBranchInstructionNode *>(i)) {
            scan_var(p->condition_);
          } else if (auto *p = dynamic_cast<IRReturnInstructionNode *>(i)) {
            if (!p->type_->IsEmpty()) scan_var(p->name_);
          } else if (auto *p = dynamic_cast<IRSelectInstructionNode *>(i)) {
            scan_var(p->cond_); scan_var(p->result_);
          }
        }
      }
      // Also count stack offsets used by SaveRegister/FlushSavedRegisters.
      // These are used every time a call is made, so they can be very frequent.
      // Only add if the function actually has a stack frame and makes calls.
      // Weight by call count — each call site emits each save offset twice
      // (once in SaveRegister, once in RestoreRegister/FlushSavedRegisters),
      // so a function with many calls will load/store these offsets far more
      // often than any individual kMemory variable's slot.
      if (current_stack_ > 0 && node->a_reg_used_cnt_ > 0) {
        uint32_t save_weight = 0;
        for (auto &block : node->blocks_) {
          for (auto &ins : block->instructions_) {
            if (ins->removed_) continue;
            if (dynamic_cast<IRCallInstructionNode *>(ins.get())) ++save_weight;
            // Struct/array Load/Store/Move that lower to builtin_memcpy
            // also save/restore, but only large ones — skip for simplicity.
          }
        }
        save_weight = save_weight > 0 ? save_weight * 2 : 1;
        for (uint32_t i = 0; i < node->a_reg_used_cnt_; ++i) {
          int64_t off = static_cast<int32_t>(save_area_base_ + 8 * i);
          if (off < -2048 || off > 2047) const_freq[off] += save_weight;
        }
        int64_t ra_off = static_cast<int32_t>(save_area_base_ + 8 * node->a_reg_used_cnt_);
        if (ra_off < -2048 || ra_off > 2047) const_freq[ra_off] += save_weight;
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
        EmitLI(r, v);
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