#include "parser/node/expression.h"
#include "parser/node/terminal.h"
#include "parser/node/path.h"
#include "parser/node/statement.h"
#include "parser/node/type.h"
#include "common/error.h"

LiteralExpressionNode::LiteralExpressionNode(const std::vector<Token> &tokens, uint32_t &pos, const uint32_t &length) : ASTNode("Litearal Expression") {
  try {
    CheckLength(pos, length);
    if (tokens[pos].type == kCHAR_LITERAL) {
      char_literal_ = std::make_shared<CharLiteralNode>(tokens, pos, length);
    } else if (tokens[pos].type == kSTRING_LITERAL) {
      string_literal_ = std::make_shared<StringLiteralNode>(tokens, pos, length);
    } else if (tokens[pos].type == kRAW_STRING_LITERAL) {
      raw_string_literal_ = std::make_shared<RawStringLiteralNode>(tokens, pos, length);
    } else if (tokens[pos].type == kC_STRING_LITERAL) {
      c_string_literal_ = std::make_shared<CStringLiteralNode>(tokens, pos, length);
    } else if (tokens[pos].type == kRAW_C_STRING_LITERAL) {
      raw_c_string_literal_ = std::make_shared<RawCStringLiteralNode>(tokens, pos, length);
    } else if (tokens[pos].type == kINTEGER_LITERAL) {
      integer_literal_ = std::make_shared<IntegerLiteralNode>(tokens, pos, length);
    } else if (tokens[pos].lexeme == "true") {
      true_ = std::make_shared<TrueNode>(tokens, pos, length);
    } else if (tokens[pos].lexeme == "false") {
      false_ = std::make_shared<FalseNode>(tokens, pos, length);
    } else {
      throw Error("try parsing Literal Expression Node but unexpected token");
    }
  } catch (Error &) { throw; }
}

ArrayElementsNode::ArrayElementsNode(const std::vector<Token> &tokens, uint32_t &pos, const uint32_t &length) : ASTNode("Array Elements") {
  try {
    exprs_.push_back(std::make_shared<ExpressionNode>(tokens, pos, length));
    CheckLength(pos, length);
    if (tokens[pos].lexeme == ";") {
      semicolon_ = true;
      ++pos;
      exprs_.push_back(std::make_shared<ExpressionNode>(tokens, pos, length));
    } else {
      while (pos < length && tokens[pos].lexeme != "]") {
        if (tokens[pos].lexeme != ",") {
          throw Error("try parsing Array Elements Node but not ,");
        }
        ++comma_cnt_;
        ++pos;
        CheckLength(pos, length);
        if (tokens[pos].lexeme == "]") {
          break;
        }
        exprs_.push_back(std::make_shared<ExpressionNode>(tokens, pos, length));
      }
    }
  } catch (Error &) { throw; }
}

ArrayExpressionNode::ArrayExpressionNode(const std::vector<Token> &tokens, uint32_t &pos, const uint32_t &length) : ASTNode("Array Expression") {
  try {
    CheckLength(pos, length);
    if (tokens[pos].lexeme != "[") {
      throw Error("try parsing Array Expression Node but no [");
    }
    ++pos;
    CheckLength(pos, length);
    if (tokens[pos].lexeme != "]") {
      array_elements_ = std::make_shared<ArrayElementsNode>(tokens, pos, length);
      CheckLength(pos, length);
      if (tokens[pos].lexeme != "]") {
        throw Error("try parsing Array Expression Node but no ]");
      }
    }
    ++pos;
  } catch (Error &) { throw; }
}

PathInExpressionNode::PathInExpressionNode(const std::vector<Token> &tokens, uint32_t &pos, const uint32_t &length) : ASTNode("Path In Expression"){
  try {
    path_expr_segment1_ = std::make_shared<PathExprSegmentNode>(tokens, pos, length);
    if (pos < length && tokens[pos].lexeme == "::") {
      ++pos;
      path_expr_segment2_ = std::make_shared<PathExprSegmentNode>(tokens, pos, length);
    }
  } catch (Error &) { throw; }
}

