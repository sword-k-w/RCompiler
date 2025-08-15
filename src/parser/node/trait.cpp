#include "parser/node/trait.h"

TraitBoundNode::TraitBoundNode(const std::vector<Token> &tokens, uint32_t &pos, const uint32_t &length) : ASTNode("Trait Bound") {
  try {
    CheckLength(pos, length);
    if (tokens[pos].lexeme == "(") {
      parenthesis_ = true;
      ++pos;
      CheckLength(pos, length);
      if (tokens[pos].lexeme == "?") {
        question_ = true;
        ++pos;
      }
      type_path_ = node_pool.Make<TypePathNode>(tokens, pos, length);
      CheckLength(pos, length);
      if (tokens[pos].lexeme != ")") {
        throw Error("try parsing Trait Bound Node but no )");
      }
      ++pos;
    } else {
      if (tokens[pos].lexeme == "?") {
        question_ = true;
        ++pos;
      }
      type_path_ = node_pool.Make<TypePathNode>(tokens, pos, length);
    }
  } catch (Error &err) {
    throw err;
  }
}

UseBoundGenericArgNode::UseBoundGenericArgNode(const std::vector<Token> &tokens, uint32_t &pos, const uint32_t &length) : ASTNode("Use Bound Generic Arg") {
  try {
    CheckLength(pos, length);
    if (tokens[pos].lexeme == "Self") {
      self_upper_ = node_pool.Make<SelfUpperNode>(tokens, pos, length);
    } else {
      identifier_ = node_pool.Make<IdentifierNode>(tokens, pos, length);
    }
  } catch (Error &err) {
    throw err;
  }
}

UseBoundGenericArgsNode::UseBoundGenericArgsNode(const std::vector<Token> &tokens, uint32_t &pos, const uint32_t &length) : ASTNode("Use Bound Generic Args") {
  try {
    CheckLength(pos, length);
    if (tokens[pos].lexeme != "<") {
      throw Error("try parsing Use Bound Generic Arg Node but no <");
    }
    ++pos;
    while (pos < length && tokens[pos].lexeme != ">") {
      use_bound_generic_arg_s_.push_back(node_pool.Make<UseBoundGenericArgNode>(tokens, pos, length));
      CheckLength(pos, length);
      if (tokens[pos].lexeme == ">") {
        break;
      }
      if (tokens[pos].lexeme != ",") {
        throw Error("try parsing Use Bound Generic Arg Node but not ,");
      }
      ++comma_cnt_;
      ++pos;
    }
    if (pos >= length || tokens[pos].lexeme != ">") {
      throw Error("try parsing Use Bound Generic Arg Node but no >");
    }
    ++pos;
  } catch (Error &err) {
    throw err;
  }
}

UseBoundNode::UseBoundNode(const std::vector<Token> &tokens, uint32_t &pos, const uint32_t &length) : ASTNode("Use Bound") {
  try {
    CheckLength(pos, length);
    if (tokens[pos].lexeme != "use") {
      throw Error("try parsing Use Bound Node but no use");
    }
    use_bound_generic_args_ = node_pool.Make<UseBoundGenericArgsNode>(tokens, pos, length);
  } catch (Error &err) {
    throw err;
  }
}

TypeParamBoundNode::TypeParamBoundNode(const std::vector<Token> &tokens, uint32_t &pos, const uint32_t &length) : ASTNode("Type Param Bound") {
  try {
    uint32_t tmp = pos;
    try {
      trait_bound_ = node_pool.Make<TraitBoundNode>(tokens, pos, length);
    } catch (...) {
      trait_bound_ = nullptr;
      pos = tmp;
      use_bound_ = node_pool.Make<UseBoundNode>(tokens, pos, length);
    }
  } catch (Error &err) {
    throw err;
  }
}

TypeParamBoundsNode::TypeParamBoundsNode(const std::vector<Token> &tokens, uint32_t &pos, const uint32_t &length) : ASTNode("Type Param Bounds") {
  try {
    type_param_bound_s_.push_back(node_pool.Make<TypeParamBoundNode>(tokens, pos, length));
    while (pos < length && tokens[pos].lexeme == "+") {
      ++plus_cnt_;
      ++pos;
      uint32_t tmp = pos;
      TypeParamBoundNode *ptr;
      try {
        ptr = node_pool.Make<TypeParamBoundNode>(tokens, pos, length);
      } catch (...) {
        pos = tmp;
        break;
      }
      type_param_bound_s_.push_back(ptr);
    }
  } catch (Error &err) {
    throw err;
  }
}

TraitNode::TraitNode(const std::vector<Token> &tokens, uint32_t &pos, const uint32_t &length) : ASTNode("Trait") {
  try {
    CheckLength(pos, length);
    if (tokens[pos].lexeme != "trait") {
      throw Error("try parsing Trait Node but the first token is not trait");
    }
    ++pos;
    identifier_ = node_pool.Make<IdentifierNode>(tokens, pos, length);
    CheckLength(pos, length);
    if (tokens[pos].lexeme == ":") {
      colon_ = true;
      ++pos;
      CheckLength(pos, length);
      if (tokens[pos].lexeme != "{") {
        type_param_bounds_ = node_pool.Make<TypeParamBoundsNode>(tokens, pos, length);
        CheckLength(pos, length);
      }
    }
    if (tokens[pos].lexeme != "{") {
      throw Error("try parsing Trait Node but no {");
    }
    ++pos;
    while (pos < length && tokens[pos].lexeme != "}") {
      asscociated_items_.push_back(node_pool.Make<AsscociatedItemNode>(tokens, pos, length));
    }
    if (pos >= length || tokens[pos].lexeme != "}") {
      throw Error("try parsing Trait Node but no }");
    }
    ++pos;
  } catch (Error &err) {
    throw err;
  }
}
