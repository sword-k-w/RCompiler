#include "visitor/IR_generator/IR_generator.h"

#include "parser/node/enumeration.h"
#include "parser/node/expression.h"
#include "parser/node/function.h"
#include "parser/node/implementation.h"
#include "parser/node/struct.h"
#include "parser/node/terminal.h"
#include "parser/node/trait.h"
#include "parser/node/type.h"
#include "parser/node/pattern.h"
#include "parser/node/crate.h"
#include "parser/node/item.h"
#include "parser/node/path.h"
#include "parser/node/statement.h"
#include "semantic/builtin/builtin_node.h"

IRGenerator::IRGenerator(std::shared_ptr<IRRootNode> root) : root_(root) {}

void IRGenerator::Visit(CrateNode *node) {
  for (auto &item : node->items_) {
    item->Accept(this);
  }
}

void IRGenerator::Visit(BlockExpressionNode *node) {
  if (node->statements_ != nullptr) {
    auto statements = node->statements_;
    for (auto &statement : statements->statement_s_) {
      statement->Accept(this);
    }
    if (statements->expr_without_block_ != nullptr) {
      statements->expr_without_block_->expr_->Accept(this);
      node->IR_name_ = statements->expr_without_block_->expr_->IR_name_;
    } else {
      auto trailer_statement = *statements->statement_s_.rbegin();
      if (trailer_statement->expr_statement_ != nullptr
        && trailer_statement->expr_statement_->semicolon_ == false) {
        node->IR_name_ = trailer_statement->expr_statement_->expr_with_block_->IR_name_;
      }
    }
  }
}

void IRGenerator::Visit(InfiniteLoopExpressionNode *node) {
  auto loop_body = std::make_shared<IRBlockNode>(cur_tag_cnt_);
  cur_function_->AddBlock(loop_body);
  ++cur_tag_cnt_;
  auto loop_end = std::make_shared<IRBlockNode>(cur_tag_cnt_);
  cur_function_->AddBlock(loop_end);
  ++cur_tag_cnt_;
  loop_condition_block_.emplace(nullptr);
  loop_body_block_.emplace(loop_body);
  loop_end_block_.emplace(loop_end);
  cur_block_->AddInstruction(std::make_shared<IRJumpInstructionNode>(loop_body->GetID()));
  cur_block_ = loop_body;
  node->block_expr_->Accept(this);
  cur_block_->AddInstruction(std::make_shared<IRJumpInstructionNode>(loop_body->GetID()));
  cur_block_ = loop_end;
  loop_condition_block_.pop();
  loop_body_block_.pop();
  loop_end_block_.pop();
}

void IRGenerator::Visit(ConditionsNode *node) {
  node->expr_->Accept(this);
  node->IR_name_ = node->expr_->IR_name_;
}

void IRGenerator::Visit(PredicateLoopExpressionNode *node) {
  auto loop_condition = std::make_shared<IRBlockNode>(cur_tag_cnt_);
  cur_function_->AddBlock(loop_condition);
  ++cur_tag_cnt_;
  auto loop_body = std::make_shared<IRBlockNode>(cur_tag_cnt_);
  cur_function_->AddBlock(loop_body);
  ++cur_tag_cnt_;
  auto loop_end = std::make_shared<IRBlockNode>(cur_tag_cnt_);
  cur_function_->AddBlock(loop_end);
  ++cur_tag_cnt_;
  loop_condition_block_.emplace(loop_condition);
  loop_body_block_.emplace(loop_body);
  loop_end_block_.emplace(loop_end);
  cur_block_->AddInstruction(std::make_shared<IRJumpInstructionNode>(loop_condition->GetID()));
  cur_block_ = loop_condition;
  node->conditions_->Accept(this);
  std::string cond = name_allocator_.Allocate("%tmp.");
  cur_block_->AddInstruction(std::make_shared<IRLoadInstructionNode>(cond, "i1", node->conditions_->IR_name_));
  cur_block_->AddInstruction(std::make_shared<IRBranchInstructionNode>(cond, loop_body->GetID(), loop_end->GetID()));
  cur_block_ = loop_body;
  node->block_expr_->Accept(this);
  cur_block_->AddInstruction(std::make_shared<IRJumpInstructionNode>(loop_condition->GetID()));
  cur_block_ = loop_end;
  loop_condition_block_.pop();
  loop_body_block_.pop();
  loop_end_block_.pop();
}

void IRGenerator::Visit(LoopExpressionNode *node) {
  if (node->infinite_loop_expr_ != nullptr) {
    node->infinite_loop_expr_->Accept(this);
    node->IR_name_ = node->infinite_loop_expr_->IR_name_;
  } else {
    node->predicate_loop_expr_->Accept(this);
    node->IR_name_ = node->predicate_loop_expr_->IR_name_;
  }
}

