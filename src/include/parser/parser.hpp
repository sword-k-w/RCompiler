#pragma once

#include "lexer/lexer.h"
#include "parser/node_pool.h"
#include "common/error.h"

class Parser {
public:
  Parser() = delete;
  explicit Parser(const std::vector<Token> &tokens) : tokens_(tokens), length_(tokens.size()) {}
  template<class T>
  T *Run() {
    uint32_t pos = 0;
    try {
      return node_pool.Make<T>(tokens_, pos, length_);
    } catch (Error &err) {
      throw err;
    }
  }
private:
  std::vector<Token> tokens_;
  uint32_t length_;
};
