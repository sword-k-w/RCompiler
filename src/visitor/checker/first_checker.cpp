#include "visitor/checker/first_checker.h"

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

void FirstChecker::Visit(CrateNode *node) {
  try {
    for (auto &item : node->items_) {
      OldScope(node, item.get());
    }
  } catch (Error &) { throw; }
}

void FirstChecker::Visit(EnumVariantsNode *node) {
  try {
    for (auto &enum_variant : node->enum_variant_s_) {
      OldScope(node, enum_variant.get());
    }
  } catch (Error &) { throw; }
}

void FirstChecker::Visit(EnumerationNode *node) {
  try {
    node->symbol_type_ = kType;
    node->scope_->AddTypeName(node->identifier_->val_, node);
    node->scope_->AddValueName(node->identifier_->val_, node, false);
    OldScope(node, node->identifier_.get());
    if (node->enum_variants_ != nullptr) {
      NewScope(node, node->enum_variants_.get(), node->identifier_->val_);
      for (auto &enum_variant : node->enum_variants_->enum_variant_s_) {
        if (!node->enum_.emplace(enum_variant->val_).second) {
          throw Error("FirstChecker : repeated enum member");
        }
      }
    }
  } catch (Error &) { throw; }
}

void FirstChecker::Visit(LiteralExpressionNode *node) {
  try {
    if (node->char_literal_ != nullptr) {
      OldScope(node, node->char_literal_.get());
    } else if (node->string_literal_ != nullptr) {
      OldScope(node, node->string_literal_.get());
    } else if (node->raw_string_literal_ != nullptr) {
      OldScope(node, node->raw_string_literal_.get());
    } else if (node->c_string_literal_ != nullptr) {
      OldScope(node, node->c_string_literal_.get());
    } else if (node->raw_c_string_literal_ != nullptr) {
      OldScope(node, node->raw_c_string_literal_.get());
    } else if (node->integer_literal_ != nullptr) {
      OldScope(node, node->integer_literal_.get());
    } else if (node->true_ != nullptr) {
      OldScope(node, node->true_.get());
    } else {
      OldScope(node, node->false_.get());
    }
  } catch (Error &) { throw; }
}

void FirstChecker::Visit(ArrayElementsNode *node) {
  try {
    for (auto &expr : node->exprs_) {
      OldScope(node, expr.get());
    }
  } catch (Error &) { throw; }
}

void FirstChecker::Visit(ArrayExpressionNode *node) {
  try {
    if (node->array_elements_ != nullptr) {
      OldScope(node, node->array_elements_.get());
    }
  } catch (Error &) { throw; }
}

void FirstChecker::Visit(PathInExpressionNode *node) {
  try {
    OldScope(node, node->path_expr_segment1_.get());
    if (node->path_expr_segment2_ != nullptr) {
      OldScope(node, node->path_expr_segment2_.get());
    }
  } catch (Error &) { throw; }
}

void FirstChecker::Visit(StructExprFieldNode *node) {
  try {
    OldScope(node, node->identifier_.get());
    OldScope(node, node->expr_.get());
  } catch (Error &) { throw; }
}

void FirstChecker::Visit(StructExprFieldsNode *node) {
  try {
    for (auto &struct_expr_field : node->struct_expr_field_s_) {
      OldScope(node, struct_expr_field.get());
    }
  } catch (Error &) { throw; }
}

void FirstChecker::Visit(StructExpressionNode *node) {
  try {
    OldScope(node, node->path_in_expr_.get());
    if (node->struct_expr_fields_ != nullptr) {
      OldScope(node, node->struct_expr_fields_.get());
    }
  } catch (Error &) { throw; }
}

void FirstChecker::Visit(ExpressionWithoutBlockNode *node) {
  try {
    OldScope(node, node->expr_.get());
  } catch (Error &) { throw; }
}

void FirstChecker::Visit(BlockExpressionNode *node) {
  try {
    if (node->statements_ != nullptr) {
      NewScope(node, node->statements_.get(), "");
    }
  } catch (Error &) { throw; }
}

void FirstChecker::Visit(InfiniteLoopExpressionNode *node) {
  try {
    OldScope(node, node->block_expr_.get());
  } catch (Error &) { throw; }
}