void IRGenerator::Visit(IfExpressionNode *node) {
  if (node->type_info_->type_ != kUnitType && node->type_info_->type_ != kNeverType) {
    node->IR_name_ = name_allocator_.Allocate("%tmp.");
    cur_block_->AddInstruction(std::make_shared<IRAllocateInstructionNode>(node->IR_name_, GetIRTypeString(node->type_info_.get())));
  }
  node->conditions_->Accept(this);
  std::string cond = name_allocator_.Allocate("%tmp.");
  cur_block_->AddInstruction(std::make_shared<IRLoadInstructionNode>(cond, "i1", node->conditions_->IR_name_));
  auto true_block = std::make_shared<IRBlockNode>(cur_tag_cnt_);
  cur_function_->AddBlock(true_block);
  ++cur_tag_cnt_;
  auto end_block = std::make_shared<IRBlockNode>(cur_tag_cnt_);
  cur_function_->AddBlock(end_block);
  ++cur_tag_cnt_;
  if (node->block_expr2_ == nullptr && node->if_expr_ == nullptr) {
    cur_block_->AddInstruction(std::make_shared<IRBranchInstructionNode>(cond, true_block->GetID(), end_block->GetID()));
    cur_block_ = true_block;
    node->block_expr1_->Accept(this);
    cur_block_->AddInstruction(std::make_shared<IRJumpInstructionNode>(end_block->GetID()));
    cur_block_ = end_block;
  } else {
    auto false_block = std::make_shared<IRBlockNode>(cur_tag_cnt_);
    cur_function_->AddBlock(false_block);
    ++cur_tag_cnt_;
    cur_block_->AddInstruction(std::make_shared<IRBranchInstructionNode>(cond, true_block->GetID(), false_block->GetID()));
    cur_block_ = true_block;
    node->block_expr1_->Accept(this);
    if (!node->IR_name_.empty()) {
      Copy(node->IR_name_, node->block_expr1_->IR_name_, node->type_info_.get());
    }
    cur_block_->AddInstruction(std::make_shared<IRJumpInstructionNode>(end_block->GetID()));
    cur_block_ = false_block;
    if (node->block_expr2_ != nullptr) {
      node->block_expr2_->Accept(this);
      if (!node->IR_name_.empty()) {
        Copy(node->IR_name_, node->block_expr2_->IR_name_, node->type_info_.get());
      }
    } else {
      node->if_expr_->Accept(this);
      if (!node->IR_name_.empty()) {
        Copy(node->IR_name_, node->if_expr_->IR_name_, node->type_info_.get());
      }
    }
    cur_block_->AddInstruction(std::make_shared<IRJumpInstructionNode>(end_block->GetID()));
    cur_block_ = end_block;
  }
}

void IRGenerator::Visit(ExpressionWithBlockNode *node) {
  if (node->block_expr_ != nullptr) {
    node->block_expr_->Accept(this);
    node->IR_name_ = node->block_expr_->IR_name_;
  } else if (node->if_expr_ != nullptr) {
    node->if_expr_->Accept(this);
    node->IR_name_ = node->if_expr_->IR_name_;
  } else {
    node->loop_expr_->Accept(this);
    node->IR_name_ = node->loop_expr_->IR_name_;
  }
}

