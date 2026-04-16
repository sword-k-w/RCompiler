#include "IR_visitor/memory_allocator/memory_allocator.h"
#include "IR/struct_map.h"
#include <cassert>

uint32_t Align4(uint32_t x) {
  return (x + 3) / 4 * 4;
}

void MemoryAllocator::Visit(IRArrayNode *node) {}

void MemoryAllocator::Visit(IRStructNode *node) {}

void MemoryAllocator::Visit(IRArithmeticInstructionNode *node) {
  node->storage_type_ = kMemory;
  *current_stack_ += 4;
  node->address_ = *current_stack_;
}

void MemoryAllocator::Visit(IRNegationInstructionNode *node) {
  node->storage_type_ = kMemory;
  *current_stack_ += 4;
  node->address_ = *current_stack_;
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
  node->storage_type_ = kMemory;
  *current_stack_ += 4;
  node->address_ = *current_stack_;

  node->inner_storage_type_ = kMemory;
  *current_stack_ += Align4(node->type_->allocated_size_);
  node->inner_address_ = *current_stack_;
}

void MemoryAllocator::Visit(IRLoadInstructionNode *node) {
  node->storage_type_ = kMemory;
  *current_stack_ += Align4(node->type_->allocated_size_);
  node->address_ = *current_stack_;
}

void MemoryAllocator::Visit(IRStoreVariableInstructionNode *node) {}

void MemoryAllocator::Visit(IRStoreConstInstructionNode *node) {}

void MemoryAllocator::Visit(IRGetElementPtrInstructionNode *node) {
  node->storage_type_ = kMemory;
  *current_stack_ += 4;
  node->address_ = *current_stack_;
}

void MemoryAllocator::Visit(IRGetElementPtrPrimeInstructionNode *node) {
  node->storage_type_ = kMemory;
  *current_stack_ += 4;
  node->address_ = *current_stack_;
}

void MemoryAllocator::Visit(IRCompareInstructionNode *node) {
  node->storage_type_ = kMemory;
  *current_stack_ += 4;
  node->address_ = *current_stack_;
}

void MemoryAllocator::Visit(IRArgumentNode *node) {}

void MemoryAllocator::Visit(IRCallInstructionNode *node) {
  if (node->result_type_ != nullptr) {
    node->storage_type_ = kMemory;
    *current_stack_ += Align4(node->result_type_->allocated_size_);
    node->address_ += *current_stack_;
  }
}

void MemoryAllocator::Visit(IRSelectInstructionNode *node) {
  node->storage_type_ = kMemory;
  *current_stack_ += 4;
  node->address_ = *current_stack_;
}

void MemoryAllocator::Visit(IRBlockNode *node) {
  for (auto &instruction : node->instructions_) {
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
}

void MemoryAllocator::Visit(IRFunctionNode *node) {
  node->stack_size_ += 48 + 36; // save s0~s11, and reserve space to save a0~a7, ra
  current_stack_ = &node->stack_size_;
  current_parameter_register_ = 10;
  for (auto &parameter : node->parameters_) {
    parameter->Accept(this);
  }
  for (auto &block : node->parameters_) {
    block->Accept(this);
  }
}

void MemoryAllocator::Visit(IRRootNode *node) {
  for (auto &function_node : node->functions_) {
    function_node->Accept(this);
  }
}
