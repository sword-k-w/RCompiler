#include "visitor/checker/third_checker.h"

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

#include "common/error.h"
#include "visitor/printer/printer.h"

void ThirdChecker::Visit(CrateNode *node) {
  try {
    for (auto &item: node->items_) {
      item->Accept(this);
    }
    if (!main_exist_) {
      throw Error("ThirdChecker : no main");
    }
  } catch (Error &) { throw; }
}

void ThirdChecker::Visit(EnumVariantsNode *node) {}

void ThirdChecker::Visit(EnumerationNode *node) {}

void ThirdChecker::Visit(LiteralExpressionNode *node) {
  try {
    node->type_info_ = node->const_value_->GetType();
  } catch (Error &) { throw; }
}

void ThirdChecker::Visit(ArrayElementsNode *node) {
  try {
    if (node->const_value_ != nullptr) {
      node->type_info_ = node->const_value_->GetType();
      return;
    }
    for (auto &expr: node->exprs_) {
      expr->Accept(this);
    }
    node->type_info_ = std::make_shared<Type>();
    node->type_info_->type_ = kArrayType;
    if (node->semicolon_) {
      node->type_info_->array_type_info_.first = node->exprs_[0]->type_info_;
      if (!ExpectUsize(node->exprs_[1]->type_info_.get())) {
        throw Error("ThirdChecker : array elements but the length is not usize");
      }
      node->type_info_->array_type_info_.second = node->exprs_[1]->const_value_->u32_value_;
    } else {
      node->type_info_->array_type_info_.second = node->exprs_.size();
      for (uint32_t i = 1; i < node->type_info_->array_type_info_.second; ++i) {
        SameTypeCheck(node->exprs_[0]->type_info_.get(), node->exprs_[i]->type_info_.get());
      }
      node->type_info_->array_type_info_.first = node->exprs_[0]->type_info_;
    }
  } catch (Error &) { throw; }
}

void ThirdChecker::Visit(ArrayExpressionNode *node) {
  try {
    if (node->const_value_ != nullptr) {
      node->type_info_ = node->const_value_->GetType();
      return;
    }
    node->array_elements_->Accept(this);
    node->type_info_ = node->array_elements_->type_info_;
  } catch (Error &) { throw; }
}

void ThirdChecker::Visit(PathInExpressionNode *node) {}

void ThirdChecker::Visit(StructExprFieldNode *node) {
  try {
    if (node->const_value_ != nullptr) {
      node->type_info_ = node->const_value_->GetType();
      return;
    }
    node->expr_->Accept(this);
  } catch (Error &) { throw; }
}

void ThirdChecker::Visit(StructExprFieldsNode *node) {
  try {
    for (auto &struct_expr_field: node->struct_expr_field_s_) {
      struct_expr_field->Accept(this);
    }
  } catch (Error &) { throw; }
}

void ThirdChecker::Visit(StructExpressionNode *node) {
  try {
    if (node->const_value_ != nullptr) {
      node->type_info_ = node->const_value_->GetType();
      return;
    }
    if (node->path_in_expr_->path_expr_segment2_ != nullptr) {
      throw Error("ThirdChecker : struct but not single path");
    }
    if (node->path_in_expr_->path_expr_segment1_->identifier_ == nullptr) {
      throw Error("ThirdChecker : struct but not identifier");
    }
    ASTNode *target = node->scope_->FindTypeName(node->path_in_expr_->path_expr_segment1_->identifier_->val_);
    if (target == nullptr) {
      throw Error("ThirdChecker : struct but not found identifier");
    }
    auto struct_type = dynamic_cast<StructNode *>(target);
    if (struct_type == nullptr) {
      throw Error("ThirdChecker : struct but identifier is not a struct");
    }
    if (node->struct_expr_fields_ != nullptr) {
      if (struct_type->field_.size() != node->struct_expr_fields_->struct_expr_field_s_.size()) {
        throw Error("ThirdChecker : struct but identifier count doesn't match");
      }
      node->struct_expr_fields_->Accept(this);
      for (auto &struct_expr_field: node->struct_expr_fields_->struct_expr_field_s_) {
        auto it = struct_type->field_.find(struct_expr_field->identifier_->val_);
        if (it == struct_type->field_.end()) {
          throw Error("ThirdChecker : struct but the identifier doesn't belong to the struct");
        }
        SameTypeCheck(it->second.get(), struct_expr_field->expr_->type_info_.get());
      }
    } else {
      if (!struct_type->field_.empty()) {
        throw Error("ThirdChecker : struct but no field");
      }
    }
    node->type_info_ = std::make_shared<Type>();
    node->type_info_->type_ = kStructType;
    node->type_info_->type_name_ = struct_type->identifier_->val_;
    node->type_info_->source_ = target;
  } catch (Error &) { throw; }
}

void ThirdChecker::Visit(ExpressionWithoutBlockNode *node) {
  try {
    if (node->const_value_ != nullptr) {
      node->type_info_ = node->const_value_->GetType();
      return;
    }
    node->expr_->Accept(this);
    node->type_info_ = node->expr_->type_info_;
  } catch (Error &) { throw; }
}

void ThirdChecker::Visit(BlockExpressionNode *node) {
  try {
    if (node->statements_ != nullptr) {
      node->statements_->Accept(this);
      node->type_info_ = node->statements_->type_info_;
    } else {
      node->type_info_ = std::make_shared<Type>();
      node->type_info_->type_ = kUnitType;
    }
  } catch (Error &) { throw; }
}