void IRGenerator::Visit(ExpressionNode *node) {
  if (node->const_value_ != nullptr) {
    node->IR_name_ = name_allocator_.Allocate("%tmp.");
    if (node->type_info_->type_ != kLeafType) {
      std::cerr << "Error! : strange const value!\n";
      exit(-1);
    }
    std::string IR_type = "i1";
    if (node->type_info_->type_name_ != "bool") {
      IR_type = "i32";
    }
    cur_block_->AddInstruction(std::make_shared<IRAllocateInstructionNode>(node->IR_name_, IR_type));
    cur_block_->AddInstruction(std::make_shared<IRStoreConstInstructionNode>(
      IR_type, node->const_value_->u32_value_, node->IR_name_));
    return;
  }
  if (node->type_ == kPathExpr) {
    if (node->path_expr_->path_expr_segment2_ != nullptr) {
      if (node->type_info_->type_ != kEnumType) {
        std::cerr << "Error! : there should be enum!";
        exit(-1);
      }
      node->IR_name_ = name_allocator_.Allocate("%tmp.");
      std::string IR_type = "i32";
      cur_block_->AddInstruction(std::make_shared<IRAllocateInstructionNode>(node->IR_name_, IR_type));
      cur_block_->AddInstruction(std::make_shared<IRStoreConstInstructionNode>(
        IR_type,
        dynamic_cast<EnumerationNode *>(node->type_info_->source_)
          ->enum_[node->path_expr_->path_expr_segment2_->identifier_->val_],
        node->IR_name_));
    } else {
      if (node->path_expr_->path_expr_segment1_->identifier_ != nullptr) {
        node->IR_name_ = node->identifier_target_->IR_name_;
      } else {
        node->IR_name_ = dynamic_cast<SelfParamNode *>(node->scope_->FindValueName("self"))->IR_name_;
      }
    }
  } else if (node->type_ == kArrayExpr) {
    node->IR_name_ = name_allocator_.Allocate("%tmp.");
    std::string IR_type = GetIRTypeString(node->type_info_.get());
    cur_block_->AddInstruction(std::make_shared<IRAllocateInstructionNode>(node->IR_name_, IR_type));
    auto &[inside_type, size] = node->type_info_->array_type_info_;
    std::string IR_inside_type = GetIRTypeString(inside_type.get());
    if (node->array_expr_->array_elements_->semicolon_) {
      auto son_expr = node->array_expr_->array_elements_->exprs_[0];
      son_expr->Accept(this);
      for (uint32_t i = 0; i < size; ++i) {
        std::string tmp = name_allocator_.Allocate("%tmp.");
        cur_block_->AddInstruction(std::make_shared<IRGetElementPtrInstructionNode>(
          tmp, IR_type, node->IR_name_, i));
        Copy(tmp, son_expr->IR_name_, inside_type.get());
      }
    } else {
      for (uint32_t i = 0; i < size; ++i) {
        auto son_expr = node->array_expr_->array_elements_->exprs_[i];
        son_expr->Accept(this);
        std::string tmp = name_allocator_.Allocate("%tmp.");
        cur_block_->AddInstruction(std::make_shared<IRGetElementPtrInstructionNode>(
          tmp, IR_type, node->IR_name_, i));
        Copy(tmp, son_expr->IR_name_, inside_type.get());
      }
    }
  } else if (node->type_ == kStructExpr) {
    node->IR_name_ = name_allocator_.Allocate("%tmp.");
    std::string IR_type = GetIRTypeString(node->type_info_.get());
    cur_block_->AddInstruction(std::make_shared<IRAllocateInstructionNode>(node->IR_name_, IR_type));
    auto struct_node = dynamic_cast<StructNode *>(node->type_info_->source_);
    uint32_t size = struct_node->field_.size();
    for (uint32_t i = 0; i < size; ++i) {
      auto son_expr = node->struct_expr_->struct_expr_fields_->struct_expr_field_s_[i]->expr_;
      auto inside_type = struct_node->struct_fields_->struct_field_s_[i]->type_->type_info_.get();
      son_expr->Accept(this);
      std::string tmp = name_allocator_.Allocate("%tmp.");
      cur_block_->AddInstruction(std::make_shared<IRGetElementPtrInstructionNode>(
        tmp, IR_type, node->IR_name_, i));
      Copy(tmp, son_expr->IR_name_, inside_type);
    }
  } else if (node->type_ == kBorrowExpr) {
    node->IR_name_ = name_allocator_.Allocate("%tmp.");
    node->expr1_->Accept(this);
    Borrow(node->IR_name_, node->expr1_->IR_name_, GetIRTypeString(node->type_info_.get()));
  } else if (node->type_ == kDereferenceExpr) {
    node->IR_name_ = name_allocator_.Allocate("%tmp.");
    node->expr1_->Accept(this);
    Dereference(node->IR_name_, node->expr1_->IR_name_);
  } else if (node->type_ == kNegationExpr) {
    node->IR_name_ = name_allocator_.Allocate("%tmp.");
    node->expr1_->Accept(this);
    std::string IR_type = GetIRTypeString(node->type_info_.get());
    cur_block_->AddInstruction(std::make_shared<IRAllocateInstructionNode>(node->IR_name_, IR_type));
    std::string tmp = name_allocator_.Allocate("%tmp.");
    std::string operand = name_allocator_.Allocate("%tmp.");
    cur_block_->AddInstruction(std::make_shared<IRLoadInstructionNode>(operand, IR_type, node->expr1_->IR_name_));
    cur_block_->AddInstruction(std::make_shared<IRNegationInstructionNode>(tmp, node->op_ == "-", IR_type, operand));
    cur_block_->AddInstruction(std::make_shared<IRStoreVariableInstructionNode>(IR_type, tmp, node->IR_name_));
  } else if (node->type_ == kArithmeticOrLogicExpr) {
    node->IR_name_ = name_allocator_.Allocate("%tmp.");
    node->expr1_->Accept(this);
    node->expr2_->Accept(this);
    std::string IR_type = "i32";
    if (node->type_info_->type_name_ == "bool") {
      IR_type = "i1";
    }
    cur_block_->AddInstruction(std::make_shared<IRAllocateInstructionNode>(node->IR_name_, IR_type));
    std::string tmp = name_allocator_.Allocate("%tmp.");
    std::string operand1 = name_allocator_.Allocate("%tmp.");
    std::string operand2 = name_allocator_.Allocate("%tmp.");
    cur_block_->AddInstruction(std::make_shared<IRLoadInstructionNode>(operand1, IR_type, node->expr1_->IR_name_));
    cur_block_->AddInstruction(std::make_shared<IRLoadInstructionNode>(operand2, IR_type, node->expr2_->IR_name_));
    cur_block_->AddInstruction(std::make_shared<IRArithmeticInstructionNode>(tmp, node->op_, IR_type, operand1, operand2,
      node->type_info_->type_name_ == "u32" || node->type_info_->type_name_ == "usize"));
    cur_block_->AddInstruction(std::make_shared<IRStoreVariableInstructionNode>(IR_type, tmp, node->IR_name_));
  } else if (node->type_ == kComparisonExpr) {
    node->IR_name_ = name_allocator_.Allocate("%tmp.");
    node->expr1_->Accept(this);
    node->expr2_->Accept(this);
    std::string IR_type = "i1";
    std::string son_type = "i32";
    if (node->expr1_->type_info_->type_name_ == "bool") {
      son_type = "i1";
    }
    cur_block_->AddInstruction(std::make_shared<IRAllocateInstructionNode>(node->IR_name_, IR_type));
    std::string tmp = name_allocator_.Allocate("%tmp.");
    std::string operand1 = name_allocator_.Allocate("%tmp.");
    std::string operand2 = name_allocator_.Allocate("%tmp.");
    cur_block_->AddInstruction(std::make_shared<IRLoadInstructionNode>(operand1, son_type, node->expr1_->IR_name_));
    cur_block_->AddInstruction(std::make_shared<IRLoadInstructionNode>(operand2, son_type, node->expr2_->IR_name_));
    if (node->op_ == "==") {
      cur_block_->AddInstruction(std::make_shared<IRCompareInstructionNode>(
        tmp, IRCompareInstructionNode::Operator::kEq, son_type, operand1, operand2));
    } else if (node->op_ == "!=") {
      cur_block_->AddInstruction(std::make_shared<IRCompareInstructionNode>(
        tmp, IRCompareInstructionNode::Operator::kNe, son_type, operand1, operand2));
    } else {
      bool is_unsigned = (node->expr1_->type_info_->type_name_ == "u32" || node->expr1_->type_info_->type_name_ == "usize");
      if (node->op_ == ">") {
        cur_block_->AddInstruction(std::make_shared<IRCompareInstructionNode>(
        tmp, is_unsigned ? IRCompareInstructionNode::Operator::kUgt : IRCompareInstructionNode::Operator::kSgt,
        son_type, operand1, operand2));
      } else if (node->op_ == "<") {
        cur_block_->AddInstruction(std::make_shared<IRCompareInstructionNode>(
        tmp, is_unsigned ? IRCompareInstructionNode::Operator::kUlt : IRCompareInstructionNode::Operator::kSlt,
        son_type, operand1, operand2));
      } else if (node->op_ == ">=") {
        cur_block_->AddInstruction(std::make_shared<IRCompareInstructionNode>(
        tmp, is_unsigned ? IRCompareInstructionNode::Operator::kUge : IRCompareInstructionNode::Operator::kSge,
        son_type, operand1, operand2));
      } else {
        cur_block_->AddInstruction(std::make_shared<IRCompareInstructionNode>(
        tmp, is_unsigned ? IRCompareInstructionNode::Operator::kUle : IRCompareInstructionNode::Operator::kSle,
        son_type, operand1, operand2));
      }
    }
    cur_block_->AddInstruction(std::make_shared<IRStoreVariableInstructionNode>(IR_type, tmp, node->IR_name_));
  } else if (node->type_ == kLazyBooleanExpr) {
    node->IR_name_ = name_allocator_.Allocate("%tmp.");
    cur_block_->AddInstruction(std::make_shared<IRAllocateInstructionNode>(node->IR_name_, "i1"));
    node->expr1_->Accept(this);
    auto extra_block = std::make_shared<IRBlockNode>(cur_tag_cnt_);
    cur_function_->AddBlock(extra_block);
    ++cur_tag_cnt_;
    auto end_block = std::make_shared<IRBlockNode>(cur_tag_cnt_);
    cur_function_->AddBlock(end_block);
    ++cur_tag_cnt_;
    std::string cond = name_allocator_.Allocate("%tmp.");
    cur_block_->AddInstruction(std::make_shared<IRLoadInstructionNode>(cond, "i1", node->expr1_->IR_name_));
    Copy(node->IR_name_, node->expr1_->IR_name_, node->type_info_.get());
    if (node->op_ == "&&") {
      cur_block_->AddInstruction(std::make_shared<IRBranchInstructionNode>(cond, extra_block->GetID(), end_block->GetID()));
    } else {
      cur_block_->AddInstruction(std::make_shared<IRBranchInstructionNode>(cond, end_block->GetID(), extra_block->GetID()));
    }
    cur_block_ = extra_block;
    node->expr2_->Accept(this);
    Copy(node->IR_name_, node->expr2_->IR_name_, node->type_info_.get());
    cur_block_->AddInstruction(std::make_shared<IRJumpInstructionNode>(end_block->GetID()));
    cur_block_ = end_block;
  } else if (node->type_ == kTypeCastExpr) {
    node->expr1_->Accept(this);
    if (node->expr1_->type_info_->type_name_ == "bool") {
      node->IR_name_ = name_allocator_.Allocate("%tmp.");
      std::string tmp1 = name_allocator_.Allocate("%tmp.");
      std::string tmp2 = name_allocator_.Allocate("%tmp.");
      cur_block_->AddInstruction(std::make_shared<IRLoadInstructionNode>(tmp1, "i1", node->expr1_->IR_name_));
      cur_block_->AddInstruction(std::make_shared<IRSelectInstructionNode>(tmp2, tmp1));
      Borrow(node->IR_name_, tmp2, "i32");
    } else {
      node->IR_name_ = node->expr1_->IR_name_;
    }
  } else if (node->type_ == kGroupedExpr) {
    node->expr1_->Accept(this);
    node->IR_name_ = node->expr1_->IR_name_;
  } else if (node->type_ == kAssignmentExpr) {
    node->expr1_->Accept(this);
    node->expr2_->Accept(this);
    node->IR_name_ = node->expr1_->IR_name_;
    Copy(node->expr1_->IR_name_, node->expr2_->IR_name_, node->expr1_->type_info_.get());
  } else if (node->type_ == kCompoundAssignmentExpr) {
    node->expr1_->Accept(this);
    node->expr2_->Accept(this);
    std::string IR_type = "i32";
    if (node->expr1_->type_info_->type_name_ == "bool") {
      IR_type = "i1";
    }
    std::string tmp = name_allocator_.Allocate("%tmp.");
    std::string operand1 = name_allocator_.Allocate("%tmp.");
    std::string operand2 = name_allocator_.Allocate("%tmp.");
    cur_block_->AddInstruction(std::make_shared<IRLoadInstructionNode>(operand1, IR_type, node->expr1_->IR_name_));
    cur_block_->AddInstruction(std::make_shared<IRLoadInstructionNode>(operand2, IR_type, node->expr2_->IR_name_));
    std::string reduced_op = node->op_;
    reduced_op.pop_back();
    cur_block_->AddInstruction(std::make_shared<IRArithmeticInstructionNode>(tmp, reduced_op, IR_type, operand1, operand2,
      node->expr2_->type_info_->type_name_ == "u32" || node->expr2_->type_info_->type_name_ == "usize"));
    cur_block_->AddInstruction(std::make_shared<IRStoreVariableInstructionNode>(IR_type, tmp, node->expr1_->IR_name_));
  } else if (node->type_ == kIndexExpr) {
    node->IR_name_ = name_allocator_.Allocate("%tmp.");
    node->expr1_->Accept(this);
    node->expr2_->Accept(this);
    std::string array_pointer = node->expr1_->IR_name_; // the IR name of the pointer that point to the array
    Type *array_type = node->expr1_->type_info_.get();
    if (node->expr1_->type_info_->type_ == kPointerType) { // auto dereference
      array_pointer = name_allocator_.Allocate("%tmp.");
      array_type = array_type->pointer_type_.get();
      Dereference(array_pointer, node->expr1_->IR_name_);
    }
    std::string tmp = name_allocator_.Allocate("%tmp."); // index
    cur_block_->AddInstruction(std::make_shared<IRLoadInstructionNode>(tmp, "i32", node->expr2_->IR_name_));
    cur_block_->AddInstruction(std::make_shared<IRGetElementPtrPrimeInstructionNode>(
      node->IR_name_, GetIRTypeString(array_type), array_pointer, tmp));
  } else if (node->type_ == kFieldExpr) {
    node->IR_name_ = name_allocator_.Allocate("%tmp.");
    node->expr1_->Accept(this);
    std::string struct_pointer = node->expr1_->IR_name_;
    Type *struct_type = node->expr1_->type_info_.get();
    if (node->expr1_->type_info_->type_ == kPointerType) {
      struct_pointer = name_allocator_.Allocate("%tmp.");
      struct_type = struct_type->pointer_type_.get();
      Dereference(struct_pointer, node->expr1_->IR_name_);
    }
    uint32_t index = -1;
    auto struct_fields = dynamic_cast<StructNode *>(struct_type->source_)->struct_fields_->struct_field_s_;
    for (uint32_t i = 0; i < struct_fields.size(); ++i) {
      if (struct_fields[i]->identifier_->val_ == node->identifier_->val_) {
        index = i;
        break;
      }
    }
    cur_block_->AddInstruction(std::make_shared<IRGetElementPtrInstructionNode>(
      node->IR_name_, GetIRTypeString(struct_type), struct_pointer, index));
  } else if (node->type_ == kCallExpr) {
    std::string function_name;
    auto builtin_function = dynamic_cast<BuiltinFunctionNode *>(node->expr1_->type_info_->source_);
    if (builtin_function != nullptr) {
      function_name = builtin_function->function_name_;
      if (function_name == "exit") {
        cur_block_->AddInstruction(std::make_shared<IRReturnInstructionNode>("i32", "0"));
        return;
      }
    } else {
      auto function_node = dynamic_cast<FunctionNode *>(node->expr1_->type_info_->source_);
      if (function_node->IR_name_.empty()) {
        function_node->IR_name_ = name_allocator_.Allocate("function.." + function_node->identifier_->val_);
      }
      function_name = function_node->IR_name_;
    }
    std::shared_ptr<IRCallInstructionNode> IR_call;
    std::string result;
    if (node->type_info_->type_ != kUnitType) {
      result = name_allocator_.Allocate("%tmp.");
      IR_call = std::make_shared<IRCallInstructionNode>(result, GetIRTypeString(node->type_info_.get()), function_name);
    } else {
      IR_call = std::make_shared<IRCallInstructionNode>("", "", function_name);
    }
    if (node->call_params_ != nullptr) {
      for (auto &expr : node->call_params_->exprs_) {
        expr->Accept(this);
        std::string tmp = name_allocator_.Allocate("%tmp.");
        std::string IR_type = GetIRTypeString(expr->type_info_.get());
        cur_block_->AddInstruction(std::make_shared<IRLoadInstructionNode>(tmp, IR_type, expr->IR_name_));
        IR_call->AddArgument(std::make_shared<IRArgumentNode>(IR_type, tmp));
      }
    }
    cur_block_->AddInstruction(IR_call);
    if (!result.empty()) {
      node->IR_name_ = name_allocator_.Allocate("%tmp.");
      Borrow(node->IR_name_, result, GetIRTypeString(node->type_info_.get()));
    }
  } else if (node->type_ == kMethodCallExpr) {
    node->expr1_->Accept(this);
    std::string struct_pointer = node->expr1_->IR_name_;
    Type *struct_type = node->expr1_->type_info_.get();
    if (node->expr1_->type_info_->type_ == kPointerType) {
      struct_pointer = name_allocator_.Allocate("%tmp.");
      struct_type = struct_type->pointer_type_.get();
      Dereference(struct_pointer, node->expr1_->IR_name_);
    }
    auto function_node = dynamic_cast<FunctionNode *>(dynamic_cast<StructNode *>(struct_type->source_)
      ->impl_.find(node->path_expr_segment_->identifier_->val_)->second);
    if (function_node->IR_name_.empty()) {
      function_node->IR_name_ = name_allocator_.Allocate("function.." + function_node->identifier_->val_);
    }
    std::shared_ptr<IRCallInstructionNode> IR_call;
    std::string result;
    if (node->type_info_->type_ != kUnitType) {
      result = name_allocator_.Allocate("%tmp.");
      IR_call = std::make_shared<IRCallInstructionNode>(result, GetIRTypeString(node->type_info_.get()), function_node->IR_name_);
    } else {
      IR_call = std::make_shared<IRCallInstructionNode>("", "", function_node->IR_name_);
    }
    if (function_node->function_parameters_->self_param_->shorthand_self_->quote_) {
      IR_call->AddArgument(std::make_shared<IRArgumentNode>("ptr", struct_pointer));
    } else {
      std::string tmp = name_allocator_.Allocate("%tmp.");
      std::string IR_type = GetIRTypeString(struct_type);
      cur_block_->AddInstruction(std::make_shared<IRLoadInstructionNode>(tmp, IR_type, struct_pointer));
      IR_call->AddArgument(std::make_shared<IRArgumentNode>(IR_type, tmp));
    }
    if (node->call_params_ != nullptr) {
      for (auto &expr : node->call_params_->exprs_) {
        expr->Accept(this);
        std::string tmp = name_allocator_.Allocate("%tmp.");
        std::string IR_type = GetIRTypeString(expr->type_info_.get());
        cur_block_->AddInstruction(std::make_shared<IRLoadInstructionNode>(tmp, IR_type, expr->IR_name_));
        IR_call->AddArgument(std::make_shared<IRArgumentNode>(IR_type, tmp));
      }
    }
    cur_block_->AddInstruction(IR_call);
    if (!result.empty()) {
      node->IR_name_ = name_allocator_.Allocate("%tmp.");
      Borrow(node->IR_name_, result, GetIRTypeString(node->type_info_.get()));
    }
  } else if (node->type_ == kContinueExpr) {
    if (loop_condition_block_.top() != nullptr) {
      cur_block_->AddInstruction(std::make_shared<IRJumpInstructionNode>(loop_condition_block_.top()->GetID()));
    } else {
      cur_block_->AddInstruction(std::make_shared<IRJumpInstructionNode>(loop_body_block_.top()->GetID()));
    }
  } else if (node->type_ == kBreakExpr) {
    cur_block_->AddInstruction(std::make_shared<IRJumpInstructionNode>(loop_end_block_.top()->GetID()));
  } else if (node->type_ == kReturnExpr) {
    if (node->expr1_ != nullptr) {
      node->expr1_->Accept(this);
      Return(node->expr1_->type_info_.get(), node->expr1_->IR_name_);
    } else {
      cur_block_->AddInstruction(std::make_shared<IRReturnInstructionNode>());
    }
  } else {
    if (node->type_ != kExprWithBlock) {
      std::cerr << "Error! : expect expr with block!";
      exit(-1);
    }
    node->expr_with_block_->Accept(this);
    node->IR_name_ = node->expr_with_block_->IR_name_;
  }
}

