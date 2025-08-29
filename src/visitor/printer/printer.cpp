#include "visitor/printer/printer.h"
#include "parser/node/AST_node.h"
#include "parser/node/crate.h"
#include "parser/node/enumeration.h"
#include "parser/node/expression.h"
#include "parser/node/function.h"
#include "parser/node/implementation.h"
#include "parser/node/item.h"
#include "parser/node/path.h"
#include "parser/node/pattern.h"
#include "parser/node/statement.h"
#include "parser/node/struct.h"
#include "parser/node/terminal.h"
#include "parser/node/trait.h"
#include "parser/node/type.h"

Printer::Printer(std::ostream &os) : os_(os) {}

void Printer::Prepare() {
  prefixes_.emplace("");
  is_lasts_.emplace(true);
}

void Printer::Visit(CrateNode *node) {
  os_ << prefixes_.top();
  os_ << (is_lasts_.top() ? "└──" : "├──");
  os_ << "Crate\n";

  std::string next = prefixes_.top() + (is_lasts_.top() ?  "    " : "│   ");
  for (uint32_t i = 0; i < node->items_.size(); ++i) {
    prefixes_.emplace(next);
    is_lasts_.emplace(i + 1 == node->items_.size());
    node->items_[i]->Accept(this);
  }

  prefixes_.pop();
  is_lasts_.pop();
}

void Printer::Visit(EnumVariantsNode *node) {
  os_ << prefixes_.top();
  os_ << (is_lasts_.top() ? "└──" : "├──");
  os_ << "Enum Variants\n";

  std::string next = prefixes_.top() + (is_lasts_.top() ?  "    " : "│   ");
  for (uint32_t i = 0; i < node->enum_variant_s_.size(); ++i) {
    prefixes_.emplace(next);
    is_lasts_.emplace(i + 1 == node->enum_variant_s_.size() && node->comma_cnt_ < node->enum_variant_s_.size());
    node->enum_variant_s_[i]->Accept(this);
    os_ << next;
    if (i + 1 == node->enum_variant_s_.size() && node->comma_cnt_ == node->enum_variant_s_.size()) {
      os_ << "└──,\n";
    } else {
      os_ << "├──,\n";
    }
  }

  prefixes_.pop();
  is_lasts_.pop();
}

void Printer::Visit(EnumerationNode *node) {
  os_ << prefixes_.top();
  os_ << (is_lasts_.top() ? "└──" : "├──");
  os_ << "Enumeration\n";

  std::string next = prefixes_.top() + (is_lasts_.top() ?  "    " : "│   ");
  os_ << next << "├──enum\n";
  prefixes_.emplace(next);
  is_lasts_.emplace(false);
  node->identifier_->Accept(this);
  os_ << next << "├──{\n";
  if (node->enum_variants_) {
    prefixes_.emplace(next);
    is_lasts_.emplace(false);
    node->enum_variants_->Accept(this);
  }
  os_ << next << "└──}\n";

  prefixes_.pop();
  is_lasts_.pop();
}

void Printer::Visit(LiteralExpressionNode *node) {
  os_ << prefixes_.top();
  os_ << (is_lasts_.top() ? "└──" : "├──");
  os_ << "Literal Expression\n";
  std::string next = prefixes_.top() + (is_lasts_.top() ?  "    " : "│   ");

  if (node->char_literal_ != nullptr) {
    prefixes_.emplace(next);
    is_lasts_.emplace(true);
    node->char_literal_->Accept(this);
  } else if (node->string_literal_ != nullptr) {
    prefixes_.emplace(next);
    is_lasts_.emplace(true);
    node->string_literal_->Accept(this);
  } else if (node->raw_string_literal_ != nullptr) {
    prefixes_.emplace(next);
    is_lasts_.emplace(true);
    node->raw_string_literal_->Accept(this);
  } else if (node->c_string_literal_ != nullptr) {
    prefixes_.emplace(next);
    is_lasts_.emplace(true);
    node->c_string_literal_->Accept(this);
  } else if (node->raw_c_string_literal_ != nullptr) {
    prefixes_.emplace(next);
    is_lasts_.emplace(true);
    node->raw_c_string_literal_->Accept(this);
  } else if (node->integer_literal_ != nullptr) {
    prefixes_.emplace(next);
    is_lasts_.emplace(true);
    node->integer_literal_->Accept(this);
  } else if (node->true_ != nullptr) {
    prefixes_.emplace(next);
    is_lasts_.emplace(true);
    node->true_->Accept(this);
  } else {
    prefixes_.emplace(next);
    is_lasts_.emplace(true);
    node->false_->Accept(this);
  }

  prefixes_.pop();
  is_lasts_.pop();
}

void Printer::Visit(ArrayElementsNode *node) {
  os_ << prefixes_.top();
  os_ << (is_lasts_.top() ? "└──" : "├──");
  os_ << "Array Elements\n";

  std::string next = prefixes_.top() + (is_lasts_.top() ?  "    " : "│   ");

  if (node->semicolon_) {
    prefixes_.emplace(next);
    is_lasts_.emplace(false);
    node->exprs_[0]->Accept(this);
    os_ << next << "├──;\n";
    prefixes_.emplace(next);
    is_lasts_.emplace(true);
    node->exprs_[1]->Accept(this);
  } else {
    for (uint32_t i = 0; i < node->exprs_.size(); ++i) {
      prefixes_.emplace(next);
      is_lasts_.emplace(i + 1 == node->exprs_.size() && node->comma_cnt_ < node->exprs_.size());
      node->exprs_[i]->Accept(this);
      if (i + 1 <= node->comma_cnt_) {
        os_ << next;
        if (i + 1 == node->exprs_.size() && node->comma_cnt_ == node->exprs_.size()) {
          os_ << "└──,\n";
        } else {
          os_ << "├──,\n";
        }
      }
    }
  }

  prefixes_.pop();
  is_lasts_.pop();
}

