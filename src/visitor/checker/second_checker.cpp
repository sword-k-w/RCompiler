#include "visitor/checker/second_checker.h"

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
#include "common/error.h"
#include <set>
#include "common/tool_func.h"

void SecondChecker::Visit(CrateNode *node) {
  try {
    for (auto &item : node->items_) {
      GoDown(node, item.get());
    }
  } catch (Error &) { throw; }
}

void SecondChecker::Visit(EnumVariantsNode *node) {
  try {
    for (auto &enum_variant : node->enum_variant_s_) {
      GoDown(node, enum_variant.get());
    }
  } catch (Error &) { throw; }
}

void SecondChecker::Visit(EnumerationNode *node) {
  try {
    if (node->enum_variants_ != nullptr) {
      GoDown(node, node->enum_variants_.get());
    }
  } catch (Error &) { throw; }
}

void SecondChecker::Visit(LiteralExpressionNode *node) {
  try {
    ASTNode *choose = nullptr;
    if (node->char_literal_ != nullptr) {
      choose = node->char_literal_.get();
    } else if (node->string_literal_ != nullptr) {
      choose = node->string_literal_.get();
    } else if (node->c_string_literal_ != nullptr) {
      choose = node->c_string_literal_.get();
    } else if (node->raw_string_literal_ != nullptr) {
      choose = node->raw_string_literal_.get();
    } else if (node->raw_c_string_literal_ != nullptr) {
      choose = node->raw_c_string_literal_.get();
    } else if (node->integer_literal_ != nullptr) {
      choose = node->integer_literal_.get();
    } else if (node->true_ != nullptr) {
      choose = node->true_.get();
    } else {
      choose = node->false_.get();
    }
    assert(choose != nullptr);
    GoDown(node, choose);
    assert(choose->const_value_ != nullptr);
    node->const_value_ = choose->const_value_;
  } catch (Error &) { throw; }
}

void SecondChecker::Visit(ArrayElementsNode *node) {
  try {
    if (node->semicolon_) {
      GoDown(node, node->exprs_[0].get());

      node->exprs_[1]->need_calculate_ = true;
      node->exprs_[1]->Accept(this);

      if (node->need_calculate_) {
        ExpectUsize(node->exprs_[1]->const_value_.get());
        node->const_value_ = std::make_shared<ConstValue>();
        node->const_value_->type_ = kArrayType;
        std::vector<std::shared_ptr<ConstValue>> values(node->exprs_[1]->const_value_->u32_value_, node->exprs_[0]->const_value_);
        node->const_value_->array_value_info_ = std::make_shared<ArrayValueInfo>(values, node->exprs_[1]->const_value_->u32_value_);
      }
    } else {
      std::vector<std::shared_ptr<ConstValue>> type_values;
      for (auto &expr : node->exprs_) {
        GoDown(node, expr.get());
        type_values.emplace_back(expr->const_value_);
      }
      if (node->need_calculate_) {
        node->const_value_ = std::make_shared<ConstValue>();
        node->const_value_->type_ = kArrayType;
        node->const_value_->array_value_info_ = std::make_shared<ArrayValueInfo>(type_values, node->exprs_.size());
      }
    }
  } catch (Error &) { throw; }
}

void SecondChecker::Visit(ArrayExpressionNode *node) {
  try {
    if (node->array_elements_ != nullptr) {
      GoDown(node, node->array_elements_.get());
      if (node->need_calculate_) {
        node->const_value_ = node->array_elements_->const_value_;
      }
    }
  } catch (Error &) { throw; }
}

void SecondChecker::Visit(PathInExpressionNode *node) {
  try {
    GoDown(node, node->path_expr_segment1_.get());
    if (node->path_expr_segment2_ != nullptr) {
      GoDown(node, node->path_expr_segment2_.get());
    }
  } catch (Error &) { throw; }
}

void SecondChecker::Visit(StructExprFieldNode *node) {
  try {
    GoDown(node, node->identifier_.get());
    GoDown(node, node->expr_.get());
    if (node->need_calculate_) {
      node->const_value_ = node->expr_->const_value_;
    }
  } catch (Error &) { throw; }
}

void SecondChecker::Visit(StructExprFieldsNode *node) {
  try {
    std::set<std::string> identifiers;
    for (auto &struct_expr_field : node->struct_expr_field_s_) {
      GoDown(node, struct_expr_field.get());
      if (!identifiers.insert(struct_expr_field->identifier_->val_).second) {
        throw Error("SecondChecker : same identifier in struct expr fields");
      }
    }
  } catch (Error &) { throw; }
}

void SecondChecker::Visit(StructExpressionNode *node) {
  try {
    GoDown(node, node->path_in_expr_.get());
    if (node->struct_expr_fields_) {
      GoDown(node, node->struct_expr_fields_.get());
    }

    if (node->need_calculate_) {
      if (node->path_in_expr_->path_expr_segment2_ != nullptr) {
        throw Error("SecondChecker : const struct but not single path");
      }
      if (node->path_in_expr_->path_expr_segment1_->identifier_ == nullptr) {
        throw Error("SecondChecker : const struct but not identifier");
      }
      ASTNode *target = node->scope_->FindTypeName(node->path_in_expr_->path_expr_segment1_->identifier_->val_);
      if (target == nullptr) {
        throw Error("SecondChecker : const struct but not found identifier");
      }
      auto struct_type = dynamic_cast<StructNode *>(target);
      if (struct_type == nullptr) {
        throw Error("SecondChecker : const struct but identifier is not a struct");
      }
      if (struct_type->field_.size() != node->struct_expr_fields_->struct_expr_field_s_.size()) {
        throw Error("SecondChecker : const struct but identifier count doesn't match");
      }
      node->const_value_ = std::make_shared<ConstValue>();
      node->const_value_->type_ = kStructType;
      node->const_value_->type_name_ = struct_type->identifier_->val_;
      node->const_value_->type_source_ = target;
      node->const_value_->struct_value_info_ = std::make_shared<StructValueInfo>();
      for (auto &struct_expr_field : node->struct_expr_fields_->struct_expr_field_s_) {
        auto it = struct_type->field_.find(struct_expr_field->identifier_->val_);
        if (it == struct_type->field_.end()) {
          throw Error("SecondChecker : const struct but the identifier doesn't belong to the struct");
        }
        SameTypeCheck(it->second.get(), struct_expr_field->const_value_.get());
        node->const_value_->struct_value_info_->variant_[it->first] = struct_expr_field->const_value_;
      }
    }
  } catch (Error &) { throw; }
}

