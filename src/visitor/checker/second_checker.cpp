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

void SecondChecker::Visit(CrateNode *node) {
  try {
    for (auto &item : node->items_) {
      item->Accept(this);
    }
  } catch (Error &) {
    throw;
  }
}

void SecondChecker::Visit(EnumVariantsNode *node) {
  try {
    for (auto &enum_variant : node->enum_variant_s_) {
      enum_variant->Accept(this);
    }
  } catch (Error &) {
    throw;
  }
}

void SecondChecker::Visit(EnumerationNode *node) {
  try {
    if (node->enum_variants_ != nullptr) {
      node->enum_variants_->Accept(this);
    }
  } catch (Error &) {
    throw;
  }
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
    choose->need_calculate_ = node->need_calculate_;
    choose->Accept(this);
    if (node->need_calculate_) {
      node->type_value_ = choose->type_value_;
    }
  } catch (Error &) {
    throw;
  }
}

void SecondChecker::Visit(ArrayElementsNode *node) {
  try {
    if (node->semicolon_) {
      node->exprs_[0]->need_calculate_ = node->need_calculate_;
      node->exprs_[0]->expect_type_ = node->expect_type_;
      node->exprs_[0]->Accept(this);
      node->exprs_[1]->need_calculate_ = node->need_calculate_;
      node->exprs_[1]->expect_type_ = std::make_shared<std::string>("usize");
      node->exprs_[1]->Accept(this);

      if (node->need_calculate_) {
        node->type_value_->type_ = kArrayType;
        std::vector<std::shared_ptr<TypeValue>> type_values(*node->exprs_[1]->type_value_->u32_value_, node->exprs_[0]->type_value_);
        node->type_value_->array_type_info_ = std::make_shared<ArrayTypeInfo>(type_values, *node->exprs_[1]->type_value_->u32_value_);
      }
    } else {
      std::vector<std::shared_ptr<TypeValue>> type_values;
      for (auto &expr : node->exprs_) {
        expr->need_calculate_ = node->need_calculate_;
        expr->expect_type_ = node->expect_type_;
        expr->Accept(this);
        type_values.emplace_back(expr->type_value_);
      }
      if (node->need_calculate_) {
        node->type_value_->type_ = kArrayType;
        node->type_value_->array_type_info_ = std::make_shared<ArrayTypeInfo>(type_values, node->exprs_.size());
      }
    }
  } catch (Error &) {
    throw;
  }
}

void SecondChecker::Visit(ArrayExpressionNode *node) {
  try {
    if (node->array_elements_ != nullptr) {
      node->array_elements_->need_calculate_ = node->need_calculate_;
      node->array_elements_->expect_type_ = node->expect_type_;
      node->array_elements_->Accept(this);
      if (node->need_calculate_) {
        node->type_value_ = node->array_elements_->type_value_;
      }
    }
  } catch (Error &) {
    throw;
  }
}

void SecondChecker::Visit(PathInExpressionNode *node) {
  try {
    node->path_expr_segment1_->need_calculate_ = node->need_calculate_;
    node->path_expr_segment1_->expect_type_ = node->expect_type_;
    node->path_expr_segment1_->Accept(this);
    if (node->path_expr_segment2_ != nullptr) {
      node->path_expr_segment2_->need_calculate_ = node->need_calculate_;
      node->path_expr_segment2_->expect_type_ = node->expect_type_;
      node->path_expr_segment2_->Accept(this);
    }
  } catch (Error &) {
    throw;
  }
}

void SecondChecker::Visit(StructExprFieldNode *node) {
  try {
    node->identifier_->need_calculate_ = node->need_calculate_;
    node->identifier_->expect_type_ = node->expect_type_;
    node->identifier_->Accept(this);
    if (node->expr_ != nullptr) {
      node->expr_->need_calculate_ = node->need_calculate_;
      node->expr_->expect_type_ = node->expect_type_;
      node->expr_->Accept(this);
      if (node->need_calculate_) {
        node->type_value_ = node->expr_->type_value_;
      }

    } else if (node->need_calculate_) {
      auto target = node->scope_->FindValueName(*node->identifier_->val_);
      if (target == nullptr) {
        throw Error("SecondChecker : struct expr field omit expr but can't find identifier");
      }
      auto constant_item = dynamic_cast<ConstantItemNode *>(target);
      if (constant_item == nullptr) {
        throw Error("SecondChecker : struct expr field omit expr but the identifier isn't const");
      }
      node->type_value_ = constant_item->type_value_;
    }
  } catch (Error &) {
    throw;
  }
}