void Printer::Visit(ArrayExpressionNode *node) {
  os_ << prefixes_.top();
  os_ << (is_lasts_.top() ? "└──" : "├──");
  os_ << "Array Expression\n";

  std::string next = prefixes_.top() + (is_lasts_.top() ?  "    " : "│   ");
  os_ << next << "├──[\n";
  if (node->array_elements_ != nullptr) {
    prefixes_.emplace(next);
    is_lasts_.emplace(false);
    node->array_elements_->Accept(this);
  }
  os_ << next << "└──]\n";

  prefixes_.pop();
  is_lasts_.pop();
}

void Printer::Visit(PathInExpressionNode *node) {
  os_ << prefixes_.top();
  os_ << (is_lasts_.top() ? "└──" : "├──");
  os_ << "Path In Expression\n";

  std::string next = prefixes_.top() + (is_lasts_.top() ?  "    " : "│   ");

  prefixes_.emplace(next);
  is_lasts_.emplace(node->path_expr_segments_.size() == 1);
  node->path_expr_segments_[0]->Accept(this);
  for (uint32_t i = 1; i < node->path_expr_segments_.size(); ++i) {
    os_ << next << "├──::\n";
    prefixes_.emplace(next);
    is_lasts_.emplace(i + 1 == node->path_expr_segments_.size());
    node->path_expr_segments_[i]->Accept(this);
  }

  prefixes_.pop();
  is_lasts_.pop();
}

void Printer::Visit(StructExprFieldNode *node) {
  os_ << prefixes_.top();
  os_ << (is_lasts_.top() ? "└──" : "├──");
  os_ << "Path In Expression\n";

  std::string next = prefixes_.top() + (is_lasts_.top() ?  "    " : "│   ");
  prefixes_.emplace(next);
  is_lasts_.emplace(node->expr_ == nullptr);
  node->identifier_->Accept(this);
  if (node->expr_ != nullptr) {
    os_ << next << "├──:\n";
    prefixes_.emplace(next);
    is_lasts_.emplace(true);
    node->expr_->Accept(this);
  }

  prefixes_.pop();
  is_lasts_.pop();
}

void Printer::Visit(StructExprFieldsNode *node) {
  os_ << prefixes_.top();
  os_ << (is_lasts_.top() ? "└──" : "├──");
  os_ << "Struct Expr Fields\n";

  std::string next = prefixes_.top() + (is_lasts_.top() ?  "    " : "│   ");

  for (uint32_t i = 0; i < node->struct_expr_field_s_.size(); ++i) {
    prefixes_.emplace(next);
    is_lasts_.emplace(i + 1 == node->struct_expr_field_s_.size() && node->comma_cnt_ < node->struct_expr_field_s_.size());
    node->struct_expr_field_s_[i]->Accept(this);
    if (i + 1 <= node->comma_cnt_) {
      os_ << next;
      if (i + 1 == node->struct_expr_field_s_.size() && node->comma_cnt_ == node->struct_expr_field_s_.size()) {
        os_ << "└──,\n";
      } else {
        os_ << "├──,\n";
      }
    }
  }

  prefixes_.pop();
  is_lasts_.pop();
}

void Printer::Visit(StructExpressionNode *node) {
  os_ << prefixes_.top();
  os_ << (is_lasts_.top() ? "└──" : "├──");
  os_ << "Struct Expression\n";

  std::string next = prefixes_.top() + (is_lasts_.top() ?  "    " : "│   ");
  prefixes_.emplace(next);
  is_lasts_.emplace(false);
  node->path_in_expr_->Accept(this);
  os_ << next << "├──{\n";
  if (node->struct_expr_fields_ != nullptr) {
    prefixes_.emplace(next);
    is_lasts_.emplace(false);
    node->struct_expr_fields_->Accept(this);
  }
  os_ << next << "└──}\n";

  prefixes_.pop();
  is_lasts_.pop();
}

void Printer::Visit(ExpressionWithoutBlockNode *node) {
  os_ << prefixes_.top();
  os_ << (is_lasts_.top() ? "└──" : "├──");
  os_ << "Expression Without Block\n";

  std::string next = prefixes_.top() + (is_lasts_.top() ?  "    " : "│   ");
  prefixes_.emplace(next);
  is_lasts_.emplace(true);
  node->expr_->Accept(this);

  prefixes_.pop();
  is_lasts_.pop();
}

void Printer::Visit(BlockExpressionNode *node) {
  os_ << prefixes_.top();
  os_ << (is_lasts_.top() ? "└──" : "├──");
  os_ << "Block Expression\n";

  std::string next = prefixes_.top() + (is_lasts_.top() ?  "    " : "│   ");

  os_ << next << "├──{\n";
  if (node->statements_ != nullptr) {
    prefixes_.emplace(next);
    is_lasts_.emplace(false);
    node->statements_->Accept(this);
  }
  os_ << next << "└──}\n";

  prefixes_.pop();
  is_lasts_.pop();
}

void Printer::Visit(InfiniteLoopExpressionNode *node) {
  os_ << prefixes_.top();
  os_ << (is_lasts_.top() ? "└──" : "├──");
  os_ << "Infinite Loop Expression\n";

  std::string next = prefixes_.top() + (is_lasts_.top() ?  "    " : "│   ");

  os_ << next << "├──loop\n";
  prefixes_.emplace(next);
  is_lasts_.emplace(true);
  node->block_expr_->Accept(this);

  prefixes_.pop();
  is_lasts_.pop();
}

void Printer::Visit(ConditionsNode *node) {
  os_ << prefixes_.top();
  os_ << (is_lasts_.top() ? "└──" : "├──");
  os_ << "Conditions\n";

  std::string next = prefixes_.top() + (is_lasts_.top() ?  "    " : "│   ");

  os_ << next << "├──(\n";
  prefixes_.emplace(next);
  is_lasts_.emplace(false);
  node->expr_->Accept(this);
  os_ << next << "└──)\n";

  prefixes_.pop();
  is_lasts_.pop();
}