void SecondChecker::Visit(ExpressionWithoutBlockNode *node) {
  try {
    GoDown(node, node->expr_.get());
    if (node->need_calculate_) {
      node->const_value_ = node->expr_->const_value_;
    }
  } catch (Error &) { throw; }
}

void SecondChecker::Visit(BlockExpressionNode *node) {
  try {
    assert(node->need_calculate_ == false);
    if (node->statements_ != nullptr) {
      GoDown(node, node->statements_.get());
    }
  } catch (Error &) { throw; }
}

void SecondChecker::Visit(InfiniteLoopExpressionNode *node) {
  try {
    if (node->block_expr_ != nullptr) {
      GoDown(node, node->block_expr_.get());
    }
  } catch (Error &) { throw; }
}

void SecondChecker::Visit(ConditionsNode *node) {
  try {
    assert(node->need_calculate_ == false);
    GoDown(node, node->expr_.get());
  } catch (Error &) { throw; }
}

void SecondChecker::Visit(PredicateLoopExpressionNode *node) {
  try {
    GoDown(node, node->conditions_.get());
    GoDown(node, node->block_expr_.get());
  } catch (Error &) { throw; }
}

void SecondChecker::Visit(LoopExpressionNode *node) {
  try {
    assert(node->need_calculate_ == false);
    if (node->infinite_loop_expr_ != nullptr) {
      GoDown(node, node->infinite_loop_expr_.get());
    } else {
      GoDown(node, node->predicate_loop_expr_.get());
    }
  } catch (Error &) { throw; }
}

void SecondChecker::Visit(IfExpressionNode *node) {
  try {
    assert(node->need_calculate_ == false);;
    GoDown(node, node->conditions_.get());
    GoDown(node, node->block_expr1_.get());
    if (node->block_expr2_ != nullptr) {
      GoDown(node, node->block_expr2_.get());
    } else if (node->if_expr_ != nullptr){
      GoDown(node, node->if_expr_.get());
    }
  } catch (Error &) { throw; }
}

void SecondChecker::Visit(ExpressionWithBlockNode *node) {
  try {
    assert(node->need_calculate_ == false);
    if (node->block_expr_ != nullptr) {
      GoDown(node, node->block_expr_.get());
    } else if (node->loop_expr_ != nullptr) {
      GoDown(node, node->loop_expr_.get());
    } else {
      GoDown(node, node->if_expr_.get());
    }
  } catch (Error &) { throw; }
}

void SecondChecker::Visit(CallParamsNode *node) {
  try {
    for (auto &expr : node->exprs_) {
      GoDown(node, expr.get());
    }
  } catch (Error &) { throw; }
}

