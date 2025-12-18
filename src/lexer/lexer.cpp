#include "lexer/lexer.h"
#include "common/error.h"

Lexer::Lexer(const std::string &input) : input_(input), length_(input.size()) {}

auto Lexer::Run() -> std::vector<Token> const {
  uint32_t pos = 0;
  std::vector<Token> res;
  while (pos < length_) {
    if (CheckWhitespace(pos)) {
      ++pos;
    } else {
      uint32_t tmp = std::max(CheckLineComment(pos), CheckBlockComment(pos));
      if (tmp != pos) {
        pos = tmp;
      } else {
        uint32_t max_pos = pos, index = kTokenTypeCount;
        for (uint32_t i = 0; i < kTokenTypeCount; ++i) {
          tmp = (this->*check_token_func_[i])(pos);
          if (tmp > max_pos) {
            max_pos = tmp;
            index = i;
          }
        }
        if (index == kTokenTypeCount) {
          throw Error("lexer failed!");
        }
        res.push_back({kTokenTypes[index], input_.substr(pos, max_pos - pos)});
        pos = max_pos;
      }
    }
  }
  return res;
}