void Printer::Visit(PredicateLoopExpressionNode *node) {
  os_ << prefixes_.top();
  os_ << (is_lasts_.top() ? "└──" : "├──");
  os_ << "Predicate Loop Expression\n";

  std::string next = prefixes_.top() + (is_lasts_.top() ?  "    " : "│   ");

  os_ << next << "├──while\n";
  prefixes_.emplace(next);
  is_lasts_.emplace(false);
  node->conditions_->Accept(this);
  prefixes_.emplace(next);
  is_lasts_.emplace(true);
  node->block_expr_->Accept(this);

  prefixes_.pop();
  is_lasts_.pop();
}

void Printer::Visit(LoopExpressionNode *node) {
  os_ << prefixes_.top();
  os_ << (is_lasts_.top() ? "└──" : "├──");
  os_ << "Loop Expression\n";

  std::string next = prefixes_.top() + (is_lasts_.top() ?  "    " : "│   ");

  if (node->infinite_loop_expr_ != nullptr) {
    prefixes_.emplace(next);
    is_lasts_.emplace(true);
    node->infinite_loop_expr_->Accept(this);
  } else {
    prefixes_.emplace(next);
    is_lasts_.emplace(true);
    node->predicate_loop_expr_->Accept(this);
  }

  prefixes_.pop();
  is_lasts_.pop();
}

void Printer::Visit(IfExpressionNode *node) {
  os_ << prefixes_.top();
  os_ << (is_lasts_.top() ? "└──" : "├──");
  os_ << "If Expression\n";

  std::string next = prefixes_.top() + (is_lasts_.top() ?  "    " : "│   ");

  os_ << next << "├──if\n";
  prefixes_.emplace(next);
  is_lasts_.emplace(false);
  node->conditions_->Accept(this);
  prefixes_.emplace(next);
  is_lasts_.emplace(node->block_expr2_ == nullptr && node->if_expr_ == nullptr);
  node->block_expr1_->Accept(this);
  if (node->block_expr2_ != nullptr) {
    os_ << next << "├──else\n";
    prefixes_.emplace(next);
    is_lasts_.emplace(true);
    node->block_expr2_->Accept(this);
  } else if (node->if_expr_ != nullptr) {
    os_ << next << "├──else\n";
    prefixes_.emplace(next);
    is_lasts_.emplace(true);
    node->if_expr_->Accept(this);
  }

  prefixes_.pop();
  is_lasts_.pop();
}

void Printer::Visit(ExpressionWithBlockNode *node) {
  os_ << prefixes_.top();
  os_ << (is_lasts_.top() ? "└──" : "├──");
  os_ << "Expression With Block\n";

  std::string next = prefixes_.top() + (is_lasts_.top() ?  "    " : "│   ");
  prefixes_.emplace(next);
  is_lasts_.emplace(true);
  if (node->block_expr_ != nullptr) {
    node->block_expr_->Accept(this);
  } else if (node->loop_expr_ != nullptr) {
    node->loop_expr_->Accept(this);
  } else {
    node->if_expr_->Accept(this);
  }

  prefixes_.pop();
  is_lasts_.pop();
}

void Printer::Visit(CallParamsNode *node) {
  os_ << prefixes_.top();
  os_ << (is_lasts_.top() ? "└──" : "├──");
  os_ << "Call Params\n";

  std::string next = prefixes_.top() + (is_lasts_.top() ?  "    " : "│   ");

  for (uint32_t i = 0; i < node->exprs_.size(); ++i) {
    prefixes_.emplace(next);
    is_lasts_.emplace(i + 1 == node->exprs_.size() && node->comma_cnt_ < node->exprs_.size());
    node->exprs_[i]->Accept(this);
    if (i + 1 <= node->comma_cnt_) {
      os_ << next;
      if (i + 1 == node->exprs_.size() && node->comma_cnt_ == node->exprs_.size()) {
        os_ << "└──,\n";
      } else {
        os_ << "├──,\n";
      }
    }
  }

  prefixes_.pop();
  is_lasts_.pop();
}