void ThirdChecker::Visit(InfiniteLoopExpressionNode *node) {
  try {
    node->block_expr_->Accept(this);
    if (node->block_expr_->type_info_->type_ != kUnitType && node->block_expr_->type_info_->type_ != kNeverType) {
      throw Error("ThirdChecker : loop expression but the type of the body is not unit or never type");
    }
    node->type_info_ = node->block_expr_->type_info_;
  } catch (Error &) { throw; }
}

void ThirdChecker::Visit(ConditionsNode *node) {
  try {
    node->expr_->Accept(this);
    if (node->expr_->type_info_->type_name_ != "bool") {
      throw Error("ThirdChecker : condition but not bool type");
    }
  } catch (Error &) { throw; }
}

void ThirdChecker::Visit(PredicateLoopExpressionNode *node) {
  try {
    node->conditions_->Accept(this);
    node->block_expr_->Accept(this);
    if (node->block_expr_->type_info_->type_ != kUnitType && node->block_expr_->type_info_->type_ != kNeverType) {
      throw Error("ThirdChecker : loop expression but the type of the body is not unit or never  type");
    }
    node->type_info_ = node->block_expr_->type_info_;
  } catch (Error &) { throw; }
}

void ThirdChecker::Visit(LoopExpressionNode *node) {
  try {
    current_loop_.emplace(node);
    if (node->infinite_loop_expr_ != nullptr) {
      node->infinite_loop_expr_->Accept(this);
      if (!node->assigned_) {
        node->type_info_ = node->infinite_loop_expr_->type_info_;
      } else {
        SameTypeCheck(node->type_info_.get(), node->infinite_loop_expr_->type_info_.get());
      }
    } else {
      node->predicate_loop_expr_->Accept(this);
      if (!node->assigned_) {
        node->type_info_ = node->predicate_loop_expr_->type_info_;
      } else {
        SameTypeCheck(node->type_info_.get(), node->predicate_loop_expr_->type_info_.get());
      }
    }
    current_loop_.pop();
  } catch (Error &) { throw; }
}

void ThirdChecker::Visit(IfExpressionNode *node) {
  try {
    node->conditions_->Accept(this);
    node->block_expr1_->Accept(this);
    node->type_info_ = node->block_expr1_->type_info_;
    if (node->block_expr2_ != nullptr) {
      node->block_expr2_->Accept(this);
      SameTypeCheck(node->type_info_.get(), node->block_expr2_->type_info_.get());
    } else if (node->if_expr_ != nullptr) {
      node->if_expr_->Accept(this);
      SameTypeCheck(node->type_info_.get(), node->if_expr_->type_info_.get());
    } else {
      if (node->type_info_->type_ != kUnitType && node->type_info_->type_ != kNeverType) {
        throw Error("ThirdChecker : if expr but type not match");
      }
    }
  } catch (Error &) { throw; }
}

void ThirdChecker::Visit(ExpressionWithBlockNode *node) {
  try {
    if (node->block_expr_ != nullptr) {
      node->block_expr_->Accept(this);
      node->type_info_ = node->block_expr_->type_info_;
    } else if (node->if_expr_ != nullptr) {
      node->if_expr_->Accept(this);
      node->type_info_ = node->if_expr_->type_info_;
    } else {
      node->loop_expr_->Accept(this);
      node->type_info_ = node->loop_expr_->type_info_;
    }
  } catch (Error &) { throw; }
}

void ThirdChecker::Visit(CallParamsNode *node) {
  try {
    for (auto &expr: node->exprs_) {
      expr->Accept(this);
    }
  } catch (Error &) { throw; }
}

