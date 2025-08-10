#include "parser/node/enumeration.h"

EnumVariantTupleNode::EnumVariantTupleNode(const std::vector<Token> &tokens, uint32_t &pos, const uint32_t &length) : ASTNode("Enum Variant Tuple") {
  CheckLength(pos, length);
  if (tokens[pos].lexeme != "(") {
    Error("try parsing Enum Variant Tuple Node but no (");
  }
  ++pos;
  tuple_fields_ = node_pool.Make<TupleFieldsNode>(tokens, pos, length);
  CheckLength(pos, length);
  if (tokens[pos].lexeme != ")") {
    Error("try parsing Enum Variant Tuple Node but no )");
  }
  ++pos;
}

EnumVariantStructNode::EnumVariantStructNode(const std::vector<Token> &tokens, uint32_t &pos, const uint32_t &length) : ASTNode("Enum Variant Struct") {
  CheckLength(pos, length);
  if (tokens[pos].lexeme != "{") {
    Error("try parsing Enum Variant Tuple Node but no {");
  }
  ++pos;
  struct_fields_ = node_pool.Make<StructFieldsNode>(tokens, pos, length);
  CheckLength(pos, length);
  if (tokens[pos].lexeme != "}") {
    Error("try parsing Enum Variant Tuple Node but no }");
  }
  ++pos;
}

EnumVariantDiscriminantNode::EnumVariantDiscriminantNode(const std::vector<Token> &tokens, uint32_t &pos, const uint32_t &length) : ASTNode("Enum Variant Discriminant") {
  CheckLength(pos, length);
  if (tokens[pos].lexeme != "=") {
    Error("try parsing Enum Variant Discriminant Node but no =");
  }
  ++pos;
  expr_ = node_pool.Make<ExpressionNode>(tokens, pos, length);
}

EnumVariantNode::EnumVariantNode(const std::vector<Token> &tokens, uint32_t &pos, const uint32_t &length) : ASTNode("Enum Variant") {
  CheckLength(pos, length);
  if (tokens[pos].type != kIDENTIFIER_OR_KEYWORD || IsKeyword(tokens[pos].lexeme)) {
    Error("try parsing Enum Variant Node but not identifier");
  }
  identifier_or_keyword_ = node_pool.Make<IdentifierOrKeywordNode>(tokens, pos, length);
  CheckLength(pos, length);
  if (tokens[pos].lexeme == "(") {
    enum_variant_tuple_ = node_pool.Make<EnumVariantTupleNode>(tokens, pos, length);
    CheckLength(pos, length);
  } else if (tokens[pos].lexeme == "{") {
    enum_variant_struct_ = node_pool.Make<EnumVariantStructNode>(tokens, pos, length);
    CheckLength(pos, length);
  }
  if (tokens[pos].lexeme == "=") {
    enum_variant_discriminant_ = node_pool.Make<EnumVariantDiscriminantNode>(tokens, pos, length);
  }
}

EnumVariantsNode::EnumVariantsNode(const std::vector<Token> &tokens, uint32_t &pos, const uint32_t &length) : ASTNode("Enum Variants") {
  enum_variant_s_.push_back(node_pool.Make<EnumVariantNode>(tokens, pos, length));
  while (pos < length && tokens[pos].lexeme != "}") {
    if (tokens[pos].lexeme != ",") {
      Error("try parsing Enum Variants Node but not comma");
    }
    ++pos;
    ++comma_cnt_;
    CheckLength(pos, length);
    if (tokens[pos].lexeme == "}") {
      break;
    }
    enum_variant_s_.push_back(node_pool.Make<EnumVariantNode>(tokens, pos, length));
  }
  if (pos >= length) {
    Error("try parsing Enum Variants Node but no }");
  }
}

EnumerationNode::EnumerationNode(const std::vector<Token> &tokens, uint32_t &pos, const uint32_t &length) : ASTNode("Enumeration") {
  CheckLength(pos, length);
  if (tokens[pos].lexeme != "enum") {
    Error("try parsing Enumeration Node but the first token is not enum");
  }
  ++pos;
  CheckLength(pos, length);
  if (tokens[pos].type != kIDENTIFIER_OR_KEYWORD || IsKeyword(tokens[pos].lexeme)) {
    Error("try parsing Enumeration Node but not identifier");
  }
  identifier_or_keyword_ = node_pool.Make<IdentifierOrKeywordNode>(tokens, pos, length);
  CheckLength(pos, length);
  if (tokens[pos].lexeme == "<") {
    generic_params_ = node_pool.Make<GenericParamsNode>(tokens, pos, length);
    CheckLength(pos, length);
  }
  if (tokens[pos].lexeme == "where") {
    where_clause_ = node_pool.Make<WhereClauseNode>(tokens, pos, length);
    CheckLength(pos, length);
  }
  if (tokens[pos].lexeme != "{") {
    Error("try parsing Enumeration Node but no {");
  }
  ++pos;
  enum_variants_ = node_pool.Make<EnumVariantsNode>(tokens, pos, length);
  CheckLength(pos, length);
  if (tokens[pos].lexeme != "}") {
    Error("try parsing Enumeration Node but no }");
  }
  ++pos;
}