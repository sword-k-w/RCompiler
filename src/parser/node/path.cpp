#include "parser/node/path.h"
#include "common/error.h"
#include "parser/node/terminal.h"

PathIdentSegmentNode::PathIdentSegmentNode(const std::vector<Token> &tokens, uint32_t &pos, const uint32_t &length) : ASTNode("Path Indent Segment") {
  try {
    CheckLength(pos, length);
    if (tokens[pos].type != kIDENTIFIER_OR_KEYWORD) {
      throw Error("try parsing Path Indent Segment Node but not identifier or keyword");
    }
    if (IsKeyword(tokens[pos].lexeme)) {
      if (tokens[pos].lexeme == "self") {
        self_lower_ = std::make_shared<SelfLowerNode>(tokens, pos, length);
      } else if (tokens[pos].lexeme == "Self") {
        self_upper_ = std::make_shared<SelfUpperNode>(tokens, pos, length);
      } else {
        throw Error("try parsing Path Indent Segment Node but unexpected keyword");
      }
    } else {
      identifier_ = std::make_shared<IdentifierNode>(tokens, pos, length);
    }
  } catch (Error &) { throw; }
}
