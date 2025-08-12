#include "parser/node/type.h"

ParenthesizedTypeNode::ParenthesizedTypeNode(const std::vector<Token> &tokens, uint32_t &pos, const uint32_t &length) : ASTNode("Parenthesized Type") {
  try {
    CheckLength(pos, length);
    if (tokens[pos].lexeme != "(") {
      throw Error("try parsing Parenthesized Type Node but no (");
    }
    ++pos;
    type_ = node_pool.Make<TypeNode>(tokens, pos, length);
    CheckLength(pos, length);
    if (tokens[pos].lexeme != ")") {
      throw Error("try parsing Parenthesized Type Node but no )");
    }
    ++pos;
  } catch (Error &err) {
    throw err;
  }
}

ImplTraitTypeOneBoundNode::ImplTraitTypeOneBoundNode(const std::vector<Token> &tokens, uint32_t &pos, const uint32_t &length) : ASTNode("Impl Trait Type One Bound") {
  try {
    CheckLength(pos, length);
    if (tokens[pos].lexeme != "impl") {
      throw Error("try parsing Impl Trait Type One Bound Node but no impl");
    }
    ++pos;
    trait_bound_ = node_pool.Make<TraitBoundNode>(tokens, pos, length);
  } catch (Error &err) {
    throw err;
  }
}

TraitObjectTypeOneBoundNode::TraitObjectTypeOneBoundNode(const std::vector<Token> &tokens, uint32_t &pos, const uint32_t &length) : ASTNode("Trait Object Type One Bound") {
  try {
    CheckLength(pos, length);
    if (tokens[pos].lexeme != "dyn") {
      throw Error("try parsing Trait Object Type One Bound Node but no dyn");
    }
    ++pos;
    trait_bound_ = node_pool.Make<TraitBoundNode>(tokens, pos, length);
  } catch (Error &err) {
    throw err;
  }
}

TupleTypeNode::TupleTypeNode(const std::vector<Token> &tokens, uint32_t &pos, const uint32_t &length) : ASTNode("Tuple Type") {
  try {
    CheckLength(pos, length);
    if (tokens[pos].lexeme != "(") {
      throw Error("try parsing Tuple Type Node but no (");
    }
    ++pos;
    while (pos < length && tokens[pos].lexeme != ")") {
      types_.push_back(node_pool.Make<TypeNode>(tokens, pos, length));
      CheckLength(pos, length);
      if (tokens[pos].lexeme == ")") {
        break;
      }
      if (tokens[pos].lexeme != ",") {
        throw Error("try parsing Tuple Type Node but not ,");
      }
      ++comma_cnt_;
      ++pos;
    }
    if (pos >= length || tokens[pos].lexeme != ")") {
      throw Error("try parsing Tuple Type Node but no )");
    }
    if (!comma_cnt_ && types_.size() == 1) {
      throw Error("try parsing Tuple Type Node but get Parenthesized Type");
    }
    ++pos;
  } catch (Error &err) {
    throw err;
  }
}

RawPointerTypeNode::RawPointerTypeNode(const std::vector<Token> &tokens, uint32_t &pos, const uint32_t &length) : ASTNode("Raw Pointer Type") {
  try {
    CheckLength(pos, length);
    if (tokens[pos].lexeme != "*") {
      throw Error("try parsing Raw Pointer Type Node but no *");
    }
    ++pos;
    CheckLength(pos, length);
    if (tokens[pos].lexeme == "mut") {
      mut_ = true;
    } else if (tokens[pos].lexeme != "const") {
      throw Error("try parsing Raw Pointer Type Node but no mut or const");
    }
    ++pos;
    type_no_bounds_ = node_pool.Make<TypeNoBoundsNode>(tokens, pos, length);
  } catch (Error &err) {
    throw err;
  }
}

ReferenceTypeNode::ReferenceTypeNode(const std::vector<Token> &tokens, uint32_t &pos, const uint32_t &length) : ASTNode("Reference Type") {
  try {
    CheckLength(pos, length);
    if (tokens[pos].lexeme != "&") {
      throw Error("try parsing Reference Type Node but no &");
    }
    ++pos;
    CheckLength(pos, length);
    if (tokens[pos].lexeme == "mut") {
      mut_ = true;
      ++pos;
    }
    type_no_bounds_ = node_pool.Make<TypeNoBoundsNode>(tokens, pos, length);
  } catch (Error &err) {
    throw err;
  }
}

ArrayTypeNode::ArrayTypeNode(const std::vector<Token> &tokens, uint32_t &pos, const uint32_t &length) : ASTNode("Array Type") {
  try {
    CheckLength(pos, length);
    if (tokens[pos].lexeme != "[") {
      throw Error("try parsing Array Type Node but no [");
    }
    ++pos;
    type_ = node_pool.Make<TypeNode>(tokens, pos, length);
    CheckLength(pos, length);
    if (tokens[pos].lexeme != ";") {
      throw Error("try parsing Array Type Node but no ;");
    }
    ++pos;
    expr_ = node_pool.Make<ExpressionNode>(tokens, pos, length);
    CheckLength(pos, length);
    if (tokens[pos].lexeme != "]") {
      throw Error("try parsing Array Type Node but no ]");
    }
    ++pos;
  } catch (Error &err) {
    throw err;
  }
}