void FirstChecker::Visit(ConditionsNode *node) {
  try {
    OldScope(node, node->expr_.get());
  } catch (Error &) { throw; }
}

void FirstChecker::Visit(PredicateLoopExpressionNode *node) {
  try {
    OldScope(node, node->conditions_.get());
    OldScope(node, node->block_expr_.get());
  } catch (Error &) { throw; }
}

void FirstChecker::Visit(LoopExpressionNode *node) {
  try {
    if (node->infinite_loop_expr_ != nullptr) {
      OldScope(node, node->infinite_loop_expr_.get());
    } else {
      OldScope(node, node->predicate_loop_expr_.get());
    }
  } catch (Error &) { throw; }
}

void FirstChecker::Visit(IfExpressionNode *node) {
  try {
    OldScope(node, node->conditions_.get());
    OldScope(node, node->block_expr1_.get());
    if (node->block_expr2_ != nullptr) {
      OldScope(node, node->block_expr2_.get());
    } else if (node->if_expr_ != nullptr) {
      OldScope(node, node->if_expr_.get());
    }
  } catch (Error &) { throw; }
}

void FirstChecker::Visit(ExpressionWithBlockNode *node) {
  try {
    if (node->block_expr_ != nullptr) {
      OldScope(node, node->block_expr_.get());
    } else if (node->loop_expr_ != nullptr) {
      OldScope(node, node->loop_expr_.get());
    } else {
      OldScope(node, node->if_expr_.get());
    }
  } catch (Error &) { throw; }
}

void FirstChecker::Visit(CallParamsNode *node) {
  try {
    for (auto &expr : node->exprs_) {
      OldScope(node, expr.get());
    }
  } catch (Error &) { throw; }
}

void FirstChecker::Visit(ExpressionNode *node) {
  try {
    if (node->type_ == kLiteralExpr) {
      OldScope(node, node->literal_expr_.get());
    } else if (node->type_ == kPathExpr) {
      OldScope(node, node->path_expr_.get());
    } else if (node->type_ == kArrayExpr) {
      OldScope(node, node->array_expr_.get());
    } else if (node->type_ == kStructExpr) {
      OldScope(node, node->struct_expr_.get());
    } else if (node->type_ == kContinueExpr) {
      OldScope(node, node->continue_expr_.get());
    } else if (node->type_ == kUnderscoreExpr) {
      OldScope(node, node->underscore_expr_.get());
    } else if (node->type_ == kBorrowExpr || node->type_ == kDereferenceExpr || node->type_ == kNegationExpr
      || node->type_ == kGroupedExpr || node->type_ == kBreakExpr || node->type_ == kReturnExpr) {
      if (node->expr1_ != nullptr) {
        OldScope(node, node->expr1_.get());
      }
    } else if (node->type_ == kArithmeticOrLogicExpr || node->type_ == kComparisonExpr || node->type_ == kLazyBooleanExpr
      || node->type_ == kAssignmentExpr || node->type_ == kCompoundAssignmentExpr || node->type_ == kIndexExpr) {
      OldScope(node, node->expr1_.get());
      OldScope(node, node->expr2_.get());
    } else if (node->type_ == kTypeCastExpr) {
      OldScope(node, node->expr1_.get());
      OldScope(node, node->type_no_bounds_.get());
    } else if (node->type_ == kCallExpr) {
      OldScope(node, node->expr1_.get());
      if (node->call_params_) {
        OldScope(node, node->call_params_.get());
      }
    } else if (node->type_ == kMethodCallExpr) {
      OldScope(node, node->expr1_.get());
      OldScope(node, node->path_expr_segment_.get());
      if (node->call_params_) {
        OldScope(node, node->call_params_.get());
      }
    } else if (node->type_ == kFieldExpr) {
      OldScope(node, node->expr1_.get());
      OldScope(node, node->identifier_.get());
    } else {
      OldScope(node, node->expr_with_block_.get());
    }
  } catch (Error &) { throw; }
}

void FirstChecker::Visit(ShorthandSelfNode *node) {}