StructExprFieldNode::StructExprFieldNode(const std::vector<Token> &tokens, uint32_t &pos, const uint32_t &length) : ASTNode("Struct Expr Field") {
  try {
    identifier_ = std::make_shared<IdentifierNode>(tokens, pos, length);
    CheckLength(pos, length);
    if (tokens[pos].lexeme == ":") {
      ++pos;
      expr_ = std::make_shared<ExpressionNode>(tokens, pos, length);
    }
  } catch (Error &) { throw; }
}

StructExprFieldsNode::StructExprFieldsNode(const std::vector<Token> &tokens, uint32_t &pos, const uint32_t &length) : ASTNode("Struct Expr Fields") {
  try {
    struct_expr_field_s_.push_back(std::make_shared<StructExprFieldNode>(tokens, pos, length));
    while (pos < length && tokens[pos].lexeme != "}") {
      if (tokens[pos].lexeme != ",") {
        throw Error("try parsing Struct Expr Fields Node but not ,");
      }
      ++comma_cnt_;
      ++pos;
      if (tokens[pos].lexeme == "}") {
        break;
      }
      struct_expr_field_s_.push_back(std::make_shared<StructExprFieldNode>(tokens, pos, length));
    }
  } catch (Error &) { throw; }
}

StructExpressionNode::StructExpressionNode(const std::vector<Token> &tokens, uint32_t &pos, const uint32_t &length) : ASTNode("Struct Expression") {
  try {
    path_in_expr_ = std::make_shared<PathInExpressionNode>(tokens, pos, length);
    CheckLength(pos, length);
    if (tokens[pos].lexeme != "{") {
      throw Error("try parsing Struct Expression Node but no {");
    }
    ++pos;
    CheckLength(pos, length);
    if (tokens[pos].lexeme != "}") {
      struct_expr_fields_ = std::make_shared<StructExprFieldsNode>(tokens, pos, length);
      CheckLength(pos, length);
      if (tokens[pos].lexeme != "}") {
        throw Error("try parsing Struct Expression Node but no }");
      }
    }
    ++pos;
  } catch (Error &) { throw; }
}

ExpressionWithoutBlockNode::ExpressionWithoutBlockNode(const std::vector<Token> &tokens, uint32_t &pos, const uint32_t &length) : ASTNode("Expression Without Block") {
  try {
    CheckLength(pos, length);
    expr_ = std::make_shared<ExpressionNode>(tokens, pos, length);
    if (expr_->Type() == kExprWithBlock) {
      throw Error("try parsing Expression Without Block Node but with block");
    }
  } catch (Error &) { throw; }
}

BlockExpressionNode::BlockExpressionNode(const std::vector<Token> &tokens, uint32_t &pos, const uint32_t &length) : ASTNode("Block Expression") {
  try {
    CheckLength(pos, length);
    if (tokens[pos].lexeme != "{") {
      throw Error("try parsing Block Expression Node but no {");
    }
    ++pos;
    CheckLength(pos, length);
    if (tokens[pos].lexeme != "}") {
      statements_ = std::make_shared<StatementsNode>(tokens, pos, length);
      CheckLength(pos, length);
      if (tokens[pos].lexeme != "}") {
        throw Error("try parsing Block Expression Node but no }");
      }
    }
    ++pos;
  } catch (Error &) { throw; }
}

ConditionsNode::ConditionsNode(const std::vector<Token> &tokens, uint32_t &pos, const uint32_t &length) : ASTNode("Conditions") {
  try {
    CheckLength(pos, length);
    if (tokens[pos].lexeme != "(") {
      throw Error("try parsing Conditions Node but no (");
    }
    ++pos;
    expr_ = std::make_shared<ExpressionNode>(tokens, pos, length);
    if (expr_->Type() == kStructExpr) {
      throw Error("try parsing Conditions Node but no struct expr");
    }
    if (tokens[pos].lexeme != ")") {
      throw Error("try parsing Conditions Node but no )");
    }
    ++pos;
  } catch (Error &) { throw; }
}

InfiniteLoopExpressionNode::InfiniteLoopExpressionNode(const std::vector<Token> &tokens, uint32_t &pos, const uint32_t &length) : ASTNode("Infinite Loop Expression") {
  try {
    CheckLength(pos, length);
    if (tokens[pos].lexeme != "loop") {
      throw Error("try parsing Loop Expression Node but no loop");
    }
    ++pos;
    block_expr_ = std::make_shared<BlockExpressionNode>(tokens, pos, length);
  } catch (Error &) { throw; }
}