void Printer::Visit(ExpressionNode *node) {
  os_ << prefixes_.top();
  os_ << (is_lasts_.top() ? "└──" : "├──");
  os_ << "Expression (";
  Print(node->type_, os_);
  os_ << ")\n";

  std::string next = prefixes_.top() + (is_lasts_.top() ?  "    " : "│   ");

  if (node->type_ == kLiteralExpr) {
    prefixes_.emplace(next);
    is_lasts_.emplace(true);
    node->literal_expr_->Accept(this);
  } else if (node->type_ == kPathExpr) {
    prefixes_.emplace(next);
    is_lasts_.emplace(true);
    node->path_expr_->Accept(this);
  } else if (node->type_ == kArrayExpr) {
    prefixes_.emplace(next);
    is_lasts_.emplace(true);
    node->array_expr_->Accept(this);
  } else if (node->type_ == kStructExpr) {
    prefixes_.emplace(next);
    is_lasts_.emplace(true);
    node->struct_expr_->Accept(this);
  } else if (node->type_ == kContinueExpr) {
    prefixes_.emplace(next);
    is_lasts_.emplace(true);
    node->continue_expr_->Accept(this);
  } else if (node->type_ == kUnderscoreExpr) {
    prefixes_.emplace(next);
    is_lasts_.emplace(true);
    node->underscore_expr_->Accept(this);
  } else if (node->type_ == kBorrowExpr) {
    os_ << next << "├──" << node->op_ << '\n';
    if (node->mut_) {
      os_ << next << "├──mut\n";
    }
    prefixes_.emplace(next);
    is_lasts_.emplace(true);
    node->expr1_->Accept(this);
  } else if (node->type_ == kDereferenceExpr) {
    os_ << next << "├──*\n";
    prefixes_.emplace(next);
    is_lasts_.emplace(true);
    node->expr1_->Accept(this);
  } else if (node->type_ == kNegationExpr) {
    os_ << next << "├──" << node->op_ << '\n';
    prefixes_.emplace(next);
    is_lasts_.emplace(true);
    node->expr1_->Accept(this);
  } else if (node->type_ == kArithmeticOrLogicExpr || node->type_ == kComparisonExpr || node->type_ == kLazyBooleanExpr
    || node->type_ == kAssignmentExpr || node->type_ == kCompoundAssignmentExpr) {
    prefixes_.emplace(next);
    is_lasts_.emplace(false);
    node->expr1_->Accept(this);
    os_ << next << "├──" << node->op_ << '\n';
    prefixes_.emplace(next);
    is_lasts_.emplace(true);
    node->expr2_->Accept(this);
  } else if (node->type_ == kTypeCastExpr) {
    prefixes_.emplace(next);
    is_lasts_.emplace(false);
    node->expr1_->Accept(this);
    os_ << next << "├──as" << '\n';
    prefixes_.emplace(next);
    is_lasts_.emplace(true);
    node->type_no_bounds_->Accept(this);
  } else if (node->type_ == kGroupedExpr) {
    os_ << next << "├──(\n";
    prefixes_.emplace(next);
    is_lasts_.emplace(false);
    node->expr1_->Accept(this);
    os_ << next << "└──)\n";
  } else if (node->type_ == kIndexExpr) {
    prefixes_.emplace(next);
    is_lasts_.emplace(false);
    node->expr1_->Accept(this);
    os_ << next << "├──[\n";
    prefixes_.emplace(next);
    is_lasts_.emplace(false);
    node->expr2_->Accept(this);
    os_ << next << "└──]\n";
  } else if (node->type_ == kCallExpr) {
    prefixes_.emplace(next);
    is_lasts_.emplace(false);
    node->expr1_->Accept(this);
    os_ << next << "├──(\n";
    if (node->call_params_ != nullptr) {
      prefixes_.emplace(next);
      is_lasts_.emplace(false);
      node->call_params_->Accept(this);
    }
    os_ << next << "└──)\n";
  } else if (node->type_ == kMethodCallExpr) {
    prefixes_.emplace(next);
    is_lasts_.emplace(false);
    node->expr1_->Accept(this);
    os_ << next << "├──.\n";
    prefixes_.emplace(next);
    is_lasts_.emplace(false);
    node->path_expr_segment_->Accept(this);
    os_ << next << "├──(\n";
    if (node->call_params_ != nullptr) {
      prefixes_.emplace(next);
      is_lasts_.emplace(false);
      node->call_params_->Accept(this);
    }
    os_ << next << "└──)\n";
  } else if (node->type_ == kFieldExpr) {
    prefixes_.emplace(next);
    is_lasts_.emplace(false);
    node->expr1_->Accept(this);
    os_ << next << "├──." << '\n';
    prefixes_.emplace(next);
    is_lasts_.emplace(true);
    node->identifier_->Accept(this);
  } else if (node->type_ == kBreakExpr) {
    if (node->expr1_ != nullptr) {
      os_ << next << "├──break" << '\n';
      prefixes_.emplace(next);
      is_lasts_.emplace(true);
      node->expr1_->Accept(this);
    } else {
      os_ << next << "└──break" << '\n';
    }
  } else if (node->type_ == kReturnExpr) {
    if (node->expr1_ != nullptr) {
      os_ << next << "├──return" << '\n';
      prefixes_.emplace(next);
      is_lasts_.emplace(true);
      node->expr1_->Accept(this);
    } else {
      os_ << next << "└──return" << '\n';
    }
  } else {
    prefixes_.emplace(next);
    is_lasts_.emplace(true);
    node->expr_with_block_->Accept(this);
  }

  prefixes_.pop();
  is_lasts_.pop();
}

void Printer::Visit(ShorthandSelfNode *node) {
  os_ << prefixes_.top();
  os_ << (is_lasts_.top() ? "└──" : "├──");
  os_ << "Shorthand Self\n";

  std::string next = prefixes_.top() + (is_lasts_.top() ?  "    " : "│   ");
  if (node->quote_) {
    os_ << next << "├──&\n";
  }
  if (node->mut_) {
    os_ << next << "├──mut\n";
  }
  os_ << next << "└──self\n";

  prefixes_.pop();
  is_lasts_.pop();
}

void Printer::Visit(TypedSelfNode *node) {
  os_ << prefixes_.top();
  os_ << (is_lasts_.top() ? "└──" : "├──");
  os_ << "Typed Self\n";

  std::string next = prefixes_.top() + (is_lasts_.top() ?  "    " : "│   ");
  if (node->mut_) {
    os_ << next << "├──mut\n";
  }
  os_ << next << "├──self\n";
  os_ << next << "├──:\n";
  prefixes_.emplace(next);
  is_lasts_.emplace(true);
  node->type_->Accept(this);

  prefixes_.pop();
  is_lasts_.pop();
}

void Printer::Visit(SelfParamNode *node) {
  os_ << prefixes_.top();
  os_ << (is_lasts_.top() ? "└──" : "├──");
  os_ << "Self Param\n";

  std::string next = prefixes_.top() + (is_lasts_.top() ?  "    " : "│   ");
  prefixes_.emplace(next);
  is_lasts_.emplace(true);
  if (node->shorthand_self_) {
    node->shorthand_self_->Accept(this);
  } else {
    node->typed_self_->Accept(this);
  }

  prefixes_.pop();
  is_lasts_.pop();
}

void Printer::Visit(FunctionParamNode *node) {
  os_ << prefixes_.top();
  os_ << (is_lasts_.top() ? "└──" : "├──");
  os_ << "Function Param\n";

  std::string next = prefixes_.top() + (is_lasts_.top() ?  "    " : "│   ");
  prefixes_.emplace(next);
  is_lasts_.emplace(false);
  node->pattern_no_top_alt_->Accept(this);
  os_ << next << "├──:\n";
  prefixes_.emplace(next);
  is_lasts_.emplace(true);
  node->type_->Accept(this);

  prefixes_.pop();
  is_lasts_.pop();
}