void SecondChecker::Visit(ExpressionNode *node) {
  try {
    if (node->type_ == kLiteralExpr) {
      GoDown(node, node->literal_expr_.get());
      if (node->need_calculate_) {
        node->const_value_ = node->literal_expr_->const_value_;
      }
    } else if (node->type_ == kPathExpr) {
      GoDown(node, node->path_expr_.get());
      if (node->need_calculate_) {
        ASTNode *target = nullptr;
        if (node->path_expr_->path_expr_segment2_ != nullptr) {
          ASTNode *tmp = node->scope_->FindTypeName(node->path_expr_->path_expr_segment1_->identifier_->val_);
          auto *struct_node = dynamic_cast<StructNode *>(tmp);
          if (struct_node != nullptr) {
            auto it = struct_node->impl_.find(node->path_expr_->path_expr_segment2_->identifier_->val_);
            if (it == struct_node->impl_.end()) {
              throw Error("SecondChecker : cannot resolve path expr");
            }
            target = it->second;
            auto const_item = dynamic_cast<ConstantItemNode *>(target);
            if (const_item == nullptr) {
              throw Error("SecondChecker : path expr but function");
            }
            node->const_value_ = const_item->const_value_;
          } else {
            auto enum_node = dynamic_cast<EnumerationNode *>(tmp);
            if (enum_node != nullptr) {
              if (enum_node->enum_.find(node->path_expr_->path_expr_segment2_->identifier_->val_) == enum_node->enum_.end()) {
                throw Error("SecondChecker : the variant doesn't exist in enumeration");
              }
              node->const_value_ = std::make_shared<ConstValue>();
              node->const_value_->type_source_ = tmp;
              node->const_value_->type_ = kEnumType;
              node->const_value_->type_name_ = enum_node->identifier_->val_;
              node->const_value_->str_value_ = node->path_expr_->path_expr_segment2_->identifier_->val_;
            } else {
              auto trait_node = dynamic_cast<TraitNode *>(tmp);
              assert(trait_node != nullptr);
              auto it = trait_node->items_.find(node->path_expr_->path_expr_segment2_->identifier_->val_);
              if (it == trait_node->items_.end()) {
                throw Error("SecondChecker : the identifier doesn't exist in trait");
              }
              auto const_item = dynamic_cast<ConstantItemNode *>(it->second);
              if (const_item == nullptr) {
                throw Error("SecondChecker : path expr but function");
              }
              if (const_item->expr_ == nullptr) {
                throw Error("SecondChecker : want trait const value but no default value");
              }
              node->const_value_ = const_item->const_value_;
            }
          }
        } else {
          target = node->scope_->FindValueName(node->path_expr_->path_expr_segment1_->identifier_->val_);
          auto *struct_node = dynamic_cast<StructNode *>(target);
          if (struct_node != nullptr) {
            if (struct_node->struct_fields_ != nullptr) {
              throw Error("SecondChecker : expect unit-like struct");
            }
            node->const_value_ = std::make_shared<ConstValue>();
            node->const_value_->type_ = kStructType;
            node->const_value_->type_source_ = target;
            node->const_value_->type_name_ = struct_node->identifier_->val_;
            node->const_value_->struct_value_info_ = std::make_shared<StructValueInfo>();
          } else {
            auto *const_item = dynamic_cast<ConstantItemNode *>(target);
            if (const_item != nullptr) {
              node->const_value_ = const_item->const_value_;
            } else {
              throw Error("SecondChecker : unexpected path");
            }
          }
        }
      }
    } else if (node->type_ == kArrayExpr) {
      GoDown(node, node->array_expr_.get());
      if (node->need_calculate_) {
        node->const_value_ = node->array_expr_->const_value_;
      }
    } else if (node->type_ == kStructExpr) {
      GoDown(node, node->struct_expr_.get());
      if (node->need_calculate_) {
        node->const_value_ = node->struct_expr_->const_value_;
      }
    } else if (node->type_ == kContinueExpr) {
      if (node->need_calculate_) {
        throw Error("SecondChecker : const expr but continue");
      }
      GoDown(node, node->continue_expr_.get());
    } else if (node->type_ == kUnderscoreExpr) {
      if (node->need_calculate_) {
        throw Error("SecondChecker : const expr but underscore");
      }
      GoDown(node, node->underscore_expr_.get());
    } else if (node->type_ == kBorrowExpr) {
      if (node->need_calculate_) {
        throw Error("SecondChecker : const expr but borrow");
      }
      GoDown(node, node->expr1_.get());
    } else if (node->type_ == kDereferenceExpr) {
      if (node->need_calculate_) {
        throw Error("SecondChecker : const expr but dereference");
      }
      GoDown(node, node->expr1_.get());
    } else if (node->type_ == kNegationExpr) {
      GoDown(node, node->expr1_.get());
      if (node->need_calculate_) {
        node->const_value_ = std::make_shared<ConstValue>(*node->expr1_->const_value_);
        if (node->const_value_->type_ != kLeafType || node->const_value_->type_name_ == "str" || node->const_value_->type_name_ == "char" || node->const_value_->type_name_ == "String") {
          throw Error("SecondChecker : negation with unexpected type");
        }
        if (node->op_ == "-") {
          if (IsSignedIntegerType(node->const_value_->type_name_)) {
            node->const_value_->u32_value_ = -node->const_value_->u32_value_;
          } else {
            throw Error("SecondChecker : negation with unexpected type");
          }
        } else {
          if (node->const_value_->type_name_ == "bool") {
            node->const_value_->u32_value_ = !node->const_value_->u32_value_;
          } else {
            node->const_value_->u32_value_ = ~node->const_value_->u32_value_;
          }
        }
      }
    } else if (node->type_ == kArithmeticOrLogicExpr) {
      GoDown(node, node->expr1_.get());
      GoDown(node, node->expr2_.get());
      if (node->need_calculate_) {
        if (node->expr1_->const_value_->type_ != kLeafType || node->expr2_->const_value_->type_ != kLeafType) {
          throw Error("SecondChecker : arithmetic or logic expr but not leaf type");
        }
        if (node->expr1_->const_value_->type_name_ == "char" || node->expr2_->const_value_->type_name_ == "char") {
          throw Error("SecondChecker : arithmetic or logic expr but char");
        }
        node->const_value_ = std::make_shared<ConstValue>(*node->expr1_->const_value_);
        if (node->op_ == "<<" || node->op_ == ">>") {
          if (node->expr1_->const_value_->type_name_ == "bool" || node->expr2_->const_value_->type_name_ == "bool") {
            throw Error("SecondChecker : shift but str or bool");
          }
          if (IsSignedIntegerType(node->expr2_->const_value_->type_name_)) {
            int32_t tmp = static_cast<int32_t>(node->expr2_->const_value_->u32_value_);
            if (tmp < 0) {
              throw Error("SecondChecker : const negative shift");
            }
            if (IsSignedIntegerType(node->const_value_->type_name_)) {
              if (node->op_ == "<<") {
                node->const_value_->u32_value_ = static_cast<int32_t>(node->const_value_->u32_value_) << tmp;
              } else {
                node->const_value_->u32_value_ = static_cast<int32_t>(node->const_value_->u32_value_) >> tmp;
              }
            } else {
              if (node->op_ == "<<") {
                node->const_value_->u32_value_ <<= tmp;
              } else {
                node->const_value_->u32_value_ >>= tmp;
              }
            }
          } else {
            uint32_t tmp = node->expr2_->const_value_->u32_value_;
            if (IsSignedIntegerType(node->const_value_->type_name_)) {
              if (node->op_ == "<<") {
                node->const_value_->u32_value_ = static_cast<int32_t>(node->const_value_->u32_value_) << tmp;
              } else {
                node->const_value_->u32_value_ = static_cast<int32_t>(node->const_value_->u32_value_) >> tmp;
              }
            } else {
              if (node->op_ == "<<") {
                node->const_value_->u32_value_ <<= tmp;
              } else {
                node->const_value_->u32_value_ >>= tmp;
              }
            }
          }
          return;
        }
        node->const_value_->type_name_ = MergeLeafType(node->expr1_->const_value_->type_name_, node->expr2_->const_value_->type_name_);
        if (node->expr1_->const_value_->type_name_ == "bool") {
          if (node->op_ == "&") {
            node->const_value_->u32_value_ &= node->expr2_->const_value_->u32_value_;
          } else if (node->op_ == "|") {
            node->const_value_->u32_value_ |= node->expr2_->const_value_->u32_value_;
          } else if (node->op_ == "^") {
            node->const_value_->u32_value_ ^= node->expr2_->const_value_->u32_value_;
          } else {
            throw Error("SecondChecker : arithmetic or logic expr but bool with unexpected operation");
          }
        } else {
          if (node->op_ == "+") {
            node->const_value_->u32_value_ += node->expr2_->const_value_->u32_value_;
          } else if (node->op_ == "-") {
            node->const_value_->u32_value_ -= node->expr2_->const_value_->u32_value_;
          } else if (node->op_ == "*") {
            node->const_value_->u32_value_ *= node->expr2_->const_value_->u32_value_;
          } else if (node->op_ == "/") {
            if (node->expr2_->const_value_->u32_value_ == 0) {
              throw Error("SecondChecker : const but divide 0");
            }
            if (IsSignedIntegerType(node->const_value_->type_name_)) {
              int32_t tmp = static_cast<int32_t>(node->const_value_->u32_value_);
              tmp /= static_cast<int32_t>(node->expr2_->const_value_->u32_value_);
              node->const_value_->u32_value_ = tmp;
            } else {
              node->const_value_->u32_value_ /= node->expr2_->const_value_->u32_value_;
            }
          } else if (node->op_ == "%") {
            if (node->expr2_->const_value_->u32_value_ == 0) {
              throw Error("SecondChecker : const but divide 0");
            }
            if (IsSignedIntegerType(node->const_value_->type_name_)) {
              int32_t tmp = static_cast<int32_t>(node->const_value_->u32_value_);
              tmp %= static_cast<int32_t>(node->expr2_->const_value_->u32_value_);
              node->const_value_->u32_value_ = tmp;
            } else {
              node->const_value_->u32_value_ %= node->expr2_->const_value_->u32_value_;
            }
          } else if (node->op_ == "&") {
            node->const_value_->u32_value_ &= node->expr2_->const_value_->u32_value_;
          } else if (node->op_ == "|") {
            node->const_value_->u32_value_ |= node->expr2_->const_value_->u32_value_;
          } else {
            node->const_value_->u32_value_ ^= node->expr2_->const_value_->u32_value_;
          }
        }
      }
    } else if (node->type_ == kComparisonExpr) {
      GoDown(node, node->expr1_.get());
      GoDown(node, node->expr2_.get());
      if (node->need_calculate_) {
        if (node->expr1_->const_value_->type_ != kLeafType || node->expr2_->const_value_->type_ != kLeafType) {
          throw Error("SecondChecker : comparison expr but not leaf type");
        }
        MergeLeafType(node->expr1_->const_value_->type_name_, node->expr2_->const_value_->type_name_);
        node->const_value_ = std::make_shared<ConstValue>();
        node->const_value_->type_ = kLeafType;
        node->const_value_->type_name_ = "bool";
        if (node->op_ == "==") {
          node->const_value_->u32_value_ = node->expr1_->const_value_->u32_value_ == node->expr2_->const_value_->u32_value_;
        } else if (node->op_ == "!=") {
          node->const_value_->u32_value_ = node->expr1_->const_value_->u32_value_ != node->expr2_->const_value_->u32_value_;
        } else if (node->op_ == ">") {
          node->const_value_->u32_value_ = node->expr1_->const_value_->u32_value_ > node->expr2_->const_value_->u32_value_;
        } else if (node->op_ == "<") {
          node->const_value_->u32_value_ = node->expr1_->const_value_->u32_value_ < node->expr2_->const_value_->u32_value_;
        } else if (node->op_ == ">=") {
          node->const_value_->u32_value_ = node->expr1_->const_value_->u32_value_ >= node->expr2_->const_value_->u32_value_;
        } else {
          node->const_value_->u32_value_ = node->expr1_->const_value_->u32_value_ <= node->expr2_->const_value_->u32_value_;
        }
      }
    } else if (node->type_ == kLazyBooleanExpr) {
      GoDown(node, node->expr1_.get());
      GoDown(node, node->expr2_.get());
      if (node->need_calculate_) {
        if (node->expr1_->const_value_->type_ != kLeafType || node->expr2_->const_value_->type_ != kLeafType) {
          throw Error("SecondChecker : lazy boolean expr but not leaf type");
        }
        if (node->expr1_->const_value_->type_name_ != "bool" || node->expr2_->const_value_->type_name_ != "bool") {
          throw Error("SecondChecker : lazy boolean expr but not bool");
        }
        node->const_value_ = std::make_shared<ConstValue>();
        node->const_value_->type_ = kLeafType;
        node->const_value_->type_name_ = "bool";
        if (node->op_ == "||") {
          node->const_value_->u32_value_ = node->expr1_->const_value_->u32_value_ | node->expr2_->const_value_->u32_value_;
        } else {
          node->const_value_->u32_value_ = node->expr1_->const_value_->u32_value_ & node->expr2_->const_value_->u32_value_;
        }
      }
    } else if (node->type_ == kTypeCastExpr) {
      GoDown(node, node->expr1_.get());
      GoDown(node, node->type_no_bounds_.get());
      if (node->need_calculate_) {
        node->const_value_ = std::make_shared<ConstValue>(*node->expr1_->const_value_);
        TypeCast(node->type_no_bounds_->type_info_.get(), node->const_value_.get());
      }
    } else if (node->type_ == kAssignmentExpr) {
      if (node->need_calculate_) {
        throw Error("SecondChecker : const expr but assignment");
      }
      GoDown(node, node->expr1_.get());
      GoDown(node, node->expr2_.get());
    } else if (node->type_ == kCompoundAssignmentExpr) {
      if (node->need_calculate_) {
        throw Error("SecondChecker : const expr but compound assignment");
      }
      GoDown(node, node->expr1_.get());
      GoDown(node, node->expr2_.get());
    } else if (node->type_ == kGroupedExpr) {
      GoDown(node, node->expr1_.get());
      if (node->need_calculate_) {
        node->const_value_ = node->expr1_->const_value_;
      }
    } else if (node->type_ == kIndexExpr) {
      GoDown(node, node->expr1_.get());
      node->expr2_->need_calculate_ = node->need_calculate_;
      node->expr2_->Accept(this);
      if (node->need_calculate_) {
        if (node->expr1_->const_value_->type_ != kArrayType) {
          throw Error("SecondChecker : index expr but not array");
        }
        if (!ExpectUsize(node->expr2_->const_value_.get())) {
          throw Error("SecondChecker : index expr but index is not usize");
        }
        if (node->expr2_->const_value_->u32_value_ >= node->expr1_->const_value_->array_value_info_->length_) {
          throw Error("SecondChecker : const index expr but index exceeds length");
        }
        node->const_value_ = node->expr1_->const_value_->array_value_info_->values_[node->expr2_->const_value_->u32_value_];
      }
    } else if (node->type_ == kCallExpr) {
      if (node->need_calculate_) {
        throw Error("SecondChecker : const expr but call");
      }
      GoDown(node, node->expr1_.get());
      if (node->call_params_ != nullptr) {
        GoDown(node, node->call_params_.get());
      }
    } else if (node->type_ == kMethodCallExpr) {
      if (node->need_calculate_) {
        throw Error("SecondChecker : const expr but method call");
      }
      GoDown(node, node->expr1_.get());
      GoDown(node, node->path_expr_segment_.get());
      if (node->call_params_ != nullptr) {
        GoDown(node, node->call_params_.get());
      }
    } else if (node->type_ == kFieldExpr) {
      GoDown(node, node->expr1_.get());
      GoDown(node, node->identifier_.get());
      if (node->need_calculate_) {
        if (node->expr1_->const_value_->type_ != kStructType) {
          throw Error("SecondChecker : field expr but not struct");
        }
        auto it = node->expr1_->const_value_->struct_value_info_->variant_.find(node->identifier_->val_);
        if (it == node->expr1_->const_value_->struct_value_info_->variant_.end()) {
          throw Error("SecondChecker : field expr but can't find field");
        }
        node->const_value_ = it->second;
      }
    } else if (node->type_ == kBreakExpr) {
      if (node->need_calculate_) {
        throw Error("SecondChecker : const expr but break");
      }
      if (node->expr1_ != nullptr) {
        GoDown(node, node->expr1_.get());
      }
    } else if (node->type_ == kReturnExpr) {
      if (node->need_calculate_) {
        throw Error("SecondChecker : const expr but return");
      }
      if (node->expr1_ != nullptr) {
        GoDown(node, node->expr1_.get());
      }
    } else {
      if (node->need_calculate_) {
        throw Error("SecondChecker : const expr but with block");
      }
      GoDown(node, node->expr_with_block_.get());
    }
  } catch (Error &) { throw; }
}

