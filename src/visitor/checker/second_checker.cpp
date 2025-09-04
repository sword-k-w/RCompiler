#include "visitor/checker/second_checker.h"

#include <parser/node/crate.h>
#include <parser/node/enumeration.h>
#include <parser/node/expression.h>
#include <parser/node/function.h>
#include <parser/node/implementation.h>
#include <parser/node/item.h>
#include <parser/node/path.h>
#include <parser/node/statement.h>
#include <parser/node/struct.h>
#include <parser/node/terminal.h>
#include <parser/node/trait.h>
#include <parser/node/type.h>

#include "common/error.h"

void SecondChecker::Visit(CrateNode *node) {
  try {
    for (auto &item : node->items_) {
      item->Accept(this);
    }
  } catch (Error &) {
    throw;
  }
}

void SecondChecker::Visit(EnumVariantsNode *node) {}

void SecondChecker::Visit(EnumerationNode *node) {}

void SecondChecker::Visit(LiteralExpressionNode *node) {}

void SecondChecker::Visit(ArrayElementsNode *node) {}

void SecondChecker::Visit(ArrayExpressionNode *node) {}

void SecondChecker::Visit(PathInExpressionNode *node) {}

void SecondChecker::Visit(StructExprFieldNode *node) {}

void SecondChecker::Visit(StructExprFieldsNode *node) {}

void SecondChecker::Visit(StructExpressionNode *node) {}

void SecondChecker::Visit(ExpressionWithoutBlockNode *node) {}

void SecondChecker::Visit(BlockExpressionNode *node) {
  try {
    if (node->statements_ != nullptr) {
      node->statements_->Accept(this);
    }
  } catch (Error &) {
    throw;
  }
}

void SecondChecker::Visit(InfiniteLoopExpressionNode *node) {
  try {
    node->block_expr_->Accept(this);
  } catch (Error &) {
    throw;
  }
}

void SecondChecker::Visit(ConditionsNode *node) {}

void SecondChecker::Visit(PredicateLoopExpressionNode *node) {
  try {
    node->block_expr_->Accept(this);
  } catch (Error &) {
    throw;
  }
}

void SecondChecker::Visit(LoopExpressionNode *node) {
  try {
    if (node->infinite_loop_expr_ != nullptr) {
      node->infinite_loop_expr_->Accept(this);
    } else {
      node->predicate_loop_expr_->Accept(this);
    }
  } catch (Error &) {
    throw;
  }
}

void SecondChecker::Visit(IfExpressionNode *node) {
  try {
    node->block_expr1_->Accept(this);
    if (node->block_expr2_ != nullptr) {
      node->block_expr2_->Accept(this);
    } else if (node->if_expr_ != nullptr) {
      node->if_expr_->Accept(this);
    }
  } catch (Error &) {
    throw;
  }
}

void SecondChecker::Visit(ExpressionWithBlockNode *node) {
  try {
    if (node->block_expr_ != nullptr) {
      node->block_expr_->Accept(this);
    } else if (node->if_expr_ != nullptr) {
      node->if_expr_->Accept(this);
    } else {
      node->loop_expr_->Accept(this);
    }
  } catch (Error &) {
    throw;
  }
}

void SecondChecker::Visit(CallParamsNode *node) {}

void SecondChecker::Visit(ExpressionNode *node) {
  try {
    if (node->type_ == kBorrowExpr || node->type_ == kDereferenceExpr || node->type_ == kNegationExpr
      || node->type_ == kGroupedExpr || node->type_ == kBreakExpr || node->type_ == kReturnExpr
      || node->type_ == kTypeCastExpr || node->type_ == kCallExpr || node->type_ == kMethodCallExpr
      || node->type_ == kFieldExpr) {
      node->expr1_->Accept(this);
    } else if (node->type_ == kArithmeticOrLogicExpr || node->type_ == kComparisonExpr || node->type_ == kLazyBooleanExpr
      || node->type_ == kAssignmentExpr || node->type_ == kCompoundAssignmentExpr || node->type_ == kIndexExpr) {
      node->expr1_->Accept(this);
      node->expr2_->Accept(this);
    } else if (node->type_ == kExprWithBlock) {
      node->expr_with_block_->Accept(this);
    }
  } catch (Error &) {
    throw;
  }
}