PredicateLoopExpressionNode::PredicateLoopExpressionNode(const std::vector<Token> &tokens, uint32_t &pos, const uint32_t &length) : ASTNode("Predicate Loop Expression") {
  try {
    CheckLength(pos, length);
    if (tokens[pos].lexeme != "while") {
      throw Error("try parsing Predicate Loop Expression Node but no while");
    }
    ++pos;
    conditions_ = std::make_shared<ConditionsNode>(tokens, pos, length);
    block_expr_ = std::make_shared<BlockExpressionNode>(tokens, pos, length);
  } catch (Error &) { throw; }
}

LoopExpressionNode::LoopExpressionNode(const std::vector<Token> &tokens, uint32_t &pos, const uint32_t &length) : ASTNode("Loop Expression") {
  try {
    CheckLength(pos, length);
    if (tokens[pos].lexeme == "loop") {
      infinite_loop_expr_ = std::make_shared<InfiniteLoopExpressionNode>(tokens, pos, length);
    } else if (tokens[pos].lexeme == "while") {
      predicate_loop_expr_ = std::make_shared<PredicateLoopExpressionNode>(tokens, pos, length);
    } else {
      throw Error("try parsing Loop Expression Node but unexpected token");
    }
  } catch (Error &) { throw; }
}

IfExpressionNode::IfExpressionNode(const std::vector<Token> &tokens, uint32_t &pos, const uint32_t &length) : ASTNode("If Expression") {
  try {
    CheckLength(pos, length);
    if (tokens[pos].lexeme != "if") {
      throw Error("try parsing If Expression Node but no if");
    }
    ++pos;
    conditions_ = std::make_shared<ConditionsNode>(tokens, pos, length);
    block_expr1_ = std::make_shared<BlockExpressionNode>(tokens, pos, length);
    CheckLength(pos, length);
    if (tokens[pos].lexeme == "else") {
      ++pos;
      CheckLength(pos, length);
      if (tokens[pos].lexeme == "{") {
        block_expr2_ = std::make_shared<BlockExpressionNode>(tokens, pos, length);
      } else if (tokens[pos].lexeme == "if") {
        if_expr_ = std::make_shared<IfExpressionNode>(tokens, pos, length);
      } else {
        throw Error("try parsing If Expression Node but unexpected token after else");
      }
    }
  } catch (Error &) { throw; }
}

ExpressionWithBlockNode::ExpressionWithBlockNode(const std::vector<Token> &tokens, uint32_t &pos, const uint32_t &length) : ASTNode("Expression With Block") {
  try {
    CheckLength(pos, length);
    if (tokens[pos].lexeme == "{") {
      block_expr_ = std::make_shared<BlockExpressionNode>(tokens, pos, length);
    } else if (tokens[pos].lexeme == "loop" || tokens[pos].lexeme == "while") {
      loop_expr_ = std::make_shared<LoopExpressionNode>(tokens, pos, length);
    } else if (tokens[pos].lexeme == "if") {
      if_expr_ = std::make_shared<IfExpressionNode>(tokens, pos, length);
    } else {
      throw Error("try parsing Expression With Block Node but unexpected token");
    }
  } catch (Error &) { throw; }
}

CallParamsNode::CallParamsNode(const std::vector<Token> &tokens, uint32_t &pos, const uint32_t &length) : ASTNode("Call Params") {
  try {
    exprs_.push_back(std::make_shared<ExpressionNode>(tokens, pos, length));
    while (pos < length && tokens[pos].lexeme != ")") {
      if (tokens[pos].lexeme != ",") {
        throw Error("try parsing Call Params Node but not ,");
      }
      ++comma_cnt_;
      ++pos;
      CheckLength(pos, length);
      if (tokens[pos].lexeme == ")") {
        break;
      }
      exprs_.push_back(std::make_shared<ExpressionNode>(tokens, pos, length));
    }
  } catch (Error &) { throw; }
}