void FirstChecker::Visit(SelfParamNode *node) {
  try {
    OldScope(node, node->shorthand_self_.get());
  } catch (Error &) { throw; }
}

void FirstChecker::Visit(FunctionParamNode *node) {
  try {
    OldScope(node, node->pattern_no_top_alt_.get());
    OldScope(node, node->type_.get());
  } catch (Error &) { throw; }
}

void FirstChecker::Visit(FunctionParametersNode *node) {
  try {
    if (node->self_param_ != nullptr) {
      OldScope(node, node->self_param_.get());
    }
    for (auto &function_param : node->function_params_) {
      OldScope(node, function_param.get());
    }
  } catch (Error &) { throw; }
}

void FirstChecker::Visit(FunctionReturnTypeNode *node) {
  try {
    OldScope(node, node->type_.get());
  } catch (Error &) { throw; }
}

void FirstChecker::Visit(FunctionNode *node) {
  try {
    node->symbol_type_ = kFunction;
    node->scope_->AddValueName(node->identifier_->val_, node, false);
    OldScope(node, node->identifier_.get());
    if (node->function_parameters_ != nullptr) {
      NewScope(node, node->function_parameters_.get(), "");
    }
    if (node->function_return_type_ != nullptr) {
      OldScope(node, node->function_return_type_.get());
    }
    if (node->block_expr_ != nullptr) {
      if (node->function_parameters_ != nullptr) {
        OldScope(node->function_parameters_.get(), node->block_expr_.get());
      } else {
        NewScope(node, node->block_expr_.get(), "");
      }
    }
  } catch (Error &) { throw; }
}

void FirstChecker::Visit(ImplementationNode *node) {
  try {
    if (node->type_->type_path_ == nullptr || node->type_->type_path_->identifier_ == nullptr) {
      throw Error("FirstChecker : can't impl the type that isn't struct");
    }
    ASTNode *target = node->scope_->FindTypeName(node->type_->type_path_->identifier_->val_);
    if (target == nullptr) {
      throw Error("FirstChecker : impl the type that is not found");
    }
    auto struct_node = dynamic_cast<StructNode *>(target);
    if (struct_node == nullptr) {
      throw Error("FirstChecker : can't impl the type that isn't struct");
    }
    OldScope(node, node->type_.get());
    for (auto &associated_item : node->associated_items_) {
      if (associated_item->function_ != nullptr) {
        if (!struct_node->impl_.emplace(associated_item->function_->identifier_->val_, associated_item->function_.get()).second) {
          throw Error("FirstChecker : repeat method name for the same struct");
        }
      } else {
        if (!struct_node->impl_.emplace(associated_item->constant_item_->identifier_->val_, associated_item->constant_item_.get()).second) {
          throw Error("FirstChecker : repeat const name for the same struct");
        }
      }
      associated_item->in_implement_ = true;
      NewScope(struct_node, associated_item.get(), "?");
    }
    if (node->identifier_ != nullptr) {
      OldScope(node, node->identifier_.get());
      ASTNode *tmp = node->scope_->FindTypeName(node->identifier_->val_);
      if (tmp == nullptr) {
        throw Error("FirstChecker : impl the trait that is not found");
      }
      auto trait_node = dynamic_cast<TraitNode *>(tmp);
      if (trait_node == nullptr) {
        throw Error("FirstChecker : can't impl non-trait item");
      }
      auto items = trait_node->items_;
      for (auto &associated_item : node->associated_items_) {
        if (associated_item->function_ != nullptr) {
          auto it = items.find(associated_item->function_->identifier_->val_);
          if (it == items.end()) {
            throw Error("FirstChecker : impl a function that not in trait");
          }
          auto function_node = dynamic_cast<FunctionNode *>(it->second);
          if (function_node == nullptr) {
            throw Error("FirstChecker : impl a function which is const in trait");
          }
          items.erase(it);
        } else {
          auto it = items.find(associated_item->constant_item_->identifier_->val_);
          if (it == items.end()) {
            throw Error("FirstChecker : impl a const that not in trait");
          }
          auto const_item = dynamic_cast<ConstantItemNode *>(it->second);
          if (const_item == nullptr) {
            throw Error("FirstChecker : impl a const which is function in trait");
          }
          items.erase(it);
        }
      }
      if (!items.empty()) {
        throw Error("haven't implement trait clone yet!");
      }
    }
  } catch (Error &) { throw; }
}