void ThirdChecker::Visit(ExpressionNode *node) {
  try {
    if (node->const_value_ != nullptr) {
      node->type_info_ = node->const_value_->GetType();
      return;
    }
    if (node->type_ == kLiteralExpr) {
      node->literal_expr_->Accept(this);
      node->type_info_ = node->literal_expr_->type_info_;
    } else if (node->type_ == kPathExpr) {
      node->path_expr_->Accept(this);
      ASTNode *target = nullptr;
      if (node->path_expr_->path_expr_segment2_ != nullptr) {
        if (node->path_expr_->path_expr_segment1_->identifier_->val_ == "String") {
          if (node->path_expr_->path_expr_segment2_->identifier_->val_ != "from") {
            throw Error("ThirdChecker : unknown function");
          }
          node->type_info_ = std::make_shared<Type>();
          node->type_info_->type_ = kFunctionCallType;
          builtin_.emplace_back(std::make_shared<BuiltinFunctionNode>("from"));
          node->type_info_->source_ = builtin_.back().get();
          return;
        }
        ASTNode *tmp = node->scope_->FindTypeName(node->path_expr_->path_expr_segment1_->identifier_->val_);
        auto *struct_node = dynamic_cast<StructNode *>(tmp);
        if (struct_node != nullptr) {
          auto it = struct_node->impl_.find(node->path_expr_->path_expr_segment2_->identifier_->val_);
          if (it == struct_node->impl_.end()) {
            throw Error("ThirdChecker : cannot resolve path expr");
          }
          target = it->second;
          auto const_item = dynamic_cast<ConstantItemNode *>(target);
          if (const_item != nullptr) {
            node->type_info_ = const_item->const_value_->GetType();
          } else {
            node->type_info_ = std::make_shared<Type>();
            node->type_info_->type_ = kFunctionCallType;
            node->type_info_->source_ = target;
          }
        } else {
          auto enum_node = dynamic_cast<EnumerationNode *>(tmp);
          if (enum_node != nullptr) {
            if (enum_node->enum_.find(node->path_expr_->path_expr_segment2_->identifier_->val_) == enum_node->enum_.
                end()) {
              throw Error("ThirdChecker : the variant doesn't exist in enumeration");
            }
            node->type_info_ = std::make_shared<Type>();
            node->type_info_->source_ = tmp;
            node->type_info_->type_ = kEnumType;
            node->type_info_->type_name_ = enum_node->identifier_->val_;
          } else {
            auto trait_node = dynamic_cast<TraitNode *>(tmp);
            assert(trait_node != nullptr);
            auto it = trait_node->items_.find(node->path_expr_->path_expr_segment2_->identifier_->val_);
            if (it == trait_node->items_.end()) {
              throw Error("ThirdChecker : the identifier doesn't exist in trait");
            }
            auto const_item = dynamic_cast<ConstantItemNode *>(it->second);
            if (const_item != nullptr) {
              if (const_item->expr_ == nullptr) {
                throw Error("ThirdChecker : want trait const value but no default value");
              }
              node->type_info_ = const_item->const_value_->GetType();
            } else {
              throw Error("ThirdChecker : unexpected path");
            }
          }
        }
      } else {
        std::string val = "";
        if (node->path_expr_->path_expr_segment1_->identifier_ != nullptr) {
          val = node->path_expr_->path_expr_segment1_->identifier_->val_;
        } else if (node->path_expr_->path_expr_segment1_->self_lower_ != nullptr) {
          val = node->path_expr_->path_expr_segment1_->self_lower_->val_;
        } else {
          throw Error("ThirdChecker : path expr but Self");
        }
        if (IsBuiltinFunction(val)) {
          node->type_info_ = std::make_shared<Type>();
          node->type_info_->type_ = kFunctionCallType;
          builtin_.emplace_back(std::make_shared<BuiltinFunctionNode>(val));
          node->type_info_->source_ = builtin_.back().get();
          return;
        }
        target = node->scope_->FindValueName(val);
        auto *struct_node = dynamic_cast<StructNode *>(target);
        if (struct_node != nullptr) {
          if (struct_node->struct_fields_ != nullptr) {
            throw Error("ThirdChecker : expect unit-like struct");
          }
          node->type_info_ = std::make_shared<Type>();
          node->type_info_->type_ = kStructType;
          node->type_info_->source_ = target;
          node->type_info_->type_name_ = struct_node->identifier_->val_;
        } else {
          auto *const_item = dynamic_cast<ConstantItemNode *>(target);
          if (const_item != nullptr) {
            node->type_info_ = const_item->const_value_->GetType();
          } else {
            auto *identifier_pattern = dynamic_cast<IdentifierPatternNode *>(target);
            if (identifier_pattern != nullptr) {
              node->type_info_ = identifier_pattern->type_info_;
            } else {
              auto *function_node = dynamic_cast<FunctionNode *>(target);
              if (function_node != nullptr) {
                if (function_node->function_parameters_ != nullptr && function_node->function_parameters_->self_param_ != nullptr) {
                  throw Error("ThirdChecker : invoke method in the way of associated function");
                }
                node->type_info_ = std::make_shared<Type>();
                node->type_info_->type_ = kFunctionCallType;
                node->type_info_->source_ = target;
              } else {
                auto *self_param = dynamic_cast<SelfParamNode *>(target);
                if (self_param != nullptr) {
                  node->type_info_ = std::make_shared<Type>(*self_param->type_info_);
                  node->type_info_->is_mut_left_ = self_param->shorthand_self_->mut_;
                } else {
                  throw Error("ThirdChecker : unexpected path");
                }
              }
            }
          }
        }
      }
    } else if (node->type_ == kArrayExpr) {
      node->array_expr_->Accept(this);
      node->type_info_ = node->array_expr_->type_info_;
    } else if (node->type_ == kStructExpr) {
      node->struct_expr_->Accept(this);
      node->type_info_ = node->struct_expr_->type_info_;
    } else if (node->type_ == kContinueExpr) {
      if (current_loop_.empty()) {
        throw Error("ThirdChecker : continue expr but no loop");
      }
      node->continue_expr_->Accept(this);
      node->type_info_ = node->continue_expr_->type_info_;
    } else if (node->type_ == kUnderscoreExpr) {
      node->type_info_ = std::make_shared<Type>();
      node->type_info_->type_ = kUnitType;
    } else if (node->type_ == kBorrowExpr) {
      node->expr1_->Accept(this);
      if (node->expr1_->type_info_->type_ == kUnitType || node->expr1_->type_info_->type_ == kNeverType || node->expr1_->type_info_->type_ == kFunctionCallType) {
        throw Error("ThirdChecker : borrow expr but unexpected type");
      }
      auto tmp = std::make_shared<Type>();
      tmp->type_= kPointerType;
      tmp->pointer_type_ = node->expr1_->type_info_;
      if (node->op_ == "&&") {
        node->type_info_ = std::make_shared<Type>();
        node->type_info_->type_ = kPointerType;
        node->type_info_->pointer_type_ = tmp;
      } else {
        node->type_info_ = tmp;
      }
    } else if (node->type_ == kDereferenceExpr) {
      node->expr1_->Accept(this);
      if (node->expr1_->type_info_->type_ != kPointerType) {
        throw Error("ThirdChecker : dereference expr but not reference type");
      }
      node->type_info_ = node->expr1_->type_info_->pointer_type_;
    } else if (node->type_ == kNegationExpr) {
      node->expr1_->Accept(this);
      if (node->expr1_->type_info_->type_ != kLeafType || node->expr1_->type_info_->type_name_ == "str" || node->expr1_->type_info_->type_name_ == "char" || node->expr1_->type_info_->type_name_ == "String") {
        throw Error("ThirdChecker : negation with unexpected type");
      }
      node->type_info_ = std::make_shared<Type>(*node->expr1_->type_info_);
      if (node->op_ == "-") {
        if (!IsSignedIntegerType(node->type_info_->type_name_)) {
          throw Error("ThirdChecker : negation with unexpected type");
        }
      } else {
        if (node->type_info_->type_name_ != "bool" && !IsSignedIntegerType(node->type_info_->type_name_)) {
          throw Error("ThirdChecker : negation with unexpected type");
        }
      }
      node->type_info_->is_mut_left_ = false;
    } else if (node->type_ == kArithmeticOrLogicExpr) {
      node->expr1_->Accept(this);
      node->expr2_->Accept(this);
      if (node->expr1_->type_info_->type_ != kLeafType || node->expr2_->type_info_->type_ != kLeafType) {
        throw Error("ThirdChecker : arithmetic or logic expr but not leaf type");
      }
      if (node->expr1_->type_info_->type_name_ == "char" || node->expr2_->type_info_->type_name_ == "char") {
        throw Error("ThirdChecker : arithmetic or logic expr but char");
      }
      node->type_info_ = std::make_shared<Type>(*node->expr1_->type_info_);
      node->type_info_->is_mut_left_ = false;
      if (node->op_ == "<<" || node->op_ == ">>") {
        if (node->expr1_->type_info_->type_name_ == "bool" || node->expr2_->type_info_->type_name_ == "bool") {
          throw Error("ThirdChecker : shift but bool");
        }
        return;
      }
      node->type_info_->type_name_ = MergeLeafType(node->expr1_->type_info_->type_name_, node->expr2_->type_info_->type_name_);
      if (node->expr1_->type_info_->type_name_ == "bool") {
        if (node->op_ != "&" && node->op_ != "|" && node->op_ != "^") {
          throw Error("ThirdChecker : arithmetic or logic expr but bool with unexpected operation");
        }
      }
    } else if (node->type_ == kComparisonExpr) {
      node->expr1_->Accept(this);
      node->expr2_->Accept(this);
      auto [type1, dep1] = AutoDereference(node->expr1_->type_info_.get());
      auto [type2, dep2] = AutoDereference(node->expr2_->type_info_.get());
      if (dep1 != dep2) {
        throw Error("ThirdChecker : auto dereference but the depths don't match");
      }
      SameTypeCheck(type1, type2);
      if (node->op_ != "!=" && node->op_ != "==" && (type1->type_ != kLeafType || type2->type_ != kLeafType)) {
        throw Error("ThirdChecker : comparison expr but not leaf type");
      }
      node->type_info_ = std::make_shared<Type>();
      node->type_info_->type_ = kLeafType;
      node->type_info_->type_name_ = "bool";
    } else if (node->type_ == kLazyBooleanExpr) {
      node->expr1_->Accept(this);
      node->expr2_->Accept(this);
      if (node->expr1_->type_info_->type_ != kLeafType || node->expr2_->type_info_->type_ != kLeafType) {
        throw Error("ThirdChecker : lazy boolean expr but not leaf type");
      }
      if (node->expr1_->type_info_->type_name_ != "bool" || node->expr2_->type_info_->type_name_ != "bool") {
        throw Error("ThirdChecker : lazy boolean expr but not bool");
      }
      node->type_info_ = node->expr1_->type_info_;
      node->type_info_->is_mut_left_ = false;
    } else if (node->type_ == kTypeCastExpr) {
      node->expr1_->Accept(this);
      if (node->expr1_->type_info_->type_ != kLeafType || node->type_no_bounds_->type_info_->type_ != kLeafType) {
        throw Error("ThirdChecker : type cast expr but not leaf type");
      }
      if (node->expr1_->type_info_->type_name_ == "str" || node->type_no_bounds_->type_info_->type_name_ == "str") {
        throw Error("ThirdChecker : type cast expr but str");
      }
      if (node->expr1_->type_info_->type_name_ == "String" || node->type_no_bounds_->type_info_->type_name_ == "String") {
        throw Error("ThirdChecker : type cast expr but String");
      }
      if (node->type_no_bounds_->type_info_->type_name_ == "char") {
        throw Error("ThirdChecker : type cast expr but char");
      }
      if (node->type_no_bounds_->type_info_->type_name_ == "bool") {
        throw Error("ThirdChecker : type cast expr but bool");
      }
      node->type_info_ = node->type_no_bounds_->type_info_;
    } else if (node->type_ == kAssignmentExpr) {
      node->expr1_->Accept(this);
      node->expr2_->Accept(this);
      if (!node->expr1_->type_info_->is_mut_left_) {
        throw Error("ThirdChecker : assign expr but not mutable left value");
      }
      SameTypeCheck(node->expr1_->type_info_.get(), node->expr2_->type_info_.get());
      node->type_info_ = std::make_shared<Type>();
      node->type_info_->type_ = kUnitType;
    } else if (node->type_ == kCompoundAssignmentExpr) {
      node->expr1_->Accept(this);
      node->expr2_->Accept(this);
      if (!node->expr1_->type_info_->is_mut_left_) {
        throw Error("ThirdChecker : compound assign expr but not mutable left value");
      }
      if (node->expr1_->type_info_->type_ != kLeafType || node->expr2_->type_info_->type_ != kLeafType) {
        throw Error("ThirdChecker : compound assign expr but not leaf type");
      }
      if (node->expr1_->type_info_->type_name_ == "char" || node->expr2_->type_info_->type_name_ == "char") {
        throw Error("ThirdChecker : compound assign expr but char");
      }
      node->type_info_ = std::make_shared<Type>(*node->expr1_->type_info_);
      node->type_info_->is_mut_left_ = false;
      if (node->op_ == "<<=" || node->op_ == ">>=") {
        if (node->expr1_->const_value_->type_name_ == "bool" || node->expr2_->const_value_->type_name_ == "bool") {
          throw Error("ThirdChecker : shift but str or bool");
        }
        if (IsSignedIntegerType(node->expr2_->const_value_->type_name_)) {
          if (static_cast<int32_t>(node->expr2_->const_value_->u32_value_) < 0) {
            throw Error("ThirdChecker : const negative shift");
          }
        }
        return;
      }
      node->type_info_->type_name_ = MergeLeafType(node->expr1_->type_info_->type_name_, node->expr2_->type_info_->type_name_);
      if (node->expr1_->type_info_->type_name_ == "bool") {
        if (node->op_ != "&=" && node->op_ != "|=" && node->op_ != "^=") {
          throw Error("ThirdChecker : compound assign expr but bool with unexpected operation");
        }
      }
    } else if (node->type_ == kGroupedExpr) {
      node->expr1_->Accept(this);
      node->type_info_ = node->expr1_->type_info_;
    } else if (node->type_ == kIndexExpr) {
      node->expr1_->Accept(this);
      node->expr2_->Accept(this);
      auto [type, dep] = AutoDereference(node->expr1_->type_info_.get());
      if (type->type_ != kArrayType) {
        throw Error("ThirdChecker : index expr but not array");
      }
      if (!ExpectUsize(node->expr2_->type_info_.get())) {
        throw Error("ThirdChecker : index expr but index is not usize");
      }
      node->type_info_ = std::make_shared<Type>(*type->array_type_info_.first);
      node->type_info_->is_mut_left_ = type->is_mut_left_;
    } else if (node->type_ == kCallExpr) {
      node->expr1_->Accept(this);
      if (node->call_params_ != nullptr) {
        node->call_params_->Accept(this);
      }
      if (node->expr1_->type_info_->type_ != kFunctionCallType) {
        throw Error("ThirdChecker : call expr but not function path");
      }
      auto builtin_function = dynamic_cast<BuiltinFunctionNode *>(node->expr1_->type_info_->source_);
      if (builtin_function != nullptr) {
        if (builtin_function->function_name_ == "getString") {
          if (node->call_params_ != nullptr) {
            throw Error("ThirdChecker : getString but parameter is not empty");
          }
          node->type_info_ = std::make_shared<Type>();
          node->type_info_->type_ = kLeafType;
          node->type_info_->type_name_ = "String";
          return;
        }
        if (builtin_function->function_name_ == "getInt") {
          if (node->call_params_ != nullptr) {
            throw Error("ThirdChecker : getInt but parameter is not empty");
          }
          node->type_info_ = std::make_shared<Type>();
          node->type_info_->type_ = kLeafType;
          node->type_info_->type_name_ = "i32";
          return;
        }
        if (builtin_function->function_name_ == "exit") {
          if (node->call_params_ == nullptr || node->call_params_->exprs_.size() != 1) {
            throw Error("ThirdChecker : exit but parameter count is not 1");
          }
          if (!ExpectI32(node->call_params_->exprs_[0]->type_info_.get())) {
            throw Error("ThirdChecker : exit but not i32");
          }
          if (current_function_.size() != 1 || current_function_.top()->identifier_->val_ != "main") {
            throw Error("ThirdChecker : exit but not in main");
          }
        } else if (builtin_function->function_name_ == "print" || builtin_function->function_name_ == "println") {
          if (node->call_params_ == nullptr || node->call_params_->exprs_.size() != 1) {
            throw Error("ThirdChecker : print/println but parameter count is not 1");
          }
          if (!ExpectStr(node->call_params_->exprs_[0]->type_info_.get())) {
            throw Error("ThirdChecker : print/println but not &str");
          }
        } else if (builtin_function->function_name_ == "printInt" || builtin_function->function_name_ == "printlnInt") {
          if (node->call_params_ == nullptr || node->call_params_->exprs_.size() != 1) {
            throw Error("ThirdChecker : printInt/printlnInt but parameter count is not 1");
          }
          if (!ExpectI32(node->call_params_->exprs_[0]->type_info_.get())) {
            throw Error("ThirdChecker : printInt/printlnInt but not i32");
          }
        } else {
          assert(builtin_function->function_name_ == "from");
          if (node->call_params_ == nullptr || node->call_params_->exprs_.size() != 1) {
            throw Error("ThirdChecker : from but parameter count is not 1");
          }
          if (!ExpectStr(node->call_params_->exprs_[0]->type_info_.get())) {
            throw Error("ThirdChecker : from but not &str");
          }
          node->type_info_ = std::make_shared<Type>();
          node->type_info_->type_ = kLeafType;
          node->type_info_->type_name_ = "String";
          return;
        }
        node->type_info_ = std::make_shared<Type>();
        node->type_info_->type_ = kUnitType;
        return;
      }
      auto function_node = dynamic_cast<FunctionNode *>(node->expr1_->type_info_->source_);
      if (function_node->function_parameters_ == nullptr) {
        if (node->call_params_ != nullptr) {
          throw Error("ThirdChecker : call expr but parameter count doesn't match");
        }
      } else {
        if (function_node->function_parameters_->self_param_ != nullptr) {
          throw Error("ThirdChecker : call expr but invoke method");
        }
        if (node->call_params_ == nullptr || node->call_params_->exprs_.size() != function_node->function_parameters_->function_params_.size()) {
          throw Error("ThirdChecker : call expr but parameter count doesn't match");
        }
        uint32_t size = node->call_params_->exprs_.size();
        for (uint32_t i = 0; i < size; ++i) {
          SameTypeCheck(node->call_params_->exprs_[i]->type_info_.get(), function_node->function_parameters_->function_params_[i]->type_->type_info_.get());
        }
      }
      if (function_node->function_return_type_ == nullptr) {
        node->type_info_ = std::make_shared<Type>();
        node->type_info_->type_ = kUnitType;
      } else {
        node->type_info_ = function_node->function_return_type_->type_info_;
      }
    } else if (node->type_ == kMethodCallExpr) {
      node->expr1_->Accept(this);
      if (node->call_params_ != nullptr) {
        node->call_params_->Accept(this);
      }
      auto [type, dep] = AutoDereference(node->expr1_->type_info_.get());
      if (type->type_ != kStructType) {
        if (ExpectU32(type) || ExpectUsize(type)) {
          if (node->path_expr_segment_->identifier_->val_ != "to_string") {
            throw Error("ThirdChecker : unknown method for u32/usize");
          }
          if (node->call_params_ != nullptr) {
            throw Error("ThirdChecker : to_string but with parameter");
          }
          node->type_info_ = std::make_shared<Type>();
          node->type_info_->type_ = kLeafType;
          node->type_info_->type_name_ = "String";
          return;
        }
        if (type->type_ == kLeafType && type->type_name_ == "String") {
          if (node->path_expr_segment_->identifier_->val_ == "as_str") {
            if (node->call_params_ != nullptr) {
              throw Error("ThirdChecker : as_str but with parameter");
            }
            node->type_info_ = std::make_shared<Type>();
            node->type_info_->type_ = kPointerType;
            node->type_info_->pointer_type_ = std::make_shared<Type>();
            node->type_info_->pointer_type_->type_ = kLeafType;
            node->type_info_->pointer_type_->type_name_ = "str";
            return;
          }
          if (node->path_expr_segment_->identifier_->val_ == "as_mut_str") {
            if (!type->is_mut_left_) {
              throw Error("ThirdChecker : not mut but invoke as_mut_str");
            }
            if (node->call_params_ != nullptr) {
              throw Error("ThirdChecker : as_mut_str but with parameter");
            }
            node->type_info_ = std::make_shared<Type>();
            node->type_info_->type_ = kPointerType;
            node->type_info_->pointer_type_ = std::make_shared<Type>();
            node->type_info_->pointer_type_->type_ = kLeafType;
            node->type_info_->pointer_type_->is_mut_left_ = true;
            node->type_info_->pointer_type_->type_name_ = "str";
            return;
          }
          if (node->path_expr_segment_->identifier_->val_ == "len") {
            if (node->call_params_ != nullptr) {
              throw Error("ThirdChecker : len but with parameter");
            }
            node->type_info_ = std::make_shared<Type>();
            node->type_info_->type_ = kLeafType;
            node->type_info_->type_name_ = "usize";
            return;
          }
          if (node->path_expr_segment_->identifier_->val_ == "append") {
            if (node->call_params_ == nullptr || node->call_params_->exprs_.size() != 1 || !ExpectStr(node->call_params_->exprs_[1]->type_info_.get())) {
              throw Error("ThirdChecker : append but parameter is not &str");
            }
            node->type_info_ = std::make_shared<Type>();
            node->type_info_->type_ = kUnitType;
            return;
          }
          throw Error("ThirdChecker : unknown method for String");
        }
        if (type->type_ == kLeafType && type->type_name_ == "str" || type->type_ == kArrayType) {
          if (node->path_expr_segment_->identifier_->val_ != "len") {
            throw Error("ThirdChecker : unknown method");
          }
          if (node->call_params_ != nullptr) {
            throw Error("ThirdChecker : len but with parameter");
          }
          node->type_info_ = std::make_shared<Type>();
          node->type_info_->type_ = kLeafType;
          node->type_info_->type_name_ = "usize";
          return;
        }
        throw Error("ThirdChecker : method call expr but not struct");
      }
      auto struct_node = dynamic_cast<StructNode *>(type->source_);
      auto it = struct_node->impl_.find(node->path_expr_segment_->identifier_->val_);
      if (it == struct_node->impl_.end()) {
        throw Error("ThirdChecker : method call expr but not found method");
      }
      auto function_node = dynamic_cast<FunctionNode *>(it->second);
      if (function_node == nullptr) {
        throw Error("ThirdChecker : method call expr but the identifier is const");
      }
      if (function_node->function_parameters_ == nullptr || function_node->function_parameters_->self_param_ == nullptr) {
        throw Error("ThirdChecker : method call expr but not method");
      }
      if (function_node->function_parameters_->self_param_->shorthand_self_->mut_ && !type->is_mut_left_) {
        throw Error("ThirdChecker : method call expr but expect mut instance");
      }
      if (function_node->function_parameters_->function_params_.empty()) {
        if (node->call_params_ != nullptr) {
          throw Error("ThirdChecker : method call expr but parameter count doesn't match");
        }
      } else {
        if (node->call_params_ == nullptr || node->call_params_->exprs_.size() != function_node->function_parameters_->function_params_.size()) {
          throw Error("ThirdChecker : method call expr but parameter count doesn't match");
        }
        uint32_t size = node->call_params_->exprs_.size();
        for (uint32_t i = 0; i < size; ++i) {
          SameTypeCheck(node->call_params_->exprs_[i]->type_info_.get(), function_node->function_parameters_->function_params_[i]->type_->type_info_.get());
        }
      }
      if (function_node->function_return_type_ == nullptr) {
        node->type_info_ = std::make_shared<Type>();
        node->type_info_->type_ = kUnitType;
      } else {
        node->type_info_ = function_node->function_return_type_->type_info_;
      }
    } else if (node->type_ == kFieldExpr) {
      node->expr1_->Accept(this);
      auto [type, dep] = AutoDereference(node->expr1_->type_info_.get());
      if (type->type_ != kStructType) {
        throw Error("ThirdChecker : field expr but not struct");
      }
      auto struct_node = dynamic_cast<StructNode *>(type->source_);
      auto it = struct_node->field_.find(node->identifier_->val_);
      if (it == struct_node->field_.end()) {
        throw Error("ThirdChecker : field expr but not found field");
      }
      node->type_info_ = std::make_shared<Type>(*it->second);
      node->type_info_->is_mut_left_ = type->is_mut_left_;
    } else if (node->type_ == kBreakExpr) {
      if (current_loop_.empty()) {
        throw Error("ThirdChecker : no loop but break expr");
      }
      auto loop = current_loop_.top();
      if (node->expr1_ != nullptr) {
        node->expr1_->Accept(this);
        if (!loop->assigned_) {
          loop->type_info_ = node->expr1_->type_info_;
          loop->assigned_ = false;
        } else {
          SameTypeCheck(loop->type_info_.get(), node->expr1_->type_info_.get());
        }
      } else {
        if (!loop->assigned_) {
          loop->type_info_ = std::make_shared<Type>();
          loop->type_info_->type_ = kUnitType;
          loop->assigned_ = true;
        } else if (loop->type_info_->type_ != kUnitType) {
          throw Error("ThirdChecker : conflict type");
        }
      }
      node->type_info_ = std::make_shared<Type>();
      node->type_info_->type_ = kNeverType;
    } else if (node->type_ == kReturnExpr) {
      if (current_function_.empty()) {
        throw Error("ThirdChecker : no function but return expr");
      }
      auto function_node = current_function_.top();
      if (node->expr1_ != nullptr) {
        node->expr1_->Accept(this);
        SameTypeCheck(function_node->type_info_.get(), node->expr1_->type_info_.get());
      } else {
        if (function_node->type_info_->type_ != kUnitType) {
          throw Error("ThirdChecker : conflict type");
        }
      }
      node->type_info_ = std::make_shared<Type>();
      node->type_info_->type_ = kNeverType;
    } else {
      node->expr_with_block_->Accept(this);
      node->type_info_ = node->expr_with_block_->type_info_;
    }
  } catch (Error &) { throw; }
}