ExpressionNode::ExpressionNode(const std::vector<Token> &tokens, uint32_t &pos, const uint32_t &length, const uint32_t &context_precedence) : ASTNode("Expression") {
  try {
    CheckLength(pos, length);
    if (tokens[pos].lexeme == "&" || tokens[pos].lexeme == "&&") {
      type_ = kBorrowExpr;
      op_ = tokens[pos].lexeme;
      ++pos;
      if (pos < length && tokens[pos].lexeme == "mut") {
        mut_ = true;
        ++pos;
      }
      expr1_ = std::make_shared<ExpressionNode>(tokens, pos, length, binding_power[{"borrow", false}].second);
    } else if (tokens[pos].lexeme == "*") {
      type_ = kDereferenceExpr;
      ++pos;
      expr1_ = std::make_shared<ExpressionNode>(tokens, pos, length, binding_power[{tokens[pos].lexeme, false}].second);
    } else if (tokens[pos].lexeme == "-" || tokens[pos].lexeme == "!") {
      type_ = kNegationExpr;
      op_ = tokens[pos].lexeme;
      ++pos;
      expr1_ = std::make_shared<ExpressionNode>(tokens, pos, length, binding_power[{tokens[pos].lexeme, false}].second);
    } else if (tokens[pos].lexeme == "return") {
      type_ = kReturnExpr;
      ++pos;
      if (pos < length && tokens[pos].lexeme == ";") {
        return;
      }
      expr1_ = std::make_shared<ExpressionNode>(tokens, pos, length, binding_power[{tokens[pos].lexeme, false}].second);
    } else if (tokens[pos].lexeme == "break") {
      type_ = kBreakExpr;
      ++pos;
      if (pos < length && tokens[pos].lexeme == ";") {
        return;
      }
      expr1_ = std::make_shared<ExpressionNode>(tokens, pos, length, binding_power[{tokens[pos].lexeme, false}].second);
    } else if (tokens[pos].lexeme == "(") {
      type_ = kGroupedExpr;
      ++pos;
      expr1_ = std::make_shared<ExpressionNode>(tokens, pos, length, binding_power[{tokens[pos].lexeme, false}].second);
      CheckLength(pos, length);
      if (tokens[pos].lexeme != ")") {
        throw Error("try parsing Grouped Expression but no )");
      }
      ++pos;
    } else if (tokens[pos].lexeme == "{" || tokens[pos].lexeme == "const" || tokens[pos].lexeme == "loop"
      || tokens[pos].lexeme == "while" || tokens[pos].lexeme == "if" || tokens[pos].lexeme == "match") {
      type_ = kExprWithBlock;
      expr_with_block_ = std::make_shared<ExpressionWithBlockNode>(tokens, pos, length);
    } else if (tokens[pos].type == kCHAR_LITERAL || tokens[pos].type == kSTRING_LITERAL || tokens[pos].type == kRAW_STRING_LITERAL ||
      tokens[pos].type == kC_STRING_LITERAL || tokens[pos].type == kRAW_C_STRING_LITERAL || tokens[pos].type == kINTEGER_LITERAL ||
      tokens[pos].lexeme == "true" || tokens[pos].lexeme == "false") {
      type_ = kLiteralExpr;
      literal_expr_ = std::make_shared<LiteralExpressionNode>(tokens, pos, length);
    } else if (tokens[pos].lexeme == "[") {
      type_ = kArrayExpr;
      array_expr_ = std::make_shared<ArrayExpressionNode>(tokens, pos, length);
    } else if (tokens[pos].type == kIDENTIFIER_OR_KEYWORD && (!IsKeyword(tokens[pos].lexeme) || tokens[pos].lexeme == "self" || tokens[pos].lexeme == "Self")) {
      uint32_t tmp = pos;
      try {
        type_ = kStructExpr;
        struct_expr_ = std::make_shared<StructExpressionNode>(tokens, pos, length);
      } catch (...) {
        struct_expr_ = nullptr;
        pos = tmp;
        type_ = kPathExpr;
        path_expr_ = std::make_shared<PathExpressionNode>(tokens, pos, length);
      }
    } else if (tokens[pos].lexeme == "continue") {
      type_ = kContinueExpr;
      continue_expr_ = std::make_shared<ContinueExpressionNode>(tokens, pos, length);
    } else if (tokens[pos].lexeme == "_") {
      type_ = kUnderscoreExpr;
      underscore_expr_ = std::make_shared<UnderscoreExpressionNode>(tokens, pos, length);
    } else {
      throw Error("try parsing Expression Node but unexpected token");
    }
    while (pos < length) {
      Token op = tokens[pos];
      auto it = infix_type.find(op.lexeme);
      if (it == infix_type.end()) {
        break;
      }
      uint32_t power = binding_power[{op.lexeme, true}].first;
      if (context_precedence == power) {
        throw Error("try parsing Expression Node but same binding power");
      }
      if (context_precedence > power) {
        break;
      }
      if (type_ == kExprWithBlock && (op.lexeme == "(" || op.lexeme == "[" || op.lexeme == "{") ) {
        break;
      }
      ++pos;
      expr1_ = std::make_shared<ExpressionNode>(*this);
      type_ = it->second;
      op_ = op.lexeme;
      if (op.lexeme == "(") {
        if (tokens[pos].lexeme != ")") {
          call_params_ = std::make_shared<CallParamsNode>(tokens, pos, length);
        }
      } else if (op.lexeme == ".") {
        if (pos + 1 < length && tokens[pos + 1].lexeme == "(") {
          type_ = kMethodCallExpr;
          path_expr_segment_ = std::make_shared<PathExprSegmentNode>(tokens, pos, length);
          ++pos;
          if (pos < length && tokens[pos].lexeme != ")") {
            call_params_ = std::make_shared<CallParamsNode>(tokens, pos, length);
          }
        } else {
          identifier_ = std::make_shared<IdentifierNode>(tokens, pos, length);
        }
      } else if (op.lexeme == "as") {
        type_no_bounds_ = std::make_shared<TypeNoBoundsNode>(tokens, pos, length);
      } else {
        expr2_ = std::make_shared<ExpressionNode>(tokens, pos, length, binding_power[{op.lexeme, true}].second);
      }
      if (type_ == kMethodCallExpr) {
        CheckLength(pos, length);
        if (tokens[pos].lexeme != ")") {
          throw Error("try parsing Method Call Expression Node but no )");
        }
        ++pos;
      } else if (type_ == kIndexExpr) {
        CheckLength(pos, length);
        if (tokens[pos].lexeme != "]") {
          throw Error("try parsing Index Expression Node but no ]");
        }
        ++pos;
      } else if (type_ == kCallExpr) {
        CheckLength(pos, length);
        if (tokens[pos].lexeme != ")") {
          throw Error("try parsing Call Expression Node but no )");
        }
        ++pos;
      }
    }
  } catch (Error &) { throw; }
}