void SecondChecker::Visit(ShorthandSelfNode *node) {}

void SecondChecker::Visit(SelfParamNode *node) {
  try {
    GoDown(node, node->shorthand_self_.get());
  } catch (Error &) { throw; }
}

void SecondChecker::Visit(FunctionParamNode *node) {
  try {
    GoDown(node, node->pattern_no_top_alt_.get());
    GoDown(node, node->type_.get());
  } catch (Error &) { throw; }
}

void SecondChecker::Visit(FunctionParametersNode *node) {
  try {
    if (node->self_param_ != nullptr) {
      GoDown(node, node->self_param_.get());
    }
    for (auto &function_param : node->function_params_) {
      GoDown(node, function_param.get());
    }
  } catch (Error &) { throw; }
}

void SecondChecker::Visit(FunctionReturnTypeNode *node) {
  try {
    GoDown(node, node->type_.get());
    node->type_info_ = node->type_->type_info_;
  } catch (Error &) { throw; }
}

void SecondChecker::Visit(FunctionNode *node) {
  try {
    assert(!node->need_calculate_);
    GoDown(node, node->identifier_.get());
    if (node->function_parameters_ != nullptr) {
      GoDown(node, node->function_parameters_.get());
    }
    if (node->function_return_type_ != nullptr) {
      GoDown(node, node->function_return_type_.get());
      node->type_info_ = node->function_return_type_->type_info_;
    } else {
      node->type_info_ = std::make_shared<Type>();
      node->type_info_->type_ = kUnitType;
    }
    if (node->block_expr_ != nullptr) {
      GoDown(node, node->block_expr_.get());
    }
  } catch (Error &) { throw; }
}

