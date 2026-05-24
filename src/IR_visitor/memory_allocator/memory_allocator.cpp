#include "IR_visitor/memory_allocator/memory_allocator.h"
#include "IR/struct_map.h"
#include <cassert>

uint32_t Align4(uint32_t x) {
  return (x + 3) / 4 * 4;
}

void MemoryAllocator::AllocateOrReuse(const std::string &name, uint32_t size, IRInstructionNode *node) {
  auto it = variable_storage_->find(name);
  if (it == variable_storage_->end()) {
    *current_stack_ += size;
    (*variable_storage_)[name] = {kMemory, *current_stack_};
  }
  node->storage_type_ = kMemory;
  node->address_ = (*variable_storage_)[name].second;
}

void MemoryAllocator::Visit(IRArrayNode *node) {}

void MemoryAllocator::Visit(IRStructNode *node) {}

void MemoryAllocator::Visit(IRArithmeticInstructionNode *node) {
  AllocateOrReuse(node->result_, 4, node);
}

void MemoryAllocator::Visit(IRNegationInstructionNode *node) {
  AllocateOrReuse(node->result_, 4, node);
}

void MemoryAllocator::Visit(IRBranchInstructionNode *node) {}

void MemoryAllocator::Visit(IRJumpInstructionNode *node) {}

void MemoryAllocator::Visit(IRReturnInstructionNode *node) {
  if (node->type_ != nullptr) {
    node->storage_type_ = kRegister;
    node->address_ = 10;
  }
}

void MemoryAllocator::Visit(IRAllocateInstructionNode *node) {
  AllocateOrReuse(node->result_, 4, node);

  node->inner_storage_type_ = kMemory;
  *current_stack_ += Align4(node->type_->allocated_size_);
  node->inner_address_ = *current_stack_;
}

void MemoryAllocator::Visit(IRLoadInstructionNode *node) {
  AllocateOrReuse(node->result_, Align4(node->type_->allocated_size_), node);
}

void MemoryAllocator::Visit(IRStoreVariableInstructionNode *node) {}

void MemoryAllocator::Visit(IRStoreConstInstructionNode *node) {}

void MemoryAllocator::Visit(IRGetElementPtrInstructionNode *node) {
  AllocateOrReuse(node->result_, 4, node);
}

void MemoryAllocator::Visit(IRGetElementPtrPrimeInstructionNode *node) {
  AllocateOrReuse(node->result_, 4, node);
}

void MemoryAllocator::Visit(IRCompareInstructionNode *node) {
  AllocateOrReuse(node->result_, 4, node);
}

void MemoryAllocator::Visit(IRArgumentNode *node) {}

void MemoryAllocator::Visit(IRCallInstructionNode *node) {
  if (node->result_type_ != nullptr) {
    AllocateOrReuse(node->result_, Align4(node->result_type_->allocated_size_), node);
  }
}

void MemoryAllocator::Visit(IRPhiInstructionNode *node) {
  AllocateOrReuse(node->result_, Align4(node->type_->allocated_size_), node);
}

void MemoryAllocator::Visit(IRSelectInstructionNode *node) {
  AllocateOrReuse(node->result_, 4, node);
}

void MemoryAllocator::Visit(IRBlockNode *node) {
  for (auto &phi : node->phi_) {
    phi->Accept(this);
  }
  for (auto &instruction : node->instructions_) {
    if (instruction->removed_) {
      continue;
    }
    instruction->Accept(this);
  }
}

void MemoryAllocator::Visit(IRParameterNode *node) {
  if (node->type_->length_.empty() && current_parameter_register_ < 18
     && (node->type_->base_type_ == "i32" || node->type_->base_type_ == "ptr" || node->type_->base_type_ == "i1")) {
    node->storage_type_ = kRegister;
    node->address_ = current_parameter_register_++;
  } else {
    node->storage_type_ = kMemory;
    *current_stack_ += Align4(node->type_->allocated_size_);
    node->address_ = *current_stack_;
  }
  (*variable_storage_)[node->name_] = {node->storage_type_, node->address_};
}

void MemoryAllocator::Visit(IRFunctionNode *node) {
  node->stack_size_ += 28 + 36;
  current_stack_ = &node->stack_size_;
  current_parameter_register_ = 10;
  variable_storage_ = &node->variable_storage_;
  for (auto &parameter : node->parameters_) {
    parameter->Accept(this);
  }
  for (auto &block : node->blocks_) {
    block->Accept(this);
  }
  node->a_reg_used_cnt_ = current_parameter_register_ - 10;
}

void MemoryAllocator::Visit(IRRootNode *node) {
  for (auto &function_node : node->functions_) {
    function_node->Accept(this);
  }
}