void IRGenerator::Visit(FunctionNode *node) {
  if (node->block_expr_->statements_ != nullptr) {
    for (auto &statement : node->block_expr_->statements_->statement_s_) {
      if (statement->item_ != nullptr) {
        if (statement->item_->struct_ != nullptr) {
          statement->item_->struct_->Accept(this);
        }
        if (statement->item_->function_ != nullptr) {
          statement->item_->function_->Accept(this);
        }
      }
    }
  }

  if (node->identifier_->val_ == "main") {
    node->IR_name_ = "main";
  } else if (node->IR_name_.empty()) {
    node->IR_name_ = name_allocator_.Allocate("function.." + node->identifier_->val_);
  }
  std::string IR_type = "void";
  if (node->function_return_type_ != nullptr) {
    IR_type = GetIRTypeString(node->function_return_type_->type_info_.get());
  }
  if (node->identifier_->val_ == "main") {
    IR_type = "i32";
  }
  cur_function_ = std::make_shared<IRFunctionNode>(IR_type, node->IR_name_);
  root_->AddFunction(cur_function_);
  cur_tag_cnt_ = 0;
  cur_block_ = std::make_shared<IRBlockNode>(cur_tag_cnt_);
  cur_function_->AddBlock(cur_block_);
  ++cur_tag_cnt_;
  if (node->function_parameters_ != nullptr) {
    auto self_param = node->function_parameters_->self_param_;
    if (self_param != nullptr) {
      std::string tmp = name_allocator_.Allocate("%tmp.");
      self_param->IR_name_ = name_allocator_.Allocate("%" + current_Self_.top()->identifier_->val_ + ".self");
      cur_function_->AddParameter(std::make_shared<IRParameterNode>(
        GetIRTypeString(self_param->type_info_.get()), tmp));
      Borrow(self_param->IR_name_, tmp, GetIRTypeString(self_param->type_info_.get()));
    }
    for (auto &param : node->function_parameters_->function_params_) {
      auto pattern = param->pattern_no_top_alt_->identifier_pattern_;
      std::string tmp = name_allocator_.Allocate("%tmp.");
      pattern->IR_name_ = name_allocator_.Allocate("%" + pattern->identifier_->val_);
      cur_function_->AddParameter(std::make_shared<IRParameterNode>(
        GetIRTypeString(pattern->type_info_.get()), tmp));
      Borrow(pattern->IR_name_, tmp, GetIRTypeString(pattern->type_info_.get()));
    }
  }

  if (node->block_expr_->statements_ != nullptr) {
    node->block_expr_->statements_->Accept(this);
  } else {
    cur_block_->AddInstruction(std::make_shared<IRReturnInstructionNode>());
  }
}