void SecondChecker::Visit(ImplementationNode *node) {}

void SecondChecker::Visit(ConstantItemNode *node) {
  try {
    GoDown(node, node->type_.get());
    if (node->expr_ == nullptr) {
      if (!node->in_trait_) {
        throw Error("SecondChecker : a const item with no expr but not in trait");
      }
    } else {
      node->expr_->need_calculate_ = true;
      node->expr_->Accept(this);
      node->const_value_ = node->expr_->const_value_;
      SameTypeCheck(node->type_->type_info_.get(), node->const_value_.get());
    }
  } catch (Error &) { throw; }
}

void SecondChecker::Visit(AssociatedItemNode *node) {
  try {
    if (node->function_ != nullptr) {
      GoDown(node, node->function_.get());
    } else {
      GoDown(node, node->constant_item_.get());
    }
  } catch (Error &) { throw; }
}

void SecondChecker::Visit(ItemNode *node) {
  try {
    if (node->function_ != nullptr) {
      GoDown(node, node->function_.get());
    } else if (node->struct_ != nullptr) {
      GoDown(node, node->struct_.get());
    } else if (node->enumeration_ != nullptr) {
      GoDown(node, node->enumeration_.get());
    } else if (node->constant_item_ != nullptr) {
      GoDown(node, node->constant_item_.get());
    } else if (node->trait_ != nullptr) {
      GoDown(node, node->trait_.get());
    } else {
      // ignore implementation
    }
  } catch (Error &) { throw; }
}

