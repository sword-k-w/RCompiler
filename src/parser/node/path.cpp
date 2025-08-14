#include "parser/node/path.h"

PathIdentSegmentNode::PathIdentSegmentNode(const std::vector<Token> &tokens, uint32_t &pos, const uint32_t &length) : ASTNode("Path Indent Segment") {
  try {
    CheckLength(pos, length);
    if (tokens[pos].type != kIDENTIFIER_OR_KEYWORD) {
      throw Error("try parsing Path Indent Segment Node but not identifier or keyword");
    }
    if (IsKeyword(tokens[pos].lexeme)) {
      if (tokens[pos].lexeme == "self") {
        self_lower_ = node_pool.Make<SelfLowerNode>(tokens, pos, length);
      } else if (tokens[pos].lexeme == "Self") {
        self_upper_ = node_pool.Make<SelfUpperNode>(tokens, pos, length);
      } else {
        throw Error("try parsing Path Indent Segment Node but unexpected keyword");
      }
    } else {
      identifier_ = node_pool.Make<IdentifierNode>(tokens, pos, length);
    }
  } catch (Error &err) {
    throw err;
  }
}

TypePathFnInputsNode::TypePathFnInputsNode(const std::vector<Token> &tokens, uint32_t &pos, const uint32_t &length) : ASTNode("Type Path Fn Inputs") {
  try {
    types_.push_back(node_pool.Make<TypeNode>(tokens, pos, length));
    while (pos < length && tokens[pos].lexeme != ")") {
      if (tokens[pos].lexeme != ",") {
        throw Error("try parsing Type Path Fn Inputs Node but not ,");
      }
      ++pos;
      ++comma_cnt_;
      CheckLength(pos, length);
      if (tokens[pos].lexeme != ")") {
        types_.push_back(node_pool.Make<TypeNode>(tokens, pos, length));
      }
    }
  } catch (Error &err) {
    throw err;
  }
}

TypePathFnNode::TypePathFnNode(const std::vector<Token> &tokens, uint32_t &pos, const uint32_t &length) : ASTNode("Type Path Fn") {
  try {
    CheckLength(pos, length);
    if (tokens[pos].lexeme != "(") {
      throw Error("try parsing Type Path Fn Node but no (");
    }
    ++pos;
    CheckLength(pos, length);
    if (tokens[pos].lexeme != ")") {
      type_path_fn_inputs_ = node_pool.Make<TypePathFnInputsNode>(tokens, pos, length);
      CheckLength(pos, length);
      if (tokens[pos].lexeme != ")") {
        throw Error("try parsing Type Path Fn Node but no )");
      }
    }
    ++pos;
    CheckLength(pos, length);
    if (tokens[pos].lexeme == "->") {
      ++pos;
      type_no_bounds_ = node_pool.Make<TypeNoBoundsNode>(tokens, pos, length);
    }
  } catch (Error &err) {
    throw err;
  }
}

TypePathSegmentNode::TypePathSegmentNode(const std::vector<Token> &tokens, uint32_t &pos, const uint32_t &length) : ASTNode("Type Path Segment") {
  try {
    path_ident_segment_ = node_pool.Make<PathIdentSegmentNode>(tokens, pos, length);
    CheckLength(pos, length);
    if (tokens[pos].lexeme == "<") {
      generic_args_ = node_pool.Make<GenericArgsNode>(tokens, pos, length);
    } else if (tokens[pos].lexeme == "(") {
      type_path_fn_ = node_pool.Make<TypePathFnNode>(tokens, pos, length);
    } else if (tokens[pos].lexeme == "::" && pos + 1 < length) {
      if (tokens[pos + 1].lexeme == "<") {
        colon_ = true;
        ++pos;
        generic_args_ = node_pool.Make<GenericArgsNode>(tokens, pos, length);
      } else if (tokens[pos + 1].lexeme == "(") {
        colon_ = true;
        ++pos;
        type_path_fn_ = node_pool.Make<TypePathFnNode>(tokens, pos, length);
      }
    }
  } catch (Error &err) {
    throw err;
  }
}

QualifiedPathTypeNode::QualifiedPathTypeNode(const std::vector<Token> &tokens, uint32_t &pos, const uint32_t &length) : ASTNode("Qualified Path Type") {
  try {
    CheckLength(pos, length);
    if (tokens[pos].lexeme != "<") {
      throw Error("try parsing Qualified Path Type Node but no <");
    }
    ++pos;
    type_ = node_pool.Make<TypeNode>(tokens, pos, length);
    CheckLength(pos, length);
    if (tokens[pos].lexeme != ">") {
      if (tokens[pos].lexeme != "as") {
        throw Error("try parsing Qualified Path Type Node but no as");
      }
      ++pos;
      type_path_ = node_pool.Make<TypePathNode>(tokens, pos, length);
      if (tokens[pos].lexeme != ">") {
        throw Error("try parsing Qualified Path Type Node but no >");
      }
    }
    ++pos;
  } catch (Error &err) {
    throw err;
  }
}

QualifiedPathInTypeNode::QualifiedPathInTypeNode(const std::vector<Token> &tokens, uint32_t &pos, const uint32_t &length) : ASTNode("Qualified Path In Type") {
  try {
    qualified_path_type_ = node_pool.Make<QualifiedPathTypeNode>(tokens, pos, length);
    while (pos < length && tokens[pos].lexeme == "::") {
      ++pos;
      CheckLength(pos, length);
      type_path_segments_.push_back(node_pool.Make<TypePathSegmentNode>(tokens, pos, length));
    }
    if (type_path_segments_.empty()) {
      throw Error("try parsing Qualified Path In Type Node but no TypePathSegment");
    }
  } catch (Error &err) {
    throw err;
  }
}