void FirstChecker::Visit(ConstantItemNode *node) {
  try {
    node->symbol_type_ = kConst;
    node->scope_->AddValueName(node->identifier_->val_, node, false);
    OldScope(node, node->identifier_.get());
    OldScope(node, node->expr_.get());
    if (node->expr_ != nullptr) {
      OldScope(node, node->expr_.get());
    }
  } catch (Error &) { throw; }
}

void FirstChecker::Visit(AssociatedItemNode *node) {
  try {
    if (node->constant_item_ != nullptr) {
      node->constant_item_->in_trait_ = node->in_trait_;
      node->constant_item_->in_implement_ = node->in_implement_;
      OldScope(node, node->constant_item_.get());
    } else {
      node->function_->in_trait_ = node->in_trait_;
      node->function_->in_implement_ = node->in_implement_;
      OldScope(node, node->function_.get());
    }
  } catch (Error &) { throw; }
}

void FirstChecker::Visit(ItemNode *node) {
  try {
    if (node->function_ != nullptr) {
      OldScope(node, node->function_.get());
    } else if (node->struct_ != nullptr) {
      OldScope(node, node->struct_.get());
    } else if (node->enumeration_ != nullptr) {
      OldScope(node, node->enumeration_.get());
    } else if (node->constant_item_ != nullptr) {
      OldScope(node, node->constant_item_.get());
    } else if (node->trait_ != nullptr) {
      OldScope(node, node->trait_.get());
    } else {
      node->implementation_->scope_ = node->scope_;
      node_queue_.emplace_back(node->implementation_.get());
    }
  } catch (Error &) { throw; }
}

void FirstChecker::Visit(PathIdentSegmentNode *node) {
  try {
    if (node->identifier_ != nullptr) {
      OldScope(node, node->identifier_.get());
    }
  } catch (Error &) { throw; }
}

void FirstChecker::Visit(IdentifierPatternNode *node) {
  try {
    OldScope(node, node->identifier_.get());
  } catch (Error &) { throw; }
}

void FirstChecker::Visit(ReferencePatternNode *node) {
  try {
    OldScope(node, node->pattern_without_range_.get());
  } catch (Error &) { throw; }
}

void FirstChecker::Visit(PatternWithoutRangeNode *node) {
  try {
    if (node->identifier_pattern_ != nullptr) {
      OldScope(node, node->identifier_pattern_.get());
    } else if (node->wildcard_pattern_ != nullptr) {
      OldScope(node, node->wildcard_pattern_.get());
    } else {
      OldScope(node, node->reference_pattern_.get());
    }
  } catch (Error &) { throw; }
}

void FirstChecker::Visit(LetStatementNode *node) {
  try {
    OldScope(node, node->pattern_no_top_alt_.get());
    OldScope(node, node->type_.get());
    OldScope(node, node->expr_.get());
  } catch (Error &) { throw; }
}

void FirstChecker::Visit(ExpressionStatementNode *node) {
  try {
    OldScope(node, node->expr_.get());
  } catch (Error &) { throw; }
}

void FirstChecker::Visit(StatementNode *node) {
  try {
    if (node->item_ != nullptr) {
      OldScope(node, node->item_.get());
    } else if (node->let_statement_ != nullptr) {
      OldScope(node, node->let_statement_.get());
    } else if (node->expr_statement_ != nullptr) {
      OldScope(node, node->expr_statement_.get());
    }
  } catch (Error &) { throw; }
}

void FirstChecker::Visit(StatementsNode *node) {
  try {
    for (auto &statement : node->statement_s_) {
      OldScope(node, statement.get());
    }
    if (node->expr_without_block_ != nullptr) {
      OldScope(node, node->expr_without_block_.get());
    }
  } catch (Error &) { throw; }
}

void FirstChecker::Visit(StructFieldNode *node) {
  try {
    OldScope(node, node->identifier_.get());
    OldScope(node, node->type_.get());
  } catch (Error &) { throw; }
}