void SecondChecker::Visit(PathIdentSegmentNode *node) {
  try {
    if (node->identifier_ != nullptr) {
      GoDown(node, node->identifier_.get());
    } else if (node->self_upper_ != nullptr) {
      GoDown(node, node->self_upper_.get());
    } else {
      GoDown(node, node->self_lower_.get());
    }
  } catch (Error &) { throw; }
}

void SecondChecker::Visit(IdentifierPatternNode *node) {
  try {
    GoDown(node, node->identifier_.get());
  } catch (Error &) { throw; }
}

void SecondChecker::Visit(ReferencePatternNode *node) {
  try {
    GoDown(node, node->pattern_without_range_.get());
  } catch (Error &) { throw; }
}

void SecondChecker::Visit(PatternWithoutRangeNode *node) {
  try {
    if (node->identifier_pattern_ != nullptr) {
      GoDown(node, node->identifier_pattern_.get());
    } else if (node->wildcard_pattern_ != nullptr) {
      GoDown(node, node->wildcard_pattern_.get());
    } else {
      GoDown(node, node->reference_pattern_.get());
    }
  } catch (Error &) { throw; }
}

void SecondChecker::Visit(LetStatementNode *node) {
  try {
    GoDown(node, node->pattern_no_top_alt_.get());
    GoDown(node, node->type_.get());
    node->expr_->need_calculate_ = false;
    node->expr_->Accept(this);
  } catch (Error &) { throw; }
}

void SecondChecker::Visit(ExpressionStatementNode *node) {
  try {
    node->expr_->need_calculate_ = false;
    node->expr_->Accept(this);
  } catch (Error &) { throw; }
}

void SecondChecker::Visit(StatementNode *node) {
  try {
    if (node->item_ != nullptr) {
      GoDown(node, node->item_.get());
    } else if (node->let_statement_ != nullptr) {
      GoDown(node, node->let_statement_.get());
    } else if (node->expr_statement_ != nullptr) {
      GoDown(node, node->expr_statement_.get());
    }
  } catch (Error &) { throw; }
}

void SecondChecker::Visit(StatementsNode *node) {
  try {
    for (auto &statement : node->statement_s_) {
      GoDown(node, statement.get());
    }
    if (node->expr_without_block_ != nullptr) {
      GoDown(node, node->expr_without_block_.get());
    }
  } catch (Error &) { throw; }
}

void SecondChecker::Visit(StructFieldNode *node) {
  try {
    GoDown(node, node->identifier_.get());
    GoDown(node, node->type_.get());
  } catch (Error &) { throw; }
}

void SecondChecker::Visit(StructFieldsNode *node) {
  try {
    for (auto &struct_field : node->struct_field_s_) {
      GoDown(node, struct_field.get());
    }
  } catch (Error &) { throw; }
}

void SecondChecker::Visit(StructNode *node) {
  try {
    node->type_info_ = std::make_shared<Type>();
    node->type_info_->type_ = kStructType;
    node->type_info_->source_ = node;
    GoDown(node, node->identifier_.get());
    if (node->struct_fields_ != nullptr) {
      GoDown(node, node->struct_fields_.get());
      for (auto &struct_field : node->struct_fields_->struct_field_s_) {
        if (!node->field_.emplace(struct_field->identifier_->val_, struct_field->type_->type_info_).second) {
          throw Error("SecondChecker : struct definition but repeated identifier");
        }
      }
    }
    current_Self_.emplace(node);
    for (auto &associated_item : node->impl_) {
      GoDown(node, associated_item.second);
    }
    current_Self_.pop();
  } catch (Error &) { throw; }
}

void SecondChecker::Visit(IdentifierNode *node) {}

void SecondChecker::Visit(CharLiteralNode *node) {
  try {
    node->const_value_ = std::make_shared<ConstValue>();
    node->const_value_->type_ = kLeafType;
    node->const_value_->type_name_ = "char";
    if (node->val_[1] != '\\') {
      node->const_value_->u32_value_ = node->val_[1];
    } else if (node->val_[2] == '\'') {
      node->const_value_->u32_value_ = '\'';
    } else if (node->val_[2] == '\"') {
      node->const_value_->u32_value_ = '\"';
    } else if (node->val_[2] == 'n') {
      node->const_value_->u32_value_ = '\n';
    } else if (node->val_[2] == 'r') {
      node->const_value_->u32_value_ = '\r';
    } else if (node->val_[2] == 't') {
      node->const_value_->u32_value_ = '\t';
    } else if (node->val_[2] == '\\') {
      node->const_value_->u32_value_ = '\\';
    } else if (node->val_[2] == '\0') {
      node->const_value_->u32_value_ = '\0';
    } else {
      node->const_value_->u32_value_ = ToDigitalValue(node->val_[3]) * 16 + ToDigitalValue(node->val_[4]);
    }
  } catch (Error &) { throw; }
}