void SecondChecker::Visit(StructExprFieldsNode *node) {
  try {
    std::set<std::string> identifiers;
    for (auto &struct_expr_field : node->struct_expr_field_s_) {
      struct_expr_field->need_calculate_ = node->need_calculate_;
      struct_expr_field->expect_type_ = node->expect_type_;
      struct_expr_field->Accept(this);
      if (!identifiers.insert(*struct_expr_field->identifier_->val_).second) {
        throw Error("SecondChecker : same identifier in struct expr fields");
      }
    }
  } catch (Error &) {
    throw;
  }
}

void SecondChecker::Visit(StructExpressionNode *node) {
  try {
    node->path_in_expr_->need_calculate_ = node->need_calculate_;
    node->path_in_expr_->expect_type_ = node->expect_type_;
    node->path_in_expr_->Accept(this);
    if (node->struct_expr_fields_) {
      node->struct_expr_fields_->need_calculate_ = node->need_calculate_;
      node->struct_expr_fields_->expect_type_ = node->expect_type_;
      node->struct_expr_fields_->Accept(this);
    }

    if (node->need_calculate_) {
      if (node->path_in_expr_->path_expr_segment2_ != nullptr) {
        throw Error("SecondChecker : const struct but not single path");
      }
      if (node->path_in_expr_->path_expr_segment1_->identifier_ == nullptr) {
        throw Error("SecondChecker : const struct but not identifier");
      }
      ASTNode *target = node->scope_->FindTypeName(*node->path_in_expr_->path_expr_segment1_->identifier_->val_);
      if (target == nullptr) {
        throw Error("SecondChecker : const struct but not found identifier");
      }
      auto struct_type = dynamic_cast<StructNode *>(target);
      if (struct_type == nullptr) {
        throw Error("SecondChecker : const struct but identifier is not a struct");
      }
      if (struct_type->type_value_->struct_type_info_->variant_.size() != node->struct_expr_fields_->struct_expr_field_s_.size()) {
        throw Error("SecondChecker : const struct but identifier count doesn't match");
      }
      node->type_value_->type_ = kStructType;
      node->type_value_->struct_type_info_ = std::make_shared<StructTypeInfo>();
      for (auto &struct_expr_field : node->struct_expr_fields_->struct_expr_field_s_) {
        auto it = struct_type->type_value_->struct_type_info_->variant_.find(*struct_expr_field->identifier_->val_);
        if (it == struct_type->type_value_->struct_type_info_->variant_.end()) {
          throw Error("SecondChecker : const struct but the identifier doesn't belong to the struct");
        }
        SameTypeCheck(it->second.get(), struct_expr_field->type_value_.get());
        node->type_value_->struct_type_info_->variant_[it->first] = struct_expr_field->type_value_;
      }
    }
  } catch (Error &) {
    throw;
  }
}

void SecondChecker::Visit(ExpressionWithoutBlockNode *node) {

}

void SecondChecker::Visit(BlockExpressionNode *node) {}
void SecondChecker::Visit(InfiniteLoopExpressionNode *node) {}
void SecondChecker::Visit(ConditionsNode *node) {}
void SecondChecker::Visit(PredicateLoopExpressionNode *node) {}
void SecondChecker::Visit(LoopExpressionNode *node) {}
void SecondChecker::Visit(IfExpressionNode *node) {}
void SecondChecker::Visit(ExpressionWithBlockNode *node) {}
void SecondChecker::Visit(CallParamsNode *node) {}
void SecondChecker::Visit(ExpressionNode *node) {}
void SecondChecker::Visit(ShorthandSelfNode *node) {}
void SecondChecker::Visit(TypedSelfNode *node) {}
void SecondChecker::Visit(SelfParamNode *node) {}
void SecondChecker::Visit(FunctionParamNode *node) {}
void SecondChecker::Visit(FunctionParametersNode *node) {}
void SecondChecker::Visit(FunctionReturnTypeNode *node) {}
void SecondChecker::Visit(FunctionNode *node) {}
void SecondChecker::Visit(ImplementationNode *node) {}

