#include "IR_visitor/printer/IR_printer.h"

#include "data_loader/data_loader.h"

IRPrinter::IRPrinter(const std::string &builtin_file, std::ostream &os) : builtin_(LoadFromFile(builtin_file)), os_(os) {}

void IRPrinter::Visit(IRStructNode *node) {
  os_ << "%" << node->name_ << " = type { ";
  uint32_t size = node->members_.size();
  for (uint32_t i = 0; i + 1 < size; ++i) {
    os_ << node->members_[i] << ", ";
  }
  os_ << *node->members_.rbegin() << " }\n";
}

void IRPrinter::Visit(IRArithmeticInstructionNode *node) {
  os_ << "  " << node->result_ << " = ";
  if (node->op_ == "+") {
    os_ << "add";
  } else if (node->op_ == "-") {
    os_ << "sub";
  } else if (node->op_ == "*") {
    os_ << "mul";
  } else if (node->op_ == "/") {
    if (node->is_unsigned_) {
      os_ << "udiv";
    } else {
      os_ << "sdiv";
    }
  } else if (node->op_ == "%") {
    if (node->is_unsigned_) {
      os_ << "urem";
    } else {
      os_ << "srem";
    }
  } else if (node->op_ == "<<") {
    os_ << "shl";
  } else if (node->op_ == ">>") {
    os_ << "ashr";
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
  os_ << " " << node->type_ << " " << node->operand1_ << ", " << node->operand2_ << '\n';
}

void IRPrinter::Visit(IRNegationInstructionNode *node) {
  os_ << "  " << node->result_ << " = ";
  if (node->is_minus_) {
    os_ << "sub " << node->type_ << " 0, " << node->operand_ << '\n';
  } else {
    if (node->type_ == "i1") {
      os_ << "xor " << node->type_ << " 1, " << node->operand_ << '\n';
    } else {
      os_ << "xor " << node->type_ << " -1, " << node->operand_ << '\n';
    }
  }
}

void IRPrinter::Visit(IRBranchInstructionNode *node) {
  os_ << "  br i1 " << node->condition_ << ", label %" << node->true_branch_ << ", label %" << node->false_branch_ << '\n';
}

void IRPrinter::Visit(IRJumpInstructionNode *node) {
  os_ << "  br label %" << node->destination_ << '\n';
}

void IRPrinter::Visit(IRReturnInstructionNode *node) {
  os_ << "  ret ";
  if (node->type_.empty()) {
    os_ << "void\n";
  } else {
    os_ << node->type_ << " " << node->name_ << '\n';
  }
}

void IRPrinter::Visit(IRAllocateInstructionNode *node) {
  os_ << "  " << node->result_ << " = alloca " << node->type_ << '\n';
}

void IRPrinter::Visit(IRLoadInstructionNode *node) {
  os_ << "  " << node->result_ << " = load " << node->type_ << ", ptr " << node->pointer_ << '\n';
}

void IRPrinter::Visit(IRStoreVariableInstructionNode *node) {
  os_ << "  store " << node->type_ << " " << node->value_ << ", ptr " << node->pointer_ << '\n';
}

void IRPrinter::Visit(IRStoreConstInstructionNode *node) {
  os_ << "  store " << node->type_ << " " << node->value_ << ", ptr " << node->pointer_ << '\n';
}

void IRPrinter::Visit(IRGetElementPtrInstructionNode *node) {
  os_ << "  " << node->result_ << " = getelementptr " << node->type_ << ", ptr " << node->ptrval_ << ", i32 0, i32 " << node->index_ << '\n';
}

void IRPrinter::Visit(IRGetElementPtrPrimeInstructionNode *node) {
  os_ << "  " << node->result_ << " = getelementptr " << node->type_ << ", ptr " << node->ptrval_ << ", i32 0, i32 " << node->index_ << '\n';
}

void IRPrinter::Visit(IRCompareInstructionNode *node) {
  os_ << "  " << node->result_ << " = icmp ";
  if (node->op_ == IRCompareInstructionNode::kEq) {
    os_ << "eq";
  } else if (node->op_ == IRCompareInstructionNode::kNe) {
    os_ << "ne";
  } else if (node->op_ == IRCompareInstructionNode::kUgt) {
    os_ << "ugt";
  } else if (node->op_ == IRCompareInstructionNode::kUge) {
    os_ << "uge";
  } else if (node->op_ == IRCompareInstructionNode::kUlt) {
    os_ << "ult";
  } else if (node->op_ == IRCompareInstructionNode::kUle) {
    os_ << "ule";
  } else if (node->op_ == IRCompareInstructionNode::kSgt) {
    os_ << "sgt";
  } else if (node->op_ == IRCompareInstructionNode::kSge) {
    os_ << "sge";
  } else if (node->op_ == IRCompareInstructionNode::kSlt) {
    os_ << "slt";
  } else {
    os_ << "sle";
  }
  os_ << " " << node->type_ << " " << node->operand1_ << ", " << node->operand2_ << '\n';
}

void IRPrinter::Visit(IRArgumentNode *node) {
  os_ << node->type_ << " " << node->value_;
}

void IRPrinter::Visit(IRCallInstructionNode *node) {
  if (node->result_type_.empty()) {
    os_ << "  call void";
  } else {
    os_ << "  " << node->result_ << " = call " << node->result_type_;
  }
  os_ << " @" << node->function_name_ << "(";
  uint32_t size = node->arguments_.size();
  for (uint32_t i = 0; i + 1 < size; ++i) {
    node->arguments_[i]->Accept(this);
    os_ << ", ";
  }
  if (!node->arguments_.empty()) {
    (*node->arguments_.rbegin())->Accept(this);
  }
  os_ << ")\n";
}

void IRPrinter::Visit(IRSelectInstructionNode *node) {
  os_ << "  " << node->result_ << " = select i1 " << node->cond_ << ", i32 1, i32 0\n";
}


void IRPrinter::Visit(IRBlockNode *node) {
  if (node->instructions_.empty()) {
    node->AddInstruction(std::make_shared<IRJumpInstructionNode>(node->id_)); // meaningless but dangerous, just to avoid empty block
  }
  os_ << node->id_ << ":\n";
  for (auto &instruction : node->instructions_) {
    instruction->Accept(this);
  }
}

void IRPrinter::Visit(IRParameterNode *node) {
  os_ << node->type_ << " " << node->name_;
}

void IRPrinter::Visit(IRFunctionNode *node) {
  os_ << "define " << node->type_ << " @" << node->name_ << "(";
  uint32_t size = node->parameters_.size();
  for (uint32_t i = 0; i + 1 < size; ++i) {
    node->parameters_[i]->Accept(this);
    os_ << ", ";
  }
  if (!node->parameters_.empty()) {
    (*node->parameters_.rbegin())->Accept(this);
  }
  os_ << ") {\n";
  for (auto &block : node->blocks_) {
    block->Accept(this);
  }
  os_ << "}\n";
}

void IRPrinter::Visit(IRRootNode *node) {
  // os_ << "declare void @builtin_memcpy(ptr, ptr, i32)\n";
  // os_ << "declare i32 @getInt()\n";
  // os_ << "declare void @printlnInt(i32)\n";
  // os_ << "declare void @printInt(i32)\n";
  os_ << builtin_ << '\n';
  for (auto &struct_node : node->structs_) {
    struct_node->Accept(this);
  }
  os_ << '\n';
  for (auto &function_node : node->functions_) {
    function_node->Accept(this);
    os_ << '\n';
  }
}