void ThirdChecker::Visit(ShorthandSelfNode *node) {}

void ThirdChecker::Visit(SelfParamNode *node) {
  try {
    node->scope_->AddValueName("self", node, false);
    node->type_info_= current_Self_.top()->type_info_;
  } catch (Error &) { throw; }
}

void ThirdChecker::Visit(FunctionParamNode *node) {
  try {
    node->scope_->AddValueName(node->pattern_no_top_alt_->identifier_pattern_->identifier_->val_, node->pattern_no_top_alt_->identifier_pattern_.get(), false);
    node->pattern_no_top_alt_->identifier_pattern_->type_info_ = std::make_shared<Type>(*node->type_->type_info_);
    node->pattern_no_top_alt_->identifier_pattern_->type_info_->is_mut_left_ = node->pattern_no_top_alt_->identifier_pattern_->mut_;
  } catch (Error &) { throw; }
}

void ThirdChecker::Visit(FunctionParametersNode *node) {
  try {
    if (node->self_param_ != nullptr) {
      node->self_param_->Accept(this);
    }
    for (auto &param : node->function_params_) {
      param->Accept(this);
    }
  } catch (Error &) { throw; }
}

void ThirdChecker::Visit(FunctionReturnTypeNode *node) {}

void ThirdChecker::Visit(FunctionNode *node) {
  try {
    if (node->function_parameters_ != nullptr) {
      if (node->function_parameters_->self_param_ != nullptr) {
        if (!node->in_trait_ && !node->in_implement_) {
          throw Error("ThirdChecker : not method but self");
        }
      }
      node->function_parameters_->Accept(this);
    }
    assert(node->block_expr_ != nullptr);
    if (node->identifier_->val_ == "main") {
      if (!current_function_.empty()) {
        throw Error("ThirdChecker : find main in function");
      }
      if (main_exist_) {
        throw Error("ThirdChecker : find multiple main");
      }
      main_exist_ = true;
    }
    current_function_.emplace(node);
    node->block_expr_->Accept(this);
    SameTypeCheck(node->block_expr_->type_info_.get(), node->type_info_.get());
    current_function_.pop();
  } catch (Error &) { throw; }
}