void IRGenerator::Visit(ItemNode *node) {
  if (node->function_ != nullptr) {
    node->function_->Accept(this);
  } else if (node->struct_ != nullptr) {
    node->struct_->Accept(this);
  }
}

void IRGenerator::Visit(LetStatementNode *node) {
  node->expr_->Accept(this);
  std::string IR_type = GetIRTypeString(node->type_->type_info_.get());
  auto pattern = node->pattern_no_top_alt_->identifier_pattern_;
  pattern->IR_name_ = name_allocator_.Allocate("%" + pattern->identifier_->val_);
  cur_block_->AddInstruction(std::make_shared<IRAllocateInstructionNode>(pattern->IR_name_, IR_type));
  Copy(pattern->IR_name_, node->expr_->IR_name_, node->type_->type_info_.get());
}

void IRGenerator::Visit(ExpressionStatementNode *node) {
  if (node->expr_without_block_ != nullptr) {
    node->expr_without_block_->expr_->Accept(this);
  } else {
    node->expr_with_block_->Accept(this);
  }
}

void IRGenerator::Visit(StatementNode *node) {
  if (node->let_statement_ != nullptr) {
    node->let_statement_->Accept(this);
  } else if (node->expr_statement_ != nullptr) {
    node->expr_statement_->Accept(this);
  }
}