void SecondChecker::Visit(StringLiteralNode *node) {
  try {
    node->const_value_ = std::make_shared<ConstValue>();
    node->const_value_->type_ = kPointerType;
    node->const_value_->pointer_info_ = std::make_shared<ConstValue>();
    node->const_value_->pointer_info_->type_ = kLeafType;
    node->const_value_->pointer_info_->type_name_ = "str";
    auto it = node->val_.begin();
    ++it;
    while (*it != '\"') {
      if (*it != '\\') {
        node->const_value_->pointer_info_->str_value_.push_back(*it++);
      } else {
        ++it;
        if (*it == '\'') {
          node->const_value_->pointer_info_->str_value_.push_back('\'');
        } else if (*it == '\"') {
          node->const_value_->pointer_info_->str_value_.push_back('\"');
        } else if (*it == 'n') {
          node->const_value_->pointer_info_->str_value_.push_back('\n');
        } else if (*it == 'r') {
          node->const_value_->pointer_info_->str_value_.push_back('\r');
        } else if (*it == 't') {
          node->const_value_->pointer_info_->str_value_.push_back('\t');
        } else if (*it == '\\') {
          node->const_value_->pointer_info_->str_value_.push_back('\\');
        } else if (*it == '0') {
          node->const_value_->pointer_info_->str_value_.push_back('\0');
        } else if (*it == 'x') {
          int32_t x = ToDigitalValue(*++it);
          int32_t y = ToDigitalValue(*++it);
          node->const_value_->pointer_info_->str_value_.push_back(static_cast<char>(x * 16 + y));
        }
        ++it;
      }
    }
  } catch (Error &) { throw; }
}

void SecondChecker::Visit(RawStringLiteralNode *node) {
  try {
    node->const_value_ = std::make_shared<ConstValue>();
    node->const_value_->type_ = kPointerType;
    node->const_value_->pointer_info_ = std::make_shared<ConstValue>();
    node->const_value_->pointer_info_->type_ = kLeafType;
    node->const_value_->pointer_info_->type_name_ = "str";
    auto it = node->val_.begin();
    while (*it != '\"') { ++it; }
    ++it;
    while (*it != '\"') {
      node->const_value_->pointer_info_->str_value_.push_back(*it++);
    }
  } catch (Error &) { throw; }
}

void SecondChecker::Visit(CStringLiteralNode *node) {
  try {
    node->const_value_ = std::make_shared<ConstValue>();
    node->const_value_->type_ = kPointerType;
    node->const_value_->pointer_info_ = std::make_shared<ConstValue>();
    node->const_value_->pointer_info_->type_ = kLeafType;
    node->const_value_->pointer_info_->type_name_ = "str";
    auto it = node->val_.begin();
    ++it;
    while (*it != '\"') {
      if (*it != '\\') {
        node->const_value_->pointer_info_->str_value_.push_back(*it++);
      } else {
        ++it;
        if (*it == '\'') {
          node->const_value_->pointer_info_->str_value_.push_back('\'');
        } else if (*it == '\"') {
          node->const_value_->pointer_info_->str_value_.push_back('\"');
        } else if (*it == 'n') {
          node->const_value_->pointer_info_->str_value_.push_back('\n');
        } else if (*it == 'r') {
          node->const_value_->pointer_info_->str_value_.push_back('\r');
        } else if (*it == 't') {
          node->const_value_->pointer_info_->str_value_.push_back('\t');
        } else if (*it == '\\') {
          node->const_value_->pointer_info_->str_value_.push_back('\\');
        } else if (*it == '0') {
          node->const_value_->pointer_info_->str_value_.push_back('\0');
        } else if (*it == 'x') {
          int32_t x = ToDigitalValue(*++it);
          int32_t y = ToDigitalValue(*++it);
          node->const_value_->pointer_info_->str_value_.push_back(static_cast<char>(x * 16 + y));
        }
        ++it;
      }
    }
  } catch (Error &) { throw; }
}

void SecondChecker::Visit(RawCStringLiteralNode *node) {
  try {
    node->const_value_ = std::make_shared<ConstValue>();
    node->const_value_->type_ = kPointerType;
    node->const_value_->pointer_info_ = std::make_shared<ConstValue>();
    node->const_value_->pointer_info_->type_ = kLeafType;
    node->const_value_->pointer_info_->type_name_ = "str";
    auto it = node->val_.begin();
    while (*it != '\"') { ++it; }
    ++it;
    while (*it != '\"') {
      node->const_value_->pointer_info_->str_value_.push_back(*it++);
    }
  } catch (Error &) { throw; }
}