void ThirdChecker::Visit(ImplementationNode *node) {
  try {
    if (node->identifier_ != nullptr) {
      auto trait_node = dynamic_cast<TraitNode *>(node->scope_->FindTypeName(node->identifier_->val_));
      for (auto &associated_item : node->associated_items_) {
        if (associated_item->function_ != nullptr) {
          auto it = trait_node->items_.find(associated_item->function_->identifier_->val_);
          SameTypeCheck(it->second->type_info_.get(), associated_item->function_->function_return_type_->type_info_.get());
          auto para1 = dynamic_cast<FunctionNode *>(it->second)->function_parameters_;
          auto para2 = associated_item->function_->function_parameters_;
          if (para1->self_param_ != nullptr) {
            if (para2->self_param_ == nullptr) {
              throw Error("ThirdChecker : different function parameter");
            }
            if (para1->self_param_->shorthand_self_->mut_ != para1->self_param_->shorthand_self_->mut_ || para1->self_param_->shorthand_self_->quote_ != para2->self_param_->shorthand_self_->quote_) {
              throw Error("ThirdChecker : different function parameter");
            }
          } else if (para2->self_param_ != nullptr) {
            throw Error("ThirdChecker : different function parameter");
          }
          uint32_t size = para1->function_params_.size();
          if (size != para2->function_params_.size()) {
            throw Error("ThirdChecker : different function parameter");
          }
          for (uint32_t i = 0; i < size; ++i) {
            SameTypeCheck(para1->function_params_[i]->type_->type_info_.get(), para2->function_params_[i]->type_->type_info_.get());
          }
        } else {
          auto it = trait_node->items_.find(associated_item->constant_item_->identifier_->val_);
          SameTypeCheck(it->second->type_info_.get(), associated_item->constant_item_->type_info_.get());
        }
      }
    }
  } catch (Error &) { throw; }
}

