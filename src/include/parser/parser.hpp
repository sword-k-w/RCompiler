#ifndef PARSER_H
#define PARSER_H

#include "lexer/lexer.h"
#include "parser/node_pool.hpp"

class Parser {
public:
  Parser() = delete;
  explicit Parser(const std::vector<Token> &tokens) : tokens_(tokens), length_(tokens.size()) {}
  template<class T>
  T *Run() {
    try {
      uint32_t pos = 0;
      return node_pool.Make<T>(tokens_, pos, length_);
    } catch (Error &err) {
      throw err;
    }
  }
private:
  std::vector<Token> tokens_;
  uint32_t length_;
};

#endif //PARSER_H