void IRGenerator::Visit(StatementsNode *node) {
  for (auto &statement : node->statement_s_) {
    statement->Accept(this);
  }
  if (node->expr_without_block_ != nullptr) {
    node->expr_without_block_->expr_->Accept(this);
    Return(node->expr_without_block_->type_info_.get(), node->expr_without_block_->expr_->IR_name_);
  } else {
    auto trailer_statement = *node->statement_s_.rbegin();
    if (trailer_statement->expr_statement_ != nullptr
      && trailer_statement->expr_statement_->semicolon_ == false) {
      Return(trailer_statement->expr_statement_->type_info_.get(), trailer_statement->expr_statement_->expr_with_block_->IR_name_);
    } else {
      cur_block_->AddInstruction(std::make_shared<IRReturnInstructionNode>());
    }
  }
}

void IRGenerator::Visit(StructNode *node) {
  node->IR_name_ = name_allocator_.Allocate("struct.." + node->identifier_->val_);
  auto IR_struct = std::make_shared<IRStructNode>(node->IR_name_);
  for (auto &struct_field : node->struct_fields_->struct_field_s_) {
    IR_struct->AddMember(GetIRTypeString(struct_field->type_->type_info_.get()));
  }
  root_->AddStruct(IR_struct);

  current_Self_.emplace(node);
  for (auto &[name, item] : node->impl_) {
    auto func = dynamic_cast<FunctionNode *>(item);
    if (func != nullptr) {
      func->Accept(this);
    }
  }
  current_Self_.pop();
}