void Printer::Visit(FunctionParametersNode *node) {
  os_ << prefixes_.top();
  os_ << (is_lasts_.top() ? "└──" : "├──");
  os_ << "Function Parameters\n";

  std::string next = prefixes_.top() + (is_lasts_.top() ?  "    " : "│   ");
  if (node->function_params_.empty()) {
    prefixes_.emplace(next);
    is_lasts_.emplace(node->comma_cnt_ == 0);
    node->self_param_->Accept(this);
    if (node->comma_cnt_) {
      os_ << next << "└──,\n";
    }
  } else {
    if (node->self_param_ != nullptr) {
      prefixes_.emplace(next);
      is_lasts_.emplace(false);
      node->self_param_->Accept(this);
      os_ << next << "├──,\n";
    }
    for (uint32_t i = 0; i < node->function_params_.size(); ++i) {
      prefixes_.emplace(next);
      is_lasts_.emplace(i + 1 == node->function_params_.size() && node->comma_cnt_ < node->function_params_.size() + 1);
      node->function_params_[i]->Accept(this);
      if (i + 1 < node->comma_cnt_) {
        os_ << next;
        if (i + 1 == node->function_params_.size() && node->comma_cnt_ == node->function_params_.size() + 1) {
          os_ << "└──,\n";
        } else {
          os_ << "├──,\n";
        }
      }
    }
  }

  prefixes_.pop();
  is_lasts_.pop();
}

void Printer::Visit(FunctionReturnTypeNode *node) {
  os_ << prefixes_.top();
  os_ << (is_lasts_.top() ? "└──" : "├──");
  os_ << "Path Ident Segment Expression\n";

  std::string next = prefixes_.top() + (is_lasts_.top() ?  "    " : "│   ");
  os_ << next << "├──->\n";
  prefixes_.emplace(next);
  is_lasts_.emplace(true);
  node->type_->Accept(this);

  prefixes_.pop();
  is_lasts_.pop();
}

void Printer::Visit(FunctionNode *node) {
  os_ << prefixes_.top();
  os_ << (is_lasts_.top() ? "└──" : "├──");
  os_ << "Function\n";

  std::string next = prefixes_.top() + (is_lasts_.top() ?  "    " : "│   ");
  if (node->const_) {
    os_ << next << "├──const\n";
  }
  os_ << next << "├──fn\n";
  prefixes_.emplace(next);
  is_lasts_.emplace(false);
  node->identifier_->Accept(this);
  os_ << next << "├──(\n";
  if (node->function_parameters_ != nullptr) {
    prefixes_.emplace(next);
    is_lasts_.emplace(false);
    node->function_parameters_->Accept(this);
  }
  os_ << next << "├──)\n";
  if (node->function_return_type_ != nullptr) {
    prefixes_.emplace(next);
    is_lasts_.emplace(false);
    node->function_return_type_->Accept(this);
  }
  if (node->semicolon_) {
    os_ << next << "└──;\n";
  } else {
    prefixes_.emplace(next);
    is_lasts_.emplace(true);
    node->block_expr_->Accept(this);
  }

  prefixes_.pop();
  is_lasts_.pop();
}

void Printer::Visit(ImplementationNode *node) {
  os_ << prefixes_.top();
  os_ << (is_lasts_.top() ? "└──" : "├──");
  os_ << "Implementation\n";

  std::string next = prefixes_.top() + (is_lasts_.top() ?  "    " : "│   ");
  os_ << next << "├──impl\n";
  if (node->identifier_ != nullptr) {
    prefixes_.emplace(next);
    is_lasts_.emplace(false);
    node->identifier_->Accept(this);
    os_ << next << "├──for\n";
  }
  prefixes_.emplace(next);
  is_lasts_.emplace(false);
  node->type_->Accept(this);
  os_ << next << "├──{\n";
  for (auto &associated_item : node->associated_items_) {
    prefixes_.emplace(next);
    is_lasts_.emplace(false);
    associated_item->Accept(this);
  }
  os_ << next << "└──}\n";
  prefixes_.pop();
  is_lasts_.pop();
}

void Printer::Visit(ConstantItemNode *node) {
  os_ << prefixes_.top();
  os_ << (is_lasts_.top() ? "└──" : "├──");
  os_ << "Constant Item\n";

  std::string next = prefixes_.top() + (is_lasts_.top() ?  "    " : "│   ");
  os_ << next << "├──const\n";
  prefixes_.emplace(next);
  is_lasts_.emplace(false);
  node->identifier_->Accept(this);
  os_ << next << "├──:\n";
  prefixes_.emplace(next);
  is_lasts_.emplace(false);
  node->type_->Accept(this);
  if (node->expr_ != nullptr) {
    os_ << next << "├──=\n";
    prefixes_.emplace(next);
    is_lasts_.emplace(false);
    node->expr_->Accept(this);
  }
  os_ << next << "└──;\n";

  prefixes_.pop();
  is_lasts_.pop();
}

void Printer::Visit(AssociatedItemNode *node) {
  os_ << prefixes_.top();
  os_ << (is_lasts_.top() ? "└──" : "├──");
  os_ << "Associated Item\n";

  std::string next = prefixes_.top() + (is_lasts_.top() ?  "    " : "│   ");
  prefixes_.emplace(next);
  is_lasts_.emplace(true);
  if (node->constant_item_ != nullptr) {
    node->constant_item_->Accept(this);
  } else {
    node->function_->Accept(this);
  }

  prefixes_.pop();
  is_lasts_.pop();
}

void Printer::Visit(ItemNode *node) {
  os_ << prefixes_.top();
  os_ << (is_lasts_.top() ? "└──" : "├──");
  os_ << "Item\n";

  std::string next = prefixes_.top() + (is_lasts_.top() ?  "    " : "│   ");
  prefixes_.emplace(next);
  is_lasts_.emplace(true);
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

  prefixes_.pop();
  is_lasts_.pop();
}