void ThirdChecker::Visit(ConstantItemNode *node) {}

void ThirdChecker::Visit(AssociatedItemNode *node) {
  try {
    if (node->constant_item_ != nullptr) {
      node->constant_item_->Accept(this);
    } else {
      node->function_->Accept(this);
    }
  } catch (Error &) { throw; }
}

void ThirdChecker::Visit(ItemNode *node) {
  try {
    if (node->function_ != nullptr) {
      node->function_->Accept(this);
    } else if (node->struct_ != nullptr) {
      node->struct_->Accept(this);
    } else if (node->implementation_ != nullptr) {
      node->implementation_->Accept(this);
    } // ignore all other items
  } catch (Error &) { throw; }
}

void ThirdChecker::Visit(PathIdentSegmentNode *node) {}

void ThirdChecker::Visit(IdentifierPatternNode *node) {}

void ThirdChecker::Visit(ReferencePatternNode *node) {}

void ThirdChecker::Visit(PatternWithoutRangeNode *node) {}

void ThirdChecker::Visit(LetStatementNode *node) {
  try {
    node->type_info_ = node->type_->type_info_;
    node->expr_->Accept(this);
    SameTypeCheck(node->type_info_.get(), node->expr_->type_info_.get());
    node->scope_->AddValueName(node->pattern_no_top_alt_->identifier_pattern_->identifier_->val_, node->pattern_no_top_alt_->identifier_pattern_.get(), true);
    node->pattern_no_top_alt_->identifier_pattern_->type_info_ = std::make_shared<Type>(*node->type_->type_info_);
    node->pattern_no_top_alt_->identifier_pattern_->type_info_->is_mut_left_ = node->pattern_no_top_alt_->identifier_pattern_->mut_;
  } catch (Error &) { throw; }
}

