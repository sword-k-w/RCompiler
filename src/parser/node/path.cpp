#include "parser/node/path.h"
#include "common/error.h"
#include "parser/node_pool.h"

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