void Printer::Visit(PathIdentSegmentNode *node) {
  os_ << prefixes_.top();
  os_ << (is_lasts_.top() ? "└──" : "├──");
  os_ << "Path Ident Segment Expression\n";

  std::string next = prefixes_.top() + (is_lasts_.top() ?  "    " : "│   ");
  prefixes_.emplace(next);
  is_lasts_.emplace(true);
  if (node->identifier_ != nullptr) {
    node->identifier_->Accept(this);
  } else if (node->self_lower_ != nullptr) {
    node->self_lower_->Accept(this);
  } else {
    node->self_upper_->Accept(this);
  }

  prefixes_.pop();
  is_lasts_.pop();
}

void Printer::Visit(LiteralPatternNode *node) {
  os_ << prefixes_.top();
  os_ << (is_lasts_.top() ? "└──" : "├──");
  os_ << "Literal Pattern\n";

  std::string next = prefixes_.top() + (is_lasts_.top() ?  "    " : "│   ");
  if (node->hyphen_) {
    os_ << next << "├──-\n";
  }
  prefixes_.emplace(next);
  is_lasts_.emplace(true);
  node->literal_expr_->Accept(this);

  prefixes_.pop();
  is_lasts_.pop();
}

void Printer::Visit(IdentifierPatternNode *node) {
  os_ << prefixes_.top();
  os_ << (is_lasts_.top() ? "└──" : "├──");
  os_ << "Identifier Pattern\n";

  std::string next = prefixes_.top() + (is_lasts_.top() ?  "    " : "│   ");
  if (node->ref_) {
    os_ << next << "├──ref\n";
  }
  if (node->mut_) {
    os_ << next << "├──mut\n";
  }
  prefixes_.emplace(next);
  is_lasts_.emplace(node->pattern_no_top_alt_ == nullptr);
  node->identifier_->Accept(this);
  if (node->pattern_no_top_alt_ != nullptr) {
    os_ << next << "├──@\n";
    prefixes_.emplace(next);
    is_lasts_.emplace(true);
    node->pattern_no_top_alt_->Accept(this);
  }

  prefixes_.pop();
  is_lasts_.pop();
}

void Printer::Visit(ReferencePatternNode *node) {
  os_ << prefixes_.top();
  os_ << (is_lasts_.top() ? "└──" : "├──");
  os_ << "Reference Pattern\n";

  std::string next = prefixes_.top() + (is_lasts_.top() ?  "    " : "│   ");
  if (node->single_) {
    os_ << next << "├──&\n";
  } else {
    os_ << next << "├──&&\n";
  }
  if (node->mut_) {
    os_ << next << "├──mut\n";
  }
  prefixes_.emplace(next);
  is_lasts_.emplace(true);
  node->pattern_without_range_->Accept(this);

  prefixes_.pop();
  is_lasts_.pop();
}

void Printer::Visit(PatternWithoutRangeNode *node) {
  os_ << prefixes_.top();
  os_ << (is_lasts_.top() ? "└──" : "├──");
  os_ << "Pattern Without Range\n";

  std::string next = prefixes_.top() + (is_lasts_.top() ?  "    " : "│   ");
  prefixes_.emplace(next);
  is_lasts_.emplace(true);
  if (node->literal_pattern_ != nullptr) {
    node->literal_pattern_->Accept(this);
  } else if (node->identifier_pattern_ != nullptr) {
    node->identifier_pattern_->Accept(this);
  } else if (node->wildcard_pattern_ != nullptr) {
    node->wildcard_pattern_->Accept(this);
  } else if (node->reference_pattern_ != nullptr) {
    node->reference_pattern_->Accept(this);
  } else {
    node->path_pattern_->Accept(this);
  }

  prefixes_.pop();
  is_lasts_.pop();
}

void Printer::Visit(LetStatementNode *node) {
  os_ << prefixes_.top();
  os_ << (is_lasts_.top() ? "└──" : "├──");
  os_ << "Let Statement\n";

  std::string next = prefixes_.top() + (is_lasts_.top() ?  "    " : "│   ");
  os_ << next << "├──let\n";
  prefixes_.emplace(next);
  is_lasts_.emplace(false);
  node->pattern_no_top_alt_->Accept(this);
  if (node->type_ != nullptr) {
    os_ << next << "├──:\n";
    prefixes_.emplace(next);
    is_lasts_.emplace(false);
    node->type_->Accept(this);
  }
  if (node->expr_ != nullptr) {
    os_ << next << "├──=\n";
    prefixes_.emplace(next);
    is_lasts_.emplace(false);
    node->expr_->Accept(this);
  }
  os_ << next << "└──;\n";

  prefixes_.pop();
  is_lasts_.pop();
}

void Printer::Visit(ExpressionStatementNode *node) {
  os_ << prefixes_.top();
  os_ << (is_lasts_.top() ? "└──" : "├──");
  os_ << "Expression Statement\n";

  std::string next = prefixes_.top() + (is_lasts_.top() ?  "    " : "│   ");
  prefixes_.emplace(next);
  is_lasts_.emplace(!node->semicolon_);
  node->expr_->Accept(this);
  if (node->semicolon_) {
    os_ << next << "└──;\n";
  }

  prefixes_.pop();
  is_lasts_.pop();
}

void Printer::Visit(StatementNode *node) {
  os_ << prefixes_.top();
  os_ << (is_lasts_.top() ? "└──" : "├──");
  os_ << "Statement\n";

  std::string next = prefixes_.top() + (is_lasts_.top() ?  "    " : "│   ");
  if (node->semicolon_) {
    os_ << next << "└──;\n";
  } else {
    prefixes_.emplace(next);
    is_lasts_.emplace(true);
    if (node->item_ != nullptr) {
      node->item_->Accept(this);
    } else if (node->let_statement_ != nullptr) {
      node->let_statement_->Accept(this);
    } else {
      node->expr_statement_->Accept(this);
    }
  }

  prefixes_.pop();
  is_lasts_.pop();
}

