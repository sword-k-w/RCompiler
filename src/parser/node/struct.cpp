#include "parser/node/struct.h"

StructFieldNode::StructFieldNode(const std::vector<Token> &tokens, uint32_t &pos, const uint32_t &length) : ASTNode("Struct Field") {
  CheckLength(pos, length);
  if (tokens[pos].type != kIDENTIFIER_OR_KEYWORD || IsKeyword(tokens[pos].lexeme)) {
    Error("try parsing Struct Field Node but not identifier");
  }
  identifier_or_keyword_ = node_pool.Make<IdentifierOrKeywordNode>(tokens, pos, length);
  CheckLength(pos, length);
  if (tokens[pos].lexeme != ":") {
    Error("try parsing Struct Field Node but no :");
  }
  ++pos;
  type_ = node_pool.Make<TypeNode>(tokens, pos, length);
}

StructFieldsNode::StructFieldsNode(const std::vector<Token> &tokens, uint32_t &pos, const uint32_t &length) : ASTNode("Struct Fields") {
  struct_field_s_.push_back(node_pool.Make<StructFieldNode>(tokens, pos, length));
  while (pos < length && tokens[pos].lexeme != "}") {
    if (tokens[pos].lexeme != ",") {
      Error("try parsing Struct Fields Node but not comma");
    }
    ++pos;
    ++comma_cnt_;
    CheckLength(pos, length);
    if (tokens[pos].lexeme == "}") {
      break;
    }
    struct_field_s_.push_back(node_pool.Make<StructFieldNode>(tokens, pos, length));
  }
  if (pos >= length) {
    Error("try parsing Struct Fields Node but no }");
  }
}

TupleFieldNode::TupleFieldNode(const std::vector<Token> &tokens, uint32_t &pos, const uint32_t &length) : ASTNode("Tuple Field") {
  type_ = node_pool.Make<TypeNode>(tokens, pos, length);
}

TupleFieldsNode::TupleFieldsNode(const std::vector<Token> &tokens, uint32_t &pos, const uint32_t &length) : ASTNode("Tuple Fields") {
  tuple_field_s_.push_back(node_pool.Make<TupleFieldNode>(tokens, pos, length));
  while (pos < length && tokens[pos].lexeme != ")") {
    if (tokens[pos].lexeme != ",") {
      Error("try parsing Tuple Fields Node but not comma");
    }
    ++pos;
    ++comma_cnt_;
    CheckLength(pos, length);
    if (tokens[pos].lexeme == ")") {
      break;
    }
    tuple_field_s_.push_back(node_pool.Make<TupleFieldNode>(tokens, pos, length));
  }
  if (pos >= length) {
    Error("try parsing Tuple Fields Node but no }");
  }
}

StructNode::StructNode(const std::vector<Token> &tokens, uint32_t &pos, const uint32_t &length) : ASTNode("Struct") {
  CheckLength(pos, length);
  if (tokens[pos].lexeme != "struct") {
    Error("try parsing Struct Node but the first token is not struct");
  }
  ++pos;
  CheckLength(pos, length);
  if (tokens[pos].type != kIDENTIFIER_OR_KEYWORD || IsKeyword(tokens[pos].lexeme)) {
    Error("try parsing Struct Node but not identifier");
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
    if (tokens[pos].lexeme != "{") {
      Error("try parsing StructStruct Node but not get {");
    }
    ++pos;
    CheckLength(pos, length);
    if (tokens[pos].lexeme != "}") {
      struct_fields_ = node_pool.Make<StructFieldsNode>(tokens, pos, length);
      CheckLength(pos, length);
      if (tokens[pos].lexeme != "}") {
        Error("try parsing StructStruct Node but not get }");
      }
    }
    ++pos;
  } else if (tokens[pos].lexeme == "{") {
    ++pos;
    CheckLength(pos, length);
    if (tokens[pos].lexeme != "}") {
      struct_fields_ = node_pool.Make<StructFieldsNode>(tokens, pos, length);
      CheckLength(pos, length);
      if (tokens[pos].lexeme != "}") {
        Error("try parsing StructStruct Node but not get }");
      }
    }
    ++pos;
  } else if (tokens[pos].lexeme == ";") {
    semicolon_ = true;
    ++pos;
  } else if (tokens[pos].lexeme == "(") {
    ++pos;
    CheckLength(pos, length);
    if (tokens[pos].lexeme != ")") {
      tuple_fields_ = node_pool.Make<TupleFieldsNode>(tokens, pos, length);
      CheckLength(pos, length);
      if (tokens[pos].lexeme != ")") {
        Error("try parsing TupleStruct Node but not get )");
      }
    }
    ++pos;
    CheckLength(pos, length);
    if (tokens[pos].lexeme == "<") {
      where_clause_ = node_pool.Make<WhereClauseNode>(tokens, pos, length);
    }
    CheckLength(pos, length);
    if (tokens[pos].lexeme != ";") {
      Error("try parsing TupleStruct Node but not get ;");
    }
    semicolon_ = true;
    ++pos;
  } else {
    Error("try parsing Struct Node but get unexpected token after identifier/generic params");
  }
}