void SecondChecker::Visit(ShorthandSelfNode *node) {}

void SecondChecker::Visit(TypedSelfNode *node) {}

void SecondChecker::Visit(SelfParamNode *node) {}

void SecondChecker::Visit(FunctionParamNode *node) {}

void SecondChecker::Visit(FunctionParametersNode *node) {}

void SecondChecker::Visit(FunctionReturnTypeNode *node) {}

void SecondChecker::Visit(FunctionNode *node) {
  try {
    if (node->block_expr_ != nullptr) {
      node->block_expr_->Accept(this);
    }
  } catch (Error &) {
    throw;
  }
}

void SecondChecker::Visit(ImplementationNode *node) {
  try {
    if (node->type_->type_path_ == nullptr || node->type_->type_path_->identifier_ == nullptr) {
      throw Error("SecondChecker : can't impl the type that isn't struct");
    }
    ASTNode *target = node->scope_->FindTypeName(*node->type_->type_path_->identifier_->val_);
    if (target == nullptr) {
      throw Error("SecondChecker : impl the type that is not found");
    }
    StructNode *struct_node = dynamic_cast<StructNode *>(target);
    if (struct_node == nullptr) {
      throw Error("SecondChecker : can't impl the type that isn't struct");
    }
    for (auto &associated_item : node->associated_items_) {
      associated_item->Accept(this);
      if (associated_item->function_ != nullptr) {
        if (!struct_node->impl_.emplace(*associated_item->function_->identifier_->val_, associated_item->function_.get()).second) {
          throw Error("SecondChecker : repeat method name for the same struct");
        }
      } else {
        if (!struct_node->impl_.emplace(*associated_item->constant_item_->identifier_->val_, associated_item->constant_item_.get()).second) {
          throw Error("SecondChecker : repeat const name for the same struct");
        }
      }
    }
  } catch (Error &) {
    throw;
  }
}

void SecondChecker::Visit(ConstantItemNode *node) {
  try {
    if (node->expr_ != nullptr) {
      node->expr_->Accept(this);
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

void SecondChecker::Visit(LetStatementNode *node) {
  try {
    if (node->expr_ != nullptr) {
      node->expr_->Accept(this);
    }
  } catch (Error &) {
    throw;
  }
}

void SecondChecker::Visit(ExpressionStatementNode *node) {
  try {
    node->expr_->Accept(this);
  } catch (Error &) {
    throw;
  }
}

void SecondChecker::Visit(StatementNode *node) {
  try {
    if (node->item_ != nullptr) {
      node->item_->Accept(this);
    } else if (node->let_statement_ != nullptr) {
      node->let_statement_->Accept(this);
    } else if (node->expr_statement_ != nullptr) {
      node->expr_statement_->Accept(this);
    }
  } catch (Error &) {
    throw;
  }
}

void SecondChecker::Visit(StatementsNode *node) {
  try {
    for (auto &statement : node->statement_s_) {
      statement->Accept(this);
    }
    if (node->expr_without_block_ != nullptr) {
      node->expr_without_block_->Accept(this);
    }
  } catch (Error &) {
    throw;
  }
}

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

void SecondChecker::Visit(SelfLowerNode *node) {}

void SecondChecker::Visit(SelfUpperNode *node) {}

void SecondChecker::Visit(UnderscoreExpressionNode *node) {}

void SecondChecker::Visit(ContinueExpressionNode *node) {}

void SecondChecker::Visit(TraitNode *node) {
  try {
    for (auto &associated_item : node->asscociated_items_) {
      associated_item->Accept(this);
    }
  } catch (Error &) {
    throw;
  }
}

void SecondChecker::Visit(ReferenceTypeNode *node) {}

void SecondChecker::Visit(ArrayTypeNode *node) {}

void SecondChecker::Visit(UnitTypeNode *node) {}

void SecondChecker::Visit(TypeNoBoundsNode *) {}