void Printer::Visit(StatementsNode *node) {
  os_ << prefixes_.top();
  os_ << (is_lasts_.top() ? "└──" : "├──");
  os_ << "Statements\n";

  std::string next = prefixes_.top() + (is_lasts_.top() ?  "    " : "│   ");
  if (node->expr_without_block_ != nullptr) {
    for (auto &statement : node->statement_s_) {
      prefixes_.emplace(next);
      is_lasts_.emplace(false);
      statement->Accept(this);
    }
    prefixes_.emplace(next);
    is_lasts_.emplace(true);
    node->expr_without_block_->Accept(this);
  } else {
    for (uint32_t i = 0; i < node->statement_s_.size(); ++i) {
      prefixes_.emplace(next);
      is_lasts_.emplace(i + 1 == node->statement_s_.size());
      node->statement_s_[i]->Accept(this);
    }
  }

  prefixes_.pop();
  is_lasts_.pop();
}

void Printer::Visit(StructFieldNode *node) {
  os_ << prefixes_.top();
  os_ << (is_lasts_.top() ? "└──" : "├──");
  os_ << "Struct Field\n";

  std::string next = prefixes_.top() + (is_lasts_.top() ?  "    " : "│   ");
  prefixes_.emplace(next);
  is_lasts_.emplace(false);
  node->identifier_->Accept(this);
  os_ << next << "├──:\n";
  prefixes_.emplace(next);
  is_lasts_.emplace(true);
  node->type_->Accept(this);

  prefixes_.pop();
  is_lasts_.pop();
}

void Printer::Visit(StructFieldsNode *node) {
  os_ << prefixes_.top();
  os_ << (is_lasts_.top() ? "└──" : "├──");
  os_ << "Struct Fields\n";

  std::string next = prefixes_.top() + (is_lasts_.top() ?  "    " : "│   ");

  for (uint32_t i = 0; i < node->struct_field_s_.size(); ++i) {
    prefixes_.emplace(next);
    is_lasts_.emplace(i + 1 == node->struct_field_s_.size() && node->comma_cnt_ < node->struct_field_s_.size());
    node->struct_field_s_[i]->Accept(this);
    if (i + 1 <= node->comma_cnt_) {
      os_ << next;
      if (i + 1 == node->struct_field_s_.size() && node->comma_cnt_ == node->struct_field_s_.size()) {
        os_ << "└──,\n";
      } else {
        os_ << "├──,\n";
      }
    }
  }

  prefixes_.pop();
  is_lasts_.pop();
}

void Printer::Visit(StructNode *node) {
  os_ << prefixes_.top();
  os_ << (is_lasts_.top() ? "└──" : "├──");
  os_ << "Struct\n";

  std::string next = prefixes_.top() + (is_lasts_.top() ?  "    " : "│   ");
  os_ << next << "├──struct\n";
  prefixes_.emplace(next);
  is_lasts_.emplace(false);
  node->identifier_->Accept(this);
  if (node->semicolon_) {
    os_ << next << "└──;\n";
  } else {
    os_ << next << "├──{\n";
    if (node->struct_fields_) {
      prefixes_.emplace(next);
      is_lasts_.emplace(false);
      node->struct_fields_->Accept(this);
    }
    os_ << next << "└──}\n";
  }

  prefixes_.pop();
  is_lasts_.pop();
}

void Printer::Visit(IdentifierNode *node) {
  os_ << prefixes_.top();
  os_ << (is_lasts_.top() ? "└──" : "├──");
  os_ << "Identifier\n";

  std::string next = prefixes_.top() + (is_lasts_.top() ?  "    " : "│   ");
  os_ << next << "└──" << node->val_ << '\n';

  prefixes_.pop();
  is_lasts_.pop();
}

void Printer::Visit(CharLiteralNode *node) {
  os_ << prefixes_.top();
  os_ << (is_lasts_.top() ? "└──" : "├──");
  os_ << "Char Literal\n";

  std::string next = prefixes_.top() + (is_lasts_.top() ?  "    " : "│   ");
  os_ << next << "└──" << node->val_ << '\n';

  prefixes_.pop();
  is_lasts_.pop();
}

void Printer::Visit(StringLiteralNode *node) {
  os_ << prefixes_.top();
  os_ << (is_lasts_.top() ? "└──" : "├──");
  os_ << "String Literal\n";

  std::string next = prefixes_.top() + (is_lasts_.top() ?  "    " : "│   ");
  os_ << next << "└──" << node->val_ << '\n';

  prefixes_.pop();
  is_lasts_.pop();
}

void Printer::Visit(RawStringLiteralNode *node) {
  os_ << prefixes_.top();
  os_ << (is_lasts_.top() ? "└──" : "├──");
  os_ << "Raw String Literal\n";

  std::string next = prefixes_.top() + (is_lasts_.top() ?  "    " : "│   ");
  os_ << next << "└──" << node->val_ << '\n';

  prefixes_.pop();
  is_lasts_.pop();
}

void Printer::Visit(CStringLiteralNode *node) {
  os_ << prefixes_.top();
  os_ << (is_lasts_.top() ? "└──" : "├──");
  os_ << "C String Literal\n";

  std::string next = prefixes_.top() + (is_lasts_.top() ?  "    " : "│   ");
  os_ << next << "└──" << node->val_ << '\n';

  prefixes_.pop();
  is_lasts_.pop();
}

void Printer::Visit(RawCStringLiteralNode *node) {
  os_ << prefixes_.top();
  os_ << (is_lasts_.top() ? "└──" : "├──");
  os_ << "Raw C String Literal\n";

  std::string next = prefixes_.top() + (is_lasts_.top() ?  "    " : "│   ");
  os_ << next << "└──" << node->val_ << '\n';

  prefixes_.pop();
  is_lasts_.pop();
}

void Printer::Visit(IntegerLiteralNode *node) {
  os_ << prefixes_.top();
  os_ << (is_lasts_.top() ? "└──" : "├──");
  os_ << "Integer Literal\n";

  std::string next = prefixes_.top() + (is_lasts_.top() ?  "    " : "│   ");
  os_ << next << "└──" << node->val_ << '\n';

  prefixes_.pop();
  is_lasts_.pop();
}