void FirstChecker::Visit(StructFieldsNode *node) {
  try {
    for (auto &struct_field : node->struct_field_s_) {
      OldScope(node, struct_field.get());
    }
  } catch (Error &) { throw; }
}

void FirstChecker::Visit(StructNode *node) {
  try {
    node->symbol_type_ = kType;
    node->scope_->AddTypeName(node->identifier_->val_, node);
    node->scope_->AddValueName(node->identifier_->val_, node, false);
    OldScope(node, node->identifier_.get());
    if (node->struct_fields_ != nullptr) {
      NewScope(node, node->struct_fields_.get(), node->identifier_->val_);
    }
  } catch (Error &) { throw; }
}

void FirstChecker::Visit(IdentifierNode *node) {}

void FirstChecker::Visit(CharLiteralNode *node) {}

void FirstChecker::Visit(StringLiteralNode *node) {}

void FirstChecker::Visit(RawStringLiteralNode *node) {}

void FirstChecker::Visit(CStringLiteralNode *node) {}

void FirstChecker::Visit(RawCStringLiteralNode *node) {}

void FirstChecker::Visit(IntegerLiteralNode *node) {}

void FirstChecker::Visit(TrueNode *node) {}

void FirstChecker::Visit(FalseNode *node) {}

void FirstChecker::Visit(SelfLowerNode *node) {}

void FirstChecker::Visit(SelfUpperNode *node) {}

void FirstChecker::Visit(UnderscoreExpressionNode *node) {}

void FirstChecker::Visit(ContinueExpressionNode *node) {}

void FirstChecker::Visit(TraitNode *node) {
  try {
    node->symbol_type_ = kTrait;
    node->scope_->AddTypeName(node->identifier_->val_, node);
    OldScope(node, node->identifier_.get());
    for (auto &associated_item : node->asscociated_items_) {
      associated_item->in_trait_ = true;
      OldScope(node, associated_item.get());
      if (associated_item->function_ != nullptr) {
        if (!node->items_.emplace(associated_item->function_->identifier_->val_, associated_item->function_.get()).second) {
          throw Error("FirstChecker : repeated identifier in trait");
        }
      } else {
        if (!node->items_.emplace(associated_item->constant_item_->identifier_->val_, associated_item->constant_item_.get()).second) {
          throw Error("FirstChecker : repeated identifier in trait");
        }
      }
    }
  } catch (Error &) { throw; }
}

void FirstChecker::Visit(ReferenceTypeNode *node) {
  try {
    OldScope(node, node->type_no_bounds_.get());
  } catch (Error &) { throw; }
}

void FirstChecker::Visit(ArrayTypeNode *node) {
  try {
    OldScope(node, node->type_.get());
    OldScope(node, node->expr_.get());
  } catch (Error &) { throw; }
}

void FirstChecker::Visit(UnitTypeNode *node) {}

void FirstChecker::Visit(TypeNoBoundsNode *node) {
  try {
    if (node->type_path_ != nullptr) {
      OldScope(node, node->type_path_.get());
    } else if (node->reference_type_ != nullptr) {
      OldScope(node, node->reference_type_.get());
    } else if (node->array_type_ != nullptr) {
      OldScope(node, node->array_type_.get());
    } else {
      OldScope(node, node->unit_type_.get());
    }
  } catch (Error &) { throw; }
}

void FirstChecker::Run(CrateNode *node) {
  try {
    node->scope_ = std::make_shared<Scope>(nullptr, "");
    node_queue_.emplace_front(node);
    while (!node_queue_.empty()) {
      ASTNode *tmp = node_queue_.front();
      node_queue_.pop_front();
      tmp->Accept(this);
    }
  } catch (Error &) { throw; }
}

void FirstChecker::NewScope(ASTNode *father, ASTNode *son, const std::string &name) {
  son->scope_ = std::make_shared<Scope>(father->scope_, *father->scope_->name_ + "::" + name);
  node_queue_.emplace_back(son);
}

void FirstChecker::OldScope(ASTNode *father, ASTNode *son) {
  son->scope_ = father->scope_;
  node_queue_.emplace_front(son);
}