// name1 = &name2, name1 is not allocated at beginning
void IRGenerator::Borrow(const std::string &name1, const std::string &name2, const std::string &type) {
  cur_block_->AddInstruction(std::make_shared<IRAllocateInstructionNode>(name1, type));
  cur_block_->AddInstruction(std::make_shared<IRStoreVariableInstructionNode>(type, name2, name1));
}

// name1 = *name2 (semantic)
// don't use it to implement %1(i32) = *%2(ptr)
void IRGenerator::Dereference(const std::string &name1, const std::string &name2) {
  cur_block_->AddInstruction(std::make_shared<IRLoadInstructionNode>(name1, "ptr", name2));
}

void IRGenerator::Return(Type *type, const std::string &name) {
  if (type->type_ == kNeverType) {
    return;
  }
  if (type->type_ == kUnitType) {
    cur_block_->AddInstruction(std::make_shared<IRReturnInstructionNode>());
  } else {
    std::string tmp = name_allocator_.Allocate("%tmp.");
    std::string IR_type = GetIRTypeString(type);
    cur_block_->AddInstruction(std::make_shared<IRLoadInstructionNode>(tmp, IR_type, name));
    cur_block_->AddInstruction(std::make_shared<IRReturnInstructionNode>(IR_type, tmp));
  }
}