void ThirdChecker::Visit(ExpressionStatementNode *node) {
  try {
    if (node->expr_without_block_ != nullptr) {
      node->expr_without_block_->Accept(this);
      node->type_info_ = node->expr_without_block_->type_info_;
    } else {
      node->expr_with_block_->Accept(this);
      node->type_info_ = node->expr_with_block_->type_info_;
    }
  } catch (Error &) { throw; }
}

void ThirdChecker::Visit(StatementNode *node) {
  try {
    if (node->item_ != nullptr) {
      node->item_->Accept(this);
    } else if (node->let_statement_ != nullptr) {
      node->let_statement_->Accept(this);
    } else if (node->expr_statement_ != nullptr) {
      node->expr_statement_->Accept(this);
    }
  } catch (Error &) { throw; }
}

void ThirdChecker::Visit(StatementsNode *node) {
  try {
    for (auto &statement : node->statement_s_) {
      statement->Accept(this);
    }
    if (node->expr_without_block_ != nullptr) {
      node->expr_without_block_->Accept(this);
      node->type_info_ = node->expr_without_block_->type_info_;
      return;
    }
    auto tail_statement = *node->statement_s_.rbegin();
    if (tail_statement->expr_statement_ != nullptr) {
      if (tail_statement->expr_statement_->semicolon_ == false) {
        node->type_info_ = tail_statement->expr_statement_->type_info_;
        return;
      }
      if (tail_statement->expr_statement_->type_info_->type_ == kNeverType) {
        node->type_info_ = tail_statement->expr_statement_->type_info_;
        return;
      }
    }
    node->type_info_ = std::make_shared<Type>();
    node->type_info_->type_ = kUnitType;
  } catch (Error &) { throw; }
}

