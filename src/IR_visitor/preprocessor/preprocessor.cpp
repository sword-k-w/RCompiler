#include "IR_visitor/preprocessor/preprocessor.h"
#include "IR/struct_map.h"
#include "IR_visitor/memory_allocator/memory_allocator.h"
#include <iostream>

void Preprocessor::Visit(IRArrayNode *node) {
  if (node->IsEmpty()) {
    return;
  }
  if (node->base_type_ == "i32" || node->base_type_ == "ptr") {
    node->align_ = 4;
    node->allocated_size_ = 4;
    for (auto &length : node->length_) {
      node->allocated_size_ *= length;
    }
  } else if (node->base_type_ == "i1") {
    node->align_ = 1;
    node->allocated_size_ = 1;
    for (auto &length: node->length_) {
      node->allocated_size_ *= length;
    }
  } else {
    auto struct_node = StructMap::Instance().Query(node->base_type_);
    node->align_ = struct_node->align_;
    node->allocated_size_ = struct_node->allocated_size_;
    for (auto &length: node->length_) {
      node->allocated_size_ *= length;
    }
  }
}

void Preprocessor::Visit(IRStructNode *node) {
  node->align_ = 1;
  node->allocated_size_ = 0;
  for (auto &array_node : node->members_) {
    array_node->Accept(this);
    auto inner_size = array_node->allocated_size_;
    auto inner_align = array_node->align_;
    if (inner_align == 1 && node->align_ == 4) {
      inner_size = Align4(inner_size);
    }
    if (node->align_ == 1 && inner_align == 4) {
      node->allocated_size_ = Align4(node->allocated_size_);
      node->align_ = 4;
    }
    node->allocated_size_ += inner_size;
  }
}

void Preprocessor::Visit(IRArithmeticInstructionNode *node) {
  (*current_variables_)[node->result_] = node;
}

void Preprocessor::Visit(IRNegationInstructionNode *node) {
  (*current_variables_)[node->result_] = node;
}

void Preprocessor::Visit(IRBranchInstructionNode *node) {}

void Preprocessor::Visit(IRJumpInstructionNode *node) {}

void Preprocessor::Visit(IRReturnInstructionNode *node) {
  if (!node->type_->IsEmpty()) {
    node->type_->Accept(this);
  }
}

void Preprocessor::Visit(IRAllocateInstructionNode *node) {
  node->type_->Accept(this);
  (*current_variables_)[node->result_] = node;
}

void Preprocessor::Visit(IRLoadInstructionNode *node) {
  node->type_->Accept(this);
  (*current_variables_)[node->result_] = node;
}

void Preprocessor::Visit(IRStoreVariableInstructionNode *node) {
  node->type_->Accept(this);
}

void Preprocessor::Visit(IRStoreConstInstructionNode *node) {}

void Preprocessor::Visit(IRGetElementPtrInstructionNode *node) {
  node->type_->Accept(this);
  (*current_variables_)[node->result_] = node;
}

void Preprocessor::Visit(IRGetElementPtrPrimeInstructionNode *node) {
  node->type_->Accept(this);
  (*current_variables_)[node->result_] = node;
}

void Preprocessor::Visit(IRCompareInstructionNode *node) {
  (*current_variables_)[node->result_] = node;
}

void Preprocessor::Visit(IRArgumentNode *node) {
  node->type_->Accept(this);
}

void Preprocessor::Visit(IRCallInstructionNode *node) {
  if (node->result_type_ != nullptr) {
    node->result_type_->Accept(this);
    (*current_variables_)[node->result_] = node;
  }
  for (auto &argument : node->arguments_) {
    argument->Accept(this);
  }
}

void Preprocessor::Visit(IRSelectInstructionNode *node) {
  (*current_variables_)[node->result_] = node;
}

void Preprocessor::Visit(IRBlockNode *node) {
  for (auto &instruction : node->instructions_) {
    instruction->Accept(this);
  }
}

void Preprocessor::Visit(IRParameterNode *node) {
  node->type_->Accept(this);
  (*current_variables_)[node->name_] = node;
}

void Preprocessor::Visit(IRFunctionNode *node) {
  if (!node->type_->IsEmpty()) {
    node->type_->Accept(this);
  }
  current_variables_ = &node->variables_;
  for (auto parameter : node->parameters_) {
    parameter->Accept(this);
  }
  for (auto block : node->blocks_) {
    block->Accept(this);
  }
}

void Preprocessor::Visit(IRRootNode *node) {
  for (auto &struct_node : node->structs_) {
    struct_node->Accept(this);
  }
  for (auto &function_node : node->functions_) {
    function_node->Accept(this);
  }
}