// Generate the IR that copy 'name2' (a (temp) variable) to 'name1'. 'type' is their type.
// name1 is allocated
void IRGenerator::Copy(const std::string &name1, const std::string &name2, Type *type) {
  if (type->type_ == kLeafType || type->type_ == kEnumType || type->type_ == kPointerType) {
    std::string IR_type_name = "i1";
    if (type->type_ == kPointerType) {
      IR_type_name = "ptr";
    } else if (type->type_name_ != "bool") {
      IR_type_name = "i32";
    }
    std::string tmp1 = name_allocator_.Allocate("%tmp.");
    cur_block_->AddInstruction(std::make_shared<IRLoadInstructionNode>(tmp1, IR_type_name, name2));
    cur_block_->AddInstruction(std::make_shared<IRStoreVariableInstructionNode>(IR_type_name, tmp1, name1));
  } else if (type->type_ == kArrayType) {
    std::string IR_type = GetIRTypeString(type);
    for (uint32_t i = 0; i < type->array_type_info_.second; ++i) {
      std::string tmp1 = name_allocator_.Allocate("%tmp.");
      cur_block_->AddInstruction(std::make_shared<IRGetElementPtrInstructionNode>(tmp1, IR_type, name1, i));
      std::string tmp2 = name_allocator_.Allocate("%tmp.");
      cur_block_->AddInstruction(std::make_shared<IRGetElementPtrInstructionNode>(tmp2, IR_type, name2, i));
      Copy(tmp1, tmp2, type->array_type_info_.first.get());
    }
  } else if (type->type_ == kStructType) {
    std::string IR_type = GetIRTypeString(type);
    auto field = dynamic_cast<StructNode *>(type->source_)->struct_fields_->struct_field_s_;
    uint32_t size = field.size();
    for (uint32_t i = 0; i < size; ++i) {
      std::string tmp1 = name_allocator_.Allocate("%tmp.");
      cur_block_->AddInstruction(std::make_shared<IRGetElementPtrInstructionNode>(tmp1, IR_type, name1, i));
      std::string tmp2 = name_allocator_.Allocate("%tmp.");
      cur_block_->AddInstruction(std::make_shared<IRGetElementPtrInstructionNode>(tmp2, IR_type, name2, i));
      Copy(tmp1, tmp2, field[i]->type_->type_info_.get());
    }
  } else {
    std::cerr << "Error: try to copy strange type!\n";
    exit(-1);
  }
}

void IRGenerator::Visit(LiteralExpressionNode *node) {}
void IRGenerator::Visit(PathInExpressionNode *node) {}
void IRGenerator::Visit(ArrayElementsNode *node) {}
void IRGenerator::Visit(ArrayExpressionNode *node) {}
void IRGenerator::Visit(StructExprFieldNode *node) {}
void IRGenerator::Visit(StructExprFieldsNode *node) {}
void IRGenerator::Visit(StructExpressionNode *node) {}
void IRGenerator::Visit(CallParamsNode *node) {}
void IRGenerator::Visit(ExpressionWithoutBlockNode *node) {}
void IRGenerator::Visit(EnumVariantsNode *node) {}
void IRGenerator::Visit(EnumerationNode *node) {}
void IRGenerator::Visit(ShorthandSelfNode *node) {}
void IRGenerator::Visit(SelfParamNode *node) {}
void IRGenerator::Visit(FunctionParamNode *node) {}
void IRGenerator::Visit(FunctionParametersNode *node) {}
void IRGenerator::Visit(FunctionReturnTypeNode *node) {}
void IRGenerator::Visit(IdentifierPatternNode *node) {}
void IRGenerator::Visit(ReferencePatternNode *node) {}
void IRGenerator::Visit(PatternWithoutRangeNode *node) {}
void IRGenerator::Visit(CharLiteralNode *node) {}
void IRGenerator::Visit(StringLiteralNode *node) {}
void IRGenerator::Visit(RawStringLiteralNode *node) {}
void IRGenerator::Visit(CStringLiteralNode *node) {}
void IRGenerator::Visit(RawCStringLiteralNode *node) {}
void IRGenerator::Visit(IntegerLiteralNode *node) {}
void IRGenerator::Visit(TrueNode *node) {}
void IRGenerator::Visit(FalseNode *node) {}
void IRGenerator::Visit(SelfLowerNode *node) {}
void IRGenerator::Visit(SelfUpperNode *node) {}
void IRGenerator::Visit(ContinueExpressionNode *node) {}
void IRGenerator::Visit(TraitNode *node) {}
void IRGenerator::Visit(ReferenceTypeNode *node) {}
void IRGenerator::Visit(ArrayTypeNode *node) {}
void IRGenerator::Visit(UnitTypeNode *node) {}
void IRGenerator::Visit(TypeNoBoundsNode *node) {}
void IRGenerator::Visit(StructFieldNode *node) {}
void IRGenerator::Visit(StructFieldsNode *node) {}
void IRGenerator::Visit(PathIdentSegmentNode *node) {}
void IRGenerator::Visit(ImplementationNode *node) {}
void IRGenerator::Visit(ConstantItemNode *node) {}
void IRGenerator::Visit(AssociatedItemNode *node) {}
void IRGenerator::Visit(IdentifierNode *node) {}