void SecondChecker::Visit(ConstantItemNode *node) {
  try {
    node->type_->Accept(this);
    if (node->expr_ == nullptr) {
      if (!node->in_trait_) {
        throw Error("SecondChecker : a const item with no expr but not in trait");
      }
    } else {
      node->expr_->need_calculate_ = true;
      node->expr_->Accept(this);
      node->type_value_ = node->expr_->type_value_;
      TypeCast(node->type_->type_value_.get(), node->type_value_.get());
    }
  } catch (Error &) {
    throw;
  }
}

void SecondChecker::Visit(AssociatedItemNode *node) {
  try {
    if (node->function_ != nullptr) {
      node->function_->Accept(this);
    } else {
      node->constant_item_->Accept(this);
    }
  } catch (Error &) {
    throw;
  }
}

void SecondChecker::Visit(ItemNode *node) {
  try {
    if (node->function_ != nullptr) {
      node->function_->Accept(this);
    } else if (node->struct_ != nullptr) {
      node->struct_->Accept(this);
    } else if (node->enumeration_ != nullptr) {
      node->enumeration_->Accept(this);
    } else if (node->constant_item_ != nullptr) {
      node->constant_item_->Accept(this);
    } else if (node->trait_ != nullptr) {
      node->trait_->Accept(this);
    } else {
      node->implementation_->Accept(this);
    }
  } catch (Error &) {
    throw;
  }
}

void SecondChecker::Visit(PathIdentSegmentNode *node) {}
void SecondChecker::Visit(LiteralPatternNode *node) {}
void SecondChecker::Visit(IdentifierPatternNode *node) {}
void SecondChecker::Visit(ReferencePatternNode *node) {}
void SecondChecker::Visit(PatternWithoutRangeNode *node) {}
void SecondChecker::Visit(LetStatementNode *node) {}
void SecondChecker::Visit(ExpressionStatementNode *node) {}
void SecondChecker::Visit(StatementNode *node) {}
void SecondChecker::Visit(StatementsNode *node) {}
void SecondChecker::Visit(StructFieldNode *node) {}
void SecondChecker::Visit(StructFieldsNode *node) {}
void SecondChecker::Visit(StructNode *node) {}
void SecondChecker::Visit(IdentifierNode *node) {}
void SecondChecker::Visit(CharLiteralNode *node) {}
void SecondChecker::Visit(StringLiteralNode *node) {}
void SecondChecker::Visit(RawStringLiteralNode *node) {}
void SecondChecker::Visit(CStringLiteralNode *node) {}
void SecondChecker::Visit(RawCStringLiteralNode *node) {}
void SecondChecker::Visit(IntegerLiteralNode *node) {}
void SecondChecker::Visit(TrueNode *node) {}
void SecondChecker::Visit(FalseNode *node) {}
void SecondChecker::Visit(SuperNode *node) {}
void SecondChecker::Visit(SelfLowerNode *node) {}
void SecondChecker::Visit(SelfUpperNode *node) {}
void SecondChecker::Visit(UnderscoreExpressionNode *node) {}
void SecondChecker::Visit(ContinueExpressionNode *node) {}
void SecondChecker::Visit(TraitNode *node) {}
void SecondChecker::Visit(ReferenceTypeNode *node) {}
void SecondChecker::Visit(ArrayTypeNode *node) {}
void SecondChecker::Visit(UnitTypeNode *node) {}
void SecondChecker::Visit(TypeNoBoundsNode *node) {}