void ThirdChecker::Visit(StructFieldNode *node) {}

void ThirdChecker::Visit(StructFieldsNode *node) {}

void ThirdChecker::Visit(StructNode *node) {
  try {
    current_Self_.emplace(node);
    for (auto &associated_item : node->impl_) {
      associated_item.second->Accept(this);
    }
    current_Self_.pop();
  } catch (Error &) { throw; }
}

void ThirdChecker::Visit(IdentifierNode *node) {}

void ThirdChecker::Visit(CharLiteralNode *node) {}

void ThirdChecker::Visit(StringLiteralNode *node) {}

void ThirdChecker::Visit(RawStringLiteralNode *node) {}

void ThirdChecker::Visit(CStringLiteralNode *node) {}

void ThirdChecker::Visit(RawCStringLiteralNode *node) {}

void ThirdChecker::Visit(IntegerLiteralNode *node) {}

void ThirdChecker::Visit(TrueNode *node) {}

void ThirdChecker::Visit(FalseNode *node) {}

void ThirdChecker::Visit(SelfLowerNode *node) {}

void ThirdChecker::Visit(SelfUpperNode *node) {}

void ThirdChecker::Visit(UnderscoreExpressionNode *node) {}

void ThirdChecker::Visit(ContinueExpressionNode *node) {
  node->type_info_ = std::make_shared<Type>();
  node->type_info_->type_ = kNeverType;
}

void ThirdChecker::Visit(TraitNode *node) {}

void ThirdChecker::Visit(ReferenceTypeNode *node) {}

void ThirdChecker::Visit(ArrayTypeNode *node) {}

void ThirdChecker::Visit(UnitTypeNode *node) {}

void ThirdChecker::Visit(TypeNoBoundsNode *node) {}