ExpressionType ExpressionNode::Type() const {
  return type_;
}

void Print(ExpressionType type, std::ostream &os) {
  if (type == kLiteralExpr) {
    os << "Literal Expr";
  } else if (type == kPathExpr) {
    os << "Path Expr";
  } else if (type == kArrayExpr) {
    os << "Array Expr";
  } else if (type == kStructExpr) {
    os << "Struct Expr";
  } else if (type == kContinueExpr) {
    os << "Continue Expr";
  } else if (type == kUnderscoreExpr) {
    os << "Underscore Expr";
  } else if (type == kBorrowExpr) {
    os << "Borrow Expr";
  } else if (type == kDereferenceExpr) {
    os << "Dereference Expr";
  } else if (type == kNegationExpr) {
    os << "Negation Expr";
  } else if (type == kArithmeticOrLogicExpr) {
    os << "Arithmetic Or Logic Expr";
  } else if (type == kComparisonExpr) {
    os << "Comparison Expr";
  } else if (type == kLazyBooleanExpr) {
    os << "Lazy Boolean Expr";
  } else if (type == kTypeCastExpr) {
    os << "Type Cast Expr";
  } else if (type == kAssignmentExpr) {
    os << "Assignment Expr";
  } else if (type == kCompoundAssignmentExpr) {
    os << "Compound Assignment Expr";
  } else if (type == kGroupedExpr) {
    os << "Grouped Expr";
  } else if (type == kIndexExpr) {
    os << "Index Expr";
  } else if (type == kCallExpr) {
    os << "Call Expr";
  } else if (type == kMethodCallExpr) {
    os << "Method Call Expr";
  } else if (type == kFieldExpr) {
    os << "Field Expr";
  } else if (type == kBreakExpr) {
    os << "Break Expr";
  } else if (type == kReturnExpr) {
    os << "Return Expr";
  } else {
    os << "Expr With Block";
  }
}