SliceTypeNode::SliceTypeNode(const std::vector<Token> &tokens, uint32_t &pos, const uint32_t &length) : ASTNode("Slice Type") {
  try {
    CheckLength(pos, length);
    if (tokens[pos].lexeme != "[") {
      throw Error("try parsing Slice Type Node but no [");
    }
    ++pos;
    type_ = node_pool.Make<TypeNode>(tokens, pos, length);
    CheckLength(pos, length);
    if (tokens[pos].lexeme != "]") {
      throw Error("try parsing Slice Type Node but no ]");
    }
    ++pos;
  } catch (Error &err) {
    throw err;
  }
}

TypeNoBoundsNode::TypeNoBoundsNode(const std::vector<Token> &tokens, uint32_t &pos, const uint32_t &length) : ASTNode("Type No Bounds") {
  try {
    CheckLength(pos, length);
    if (tokens[pos].lexeme == "!") {
      never_type_ = node_pool.Make<NeverTypeNode>(tokens, pos, length);
    } else if (tokens[pos].lexeme == "_") {
      inferred_type_ = node_pool.Make<InferredTypeNode>(tokens, pos, length);
    } else if (tokens[pos].lexeme == "impl") {
      impl_trait_type_one_bound_ = node_pool.Make<ImplTraitTypeOneBoundNode>(tokens, pos, length);
    } else if (tokens[pos].lexeme == "dyn") {
      trait_object_type_one_bound_ = node_pool.Make<TraitObjectTypeOneBoundNode>(tokens, pos, length);
    } else if (tokens[pos].lexeme == "*") {
      raw_pointer_type_ = node_pool.Make<RawPointerTypeNode>(tokens, pos, length);
    } else if (tokens[pos].lexeme == "&") {
      reference_type_ = node_pool.Make<ReferenceTypeNode>(tokens, pos, length);
    } else if (tokens[pos].lexeme == "<") {
      qualified_path_in_type_ = node_pool.Make<QualifiedPathInTypeNode>(tokens, pos, length);
    } else if (tokens[pos].lexeme == "fn") {
      bare_function_type_ = node_pool.Make<BareFunctionTypeNode>(tokens, pos, length);
    } else if (tokens[pos].lexeme == "[") {
      uint32_t tmp = pos;
      try {
        array_type_ = node_pool.Make<ArrayTypeNode>(tokens, pos, length);
      } catch (...) {
        array_type_ = nullptr;
        pos = tmp;
        slice_type_ = node_pool.Make<SliceTypeNode>(tokens, pos, length);
      }
    } else if (tokens[pos].lexeme == "(") {
      uint32_t tmp = pos;
      try {
        parenthesized_type_ = node_pool.Make<ParenthesizedTypeNode>(tokens, pos, length);
      } catch (...) {
        parenthesized_type_ = nullptr;
        pos = tmp;
        tuple_type_ = node_pool.Make<TupleTypeNode>(tokens, pos, length);
      }
    } else {
      type_path_ = node_pool.Make<TypePathNode>(tokens, pos, length);
    }
  } catch (Error &err) {
    throw err;
  }
}

ImplTraitTypeNode::ImplTraitTypeNode(const std::vector<Token> &tokens, uint32_t &pos, const uint32_t &length) : ASTNode("Impl Trait Type") {
  try {
    CheckLength(pos, length);
    if (tokens[pos].lexeme != "impl") {
      throw Error("try parsing Impl Trait Type Node but no impl");
    }
    ++pos;
    type_param_bounds_ = node_pool.Make<TypeParamBoundsNode>(tokens, pos, length);
  } catch (Error &err) {
    throw err;
  }
}

TraitObjectTypeNode::TraitObjectTypeNode(const std::vector<Token> &tokens, uint32_t &pos, const uint32_t &length) : ASTNode("Trait Object Type") {
  try {
    CheckLength(pos, length);
    if (tokens[pos].lexeme != "dyn") {
      throw Error("try parsing Trait Object Type Node but no dyn");
    }
    ++pos;
    type_param_bounds_ = node_pool.Make<TypeParamBoundsNode>(tokens, pos, length);
  } catch (Error &err) {
    throw err;
  }
}

TypeNode::TypeNode(const std::vector<Token> &tokens, uint32_t &pos, const uint32_t &length) : ASTNode("Type") {
  try {
    uint32_t tmp = pos;
    try {
      type_no_bounds_ = node_pool.Make<TypeNoBoundsNode>(tokens, pos, length);
    } catch (...) {
      pos = tmp;
      type_no_bounds_ = nullptr;
      try {
        impl_trait_type_ = node_pool.Make<ImplTraitTypeNode>(tokens, pos, length);
      } catch (...) {
        pos = tmp;
        impl_trait_type_ = nullptr;
        trait_object_type_ = node_pool.Make<TraitObjectTypeNode>(tokens, pos, length);
      }
    }
  } catch (Error &err) {
    throw err;
  }
}