void SecondChecker::Visit(IntegerLiteralNode *node) {
  try {
    node->const_value_ = std::make_shared<ConstValue>();
    node->const_value_->type_ = kLeafType;
    if (node->val_.size() == 1) {
      node->const_value_->type_name_ = "$";
      node->const_value_->u32_value_ = ToDigitalValue(*node->val_.begin());
    } else {
      uint64_t base = 10;
      auto it = node->val_.begin();
      if (*it == 0) {
        if (*std::next(it) == 'b') {
          base = 2;
          ++it;
          ++it;
        } else if (*std::next(it) == 'o') {
          base = 8;
          ++it;
          ++it;
        } else if (*std::next(it) == 'x') {
          base = 16;
          ++it;
          ++it;
        }
      }
      uint64_t val = 0;
      while (it != node->val_.end()) {
        if (base == 2) {
          if (*it != '0' && *it != '1') {
            break;
          }
        } else if (base == 8) {
          if (*it < '0' || *it > '7') {
            break;
          }
        } else if (base == 10) {
          if (*it < '0' || *it > '9') {
            break;
          }
        } else {
          if (!(*it >= '0' && *it <= '9' || *it >= 'A' && *it <= 'F')) {
            break;
          }
        }
        val = val * base + ToDigitalValue(*it++);
        if (val >> 32) {
          throw Error("SecondChecker : too large integer literal");
        }
      }
      if (it != node->val_.end()) {
        while (it != node->val_.end()) {
          node->const_value_->type_name_.push_back(*it++);
        }
      } else {
        node->const_value_->type_name_ = "$";
      }
      if (!IsIntegerType(node->const_value_->type_name_)) {
        throw Error("SecondChecker : integer literal but the suffix isn't integer type");
      }
      if ((node->const_value_->type_name_ == "i32" || node->const_value_->type_name_ == "isize")
        && val > INT_MAX) {
        throw Error("SecondChecker : too large integer literal");
      }
      node->const_value_->u32_value_ = static_cast<uint32_t>(val);
    }
  } catch (Error &) { throw; }
}

void SecondChecker::Visit(TrueNode *node) {
  try {
    node->const_value_ = std::make_shared<ConstValue>();
    node->const_value_->type_ = kLeafType;
    node->const_value_->type_name_ = "bool";
    node->const_value_->u32_value_ = 1;
  } catch (Error &) { throw; }
}

void SecondChecker::Visit(FalseNode *node) {
  try {
    node->const_value_ = std::make_shared<ConstValue>();
    node->const_value_->type_ = kLeafType;
    node->const_value_->type_name_ = "bool";
    node->const_value_->u32_value_ = 0;
  } catch (Error &) { throw; }
}

void SecondChecker::Visit(SelfLowerNode *node) {}

void SecondChecker::Visit(SelfUpperNode *node) {}

void SecondChecker::Visit(UnderscoreExpressionNode *node) {}

void SecondChecker::Visit(ContinueExpressionNode *node) {}

void SecondChecker::Visit(TraitNode *node) {
  try {
    GoDown(node, node->identifier_.get());
    for (auto &associated_item : node->asscociated_items_) {
      GoDown(node, associated_item.get());
    }
  } catch (Error &) { throw; }
}

void SecondChecker::Visit(ReferenceTypeNode *node) {
  try {
    GoDown(node, node->type_no_bounds_.get());
    node->type_info_ = std::make_shared<Type>();
    node->type_info_->type_ = kPointerType;
    node->type_info_->pointer_type_ = std::make_shared<Type>(*node->type_no_bounds_->type_info_);
    node->type_info_->pointer_type_->is_mut_left_ = node->mut_;
  } catch (Error &) { throw; }
}

void SecondChecker::Visit(ArrayTypeNode *node) {
  try {
    GoDown(node, node->type_.get());
    node->expr_->need_calculate_ = true;
    node->expr_->Accept(this);
    node->type_info_ = std::make_shared<Type>();
    node->type_info_->type_ = kArrayType;
    node->type_info_->array_type_info_ = std::make_pair(node->type_->type_info_, node->expr_->const_value_->u32_value_);
  } catch (Error &) { throw; }
}

void SecondChecker::Visit(UnitTypeNode *node) {
  node->type_info_ = std::make_shared<Type>();
  node->type_info_->type_ = kUnitType;
}

void SecondChecker::Visit(TypeNoBoundsNode *node) {
  try {
    if (node->type_path_ != nullptr) {
      GoDown(node, node->type_path_.get());
      if (node->type_path_->self_upper_ != nullptr) {
        if (current_Self_.empty()) {
          throw Error("SecondChecker : no struct but Self");
        }
        node->type_info_ = std::make_shared<Type>();
        node->type_info_->type_ = kStructType;
        node->type_info_->type_name_ = current_Self_.top()->identifier_->val_;
        node->type_info_->source_ = current_Self_.top();
        return;
      }
      if (node->type_path_->self_lower_ != nullptr) {
        throw Error("SecondChecker : type path but self");
      }
      node->type_info_ = std::make_shared<Type>();
      bool flag = false;
      for (uint32_t i = 0; i < 8; ++i) {
        if (node->type_path_->identifier_->val_ == kBuiltinType[i]) {
          node->type_info_->type_ = kLeafType;
          node->type_info_->type_name_ = kBuiltinType[i];
          flag = true;
          break;
        }
      }
      if (!flag) {
        auto target = node->scope_->FindTypeName(node->type_path_->identifier_->val_);
        auto struct_node = dynamic_cast<StructNode *>(target);
        if (struct_node != nullptr) {
          node->type_info_->type_ = kStructType;
          node->type_info_->type_name_ = node->type_path_->identifier_->val_;
          node->type_info_->source_ = target;
        } else {
          auto enum_node = dynamic_cast<EnumerationNode *>(target);
          node->type_info_->type_ = kEnumType;
          node->type_info_->type_name_ = node->type_path_->identifier_->val_;
          node->type_info_->source_ = target;
          if (enum_node == nullptr) {
            throw Error("SecondChecker : not found needed type");
          }
        }
      }
    } else if (node->reference_type_ != nullptr) {
      GoDown(node, node->reference_type_.get());
      node->type_info_ = node->reference_type_->type_info_;
    } else if (node->array_type_ != nullptr) {
      GoDown(node, node->array_type_.get());
      node->type_info_ = node->array_type_->type_info_;
    } else {
      GoDown(node, node->unit_type_.get());
      node->type_info_ = node->unit_type_->type_info_;
    }
  } catch (Error &) { throw; }
}

void SecondChecker::GoDown(ASTNode *father, ASTNode *son) {
  son->need_calculate_ = father->need_calculate_;
  son->Accept(this);
}
