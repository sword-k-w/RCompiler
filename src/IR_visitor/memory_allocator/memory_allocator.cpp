#include "IR_visitor/memory_allocator/memory_allocator.h"
#include "IR/struct_map.h"
#include <cassert>

uint32_t Align8(uint32_t x) {
  return (x + 7) / 8 * 8;
}

void MemoryAllocator::RecordSize(const std::string &name, uint32_t size, IRInstructionNode *node) {
  auto it = variable_storage_->find(name);
  if (it == variable_storage_->end()) {
    (*variable_storage_)[name] = {kMemory, 0};
    (*variable_size_)[name] = size;
  }
  node->storage_type_ = kMemory;
  node->address_ = 0;
}

void MemoryAllocator::Visit(IRArrayNode *node) {}

void MemoryAllocator::Visit(IRStructNode *node) {}

void MemoryAllocator::Visit(IRArithmeticInstructionNode *node) {
  RecordSize(node->result_, 8, node);
}

void MemoryAllocator::Visit(IRNegationInstructionNode *node) {
  RecordSize(node->result_, 8, node);
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
  RecordSize(node->result_, 8, node);
  node->inner_storage_type_ = kMemory;
  node->inner_address_ = 0;
}

void MemoryAllocator::Visit(IRLoadInstructionNode *node) {
  RecordSize(node->result_, Align8(node->type_->allocated_size_), node);
}

void MemoryAllocator::Visit(IRStoreVariableInstructionNode *node) {}

void MemoryAllocator::Visit(IRStoreConstInstructionNode *node) {}

void MemoryAllocator::Visit(IRGetElementPtrInstructionNode *node) {
  RecordSize(node->result_, 8, node);
}

void MemoryAllocator::Visit(IRGetElementPtrPrimeInstructionNode *node) {
  RecordSize(node->result_, 8, node);
}

void MemoryAllocator::Visit(IRCompareInstructionNode *node) {
  RecordSize(node->result_, 8, node);
}

void MemoryAllocator::Visit(IRArgumentNode *node) {}

void MemoryAllocator::Visit(IRCallInstructionNode *node) {
  if (node->result_type_ != nullptr) {
    RecordSize(node->result_, Align8(node->result_type_->allocated_size_), node);
  }
}

void MemoryAllocator::Visit(IRMoveInstructionNode *node) {
  RecordSize(node->result_, Align8(node->type_->allocated_size_), node);
}

void MemoryAllocator::Visit(IRSelectInstructionNode *node) {
  RecordSize(node->result_, 8, node);
}

void MemoryAllocator::Visit(IRBlockNode *node) {
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
    node->address_ = 0;
    (*variable_size_)[node->name_] = Align8(node->type_->allocated_size_);
  }
  (*variable_storage_)[node->name_] = {node->storage_type_, node->address_};
}

void MemoryAllocator::Visit(IRFunctionNode *node) {
  // Determine if the function (or its generated assembly) contains any call
  // instructions: explicit IRCallInstructionNode, as well as struct/array
  // loads, stores, and moves that the assembly generator lowers to
  // builtin_memcpy calls.
  for (auto &block : node->blocks_) {
    for (auto &ins : block->instructions_) {
      if (dynamic_cast<IRCallInstructionNode *>(ins.get())) {
        node->has_calls_ = true;
        break;
      }
      {
        std::shared_ptr<IRArrayNode> type;
        if (auto *load = dynamic_cast<IRLoadInstructionNode *>(ins.get()))
          type = load->type_;
        else if (auto *store = dynamic_cast<IRStoreVariableInstructionNode *>(ins.get()))
          type = store->type_;
        else if (auto *move = dynamic_cast<IRMoveInstructionNode *>(ins.get()))
          type = move->type_;
        if (type && (!type->length_.empty() ||
                     (!type->base_type_.empty() && type->base_type_ != "i32" &&
                      type->base_type_ != "i1" && type->base_type_ != "ptr" &&
                      type->base_type_ != "void" &&
                      StructMap::Instance().Query(type->base_type_)))) {
          node->has_calls_ = true;
          break;
        }
      }
    }
    if (node->has_calls_) break;
  }
  // Check whether all parameters fit in a-regs.  If any parameter must go
  // on the stack, treat the function as if it has calls to keep parameter
  // addresses (used by both caller and callee) unchanged.
  if (!node->has_calls_) {
    uint32_t preg = 10;
    for (auto &parameter : node->parameters_) {
      if (parameter->type_->length_.empty() && preg < 18
          && (parameter->type_->base_type_ == "i32"
              || parameter->type_->base_type_ == "ptr"
              || parameter->type_->base_type_ == "i1")) {
        ++preg;
      } else {
        node->has_calls_ = true;
        break;
      }
    }
  }
  // No longer grow stack_size_ here.  Addresses are assigned after reg_alloc.
  current_parameter_register_ = 10;
  variable_storage_ = &node->variable_storage_;
  variable_size_ = &node->variable_size_;
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