void Printer::Visit(TrueNode *node) {
  os_ << prefixes_.top();
  os_ << (is_lasts_.top() ? "└──" : "├──");
  os_ << "True\n";

  std::string next = prefixes_.top() + (is_lasts_.top() ?  "    " : "│   ");
  os_ << next << "└──" << node->val_ << '\n';

  prefixes_.pop();
  is_lasts_.pop();
}

void Printer::Visit(FalseNode *node) {
  os_ << prefixes_.top();
  os_ << (is_lasts_.top() ? "└──" : "├──");
  os_ << "False\n";

  std::string next = prefixes_.top() + (is_lasts_.top() ?  "    " : "│   ");
  os_ << next << "└──" << node->val_ << '\n';

  prefixes_.pop();
  is_lasts_.pop();
}

void Printer::Visit(SuperNode *node) {
  os_ << prefixes_.top();
  os_ << (is_lasts_.top() ? "└──" : "├──");
  os_ << "Super\n";

  std::string next = prefixes_.top() + (is_lasts_.top() ?  "    " : "│   ");
  os_ << next << "└──" << node->val_ << '\n';

  prefixes_.pop();
  is_lasts_.pop();
}

void Printer::Visit(SelfLowerNode *node) {
  os_ << prefixes_.top();
  os_ << (is_lasts_.top() ? "└──" : "├──");
  os_ << "Self Lower\n";

  std::string next = prefixes_.top() + (is_lasts_.top() ?  "    " : "│   ");
  os_ << next << "└──" << node->val_ << '\n';

  prefixes_.pop();
  is_lasts_.pop();
}

void Printer::Visit(SelfUpperNode *node) {
  os_ << prefixes_.top();
  os_ << (is_lasts_.top() ? "└──" : "├──");
  os_ << "Self Upper\n";

  std::string next = prefixes_.top() + (is_lasts_.top() ?  "    " : "│   ");
  os_ << next << "└──" << node->val_ << '\n';

  prefixes_.pop();
  is_lasts_.pop();
}

void Printer::Visit(UnderscoreExpressionNode *node) {
  os_ << prefixes_.top();
  os_ << (is_lasts_.top() ? "└──" : "├──");
  os_ << "Underscore Expression\n";

  std::string next = prefixes_.top() + (is_lasts_.top() ?  "    " : "│   ");
  os_ << next << "└──" << node->val_ << '\n';

  prefixes_.pop();
  is_lasts_.pop();
}

void Printer::Visit(ContinueExpressionNode *node) {
  os_ << prefixes_.top();
  os_ << (is_lasts_.top() ? "└──" : "├──");
  os_ << "Continue Expression\n";

  std::string next = prefixes_.top() + (is_lasts_.top() ?  "    " : "│   ");
  os_ << next << "└──" << node->val_ << '\n';

  prefixes_.pop();
  is_lasts_.pop();
}

void Printer::Visit(TraitNode *node) {
  os_ << prefixes_.top();
  os_ << (is_lasts_.top() ? "└──" : "├──");
  os_ << "Trait\n";

  std::string next = prefixes_.top() + (is_lasts_.top() ?  "    " : "│   ");

  os_ << next << "├──trait\n";
  prefixes_.emplace(next);
  is_lasts_.emplace(false);
  node->identifier_->Accept(this);
  os_ << next << "├──{\n";
  for (auto &associated_item : node->asscociated_items_) {
    prefixes_.emplace(next);
    is_lasts_.emplace(false);
    associated_item->Accept(this);
  }
  os_ << next << "└──}\n";

  prefixes_.pop();
  is_lasts_.pop();
}

void Printer::Visit(ReferenceTypeNode *node) {
  os_ << prefixes_.top();
  os_ << (is_lasts_.top() ? "└──" : "├──");
  os_ << "Reference Type\n";

  std::string next = prefixes_.top() + (is_lasts_.top() ?  "    " : "│   ");
  os_ << next << "├──&\n";
  if (node->mut_) {
    os_ << next << "├──mut\n";
  }
  prefixes_.emplace(next);
  is_lasts_.emplace(true);
  node->type_no_bounds_->Accept(this);

  prefixes_.pop();
  is_lasts_.pop();
}

void Printer::Visit(ArrayTypeNode *node) {
  os_ << prefixes_.top();
  os_ << (is_lasts_.top() ? "└──" : "├──");
  os_ << "Array Type\n";

  std::string next = prefixes_.top() + (is_lasts_.top() ?  "    " : "│   ");
  os_ << next << "├──[\n";
  prefixes_.emplace(next);
  is_lasts_.emplace(false);
  node->type_->Accept(this);
  os_ << next << "├──;\n";
  prefixes_.emplace(next);
  is_lasts_.emplace(false);
  node->expr_->Accept(this);
  os_ << next << "└──]\n";

  prefixes_.pop();
  is_lasts_.pop();
}

void Printer::Visit(UnitTypeNode *node) {
  os_ << prefixes_.top();
  os_ << (is_lasts_.top() ? "└──" : "├──");
  os_ << "Unit Type\n";

  std::string next = prefixes_.top() + (is_lasts_.top() ?  "    " : "│   ");
  os_ << next << "├──(\n";
  os_ << next << "└──)\n";

  prefixes_.pop();
  is_lasts_.pop();
}

void Printer::Visit(TypeNoBoundsNode *node) {
  os_ << prefixes_.top();
  os_ << (is_lasts_.top() ? "└──" : "├──");
  os_ << "Type No Bounds\n";

  std::string next = prefixes_.top() + (is_lasts_.top() ?  "    " : "│   ");
  prefixes_.emplace(next);
  is_lasts_.emplace(true);
  if (node->type_path_ != nullptr) {
    node->type_path_->Accept(this);
  } else if (node->reference_type_ != nullptr) {
    node->reference_type_->Accept(this);
  } else if (node->array_type_ != nullptr) {
    node->array_type_->Accept(this);
  } else {
    node->unit_type_->Accept(this);
  }

  prefixes_.pop();
  is_lasts_.pop();
}
