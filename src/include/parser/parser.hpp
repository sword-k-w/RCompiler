#pragma once
#include <memory>
#include "lexer/lexer.h"
#include "common/error.h"

class Parser {
public:
  Parser() = delete;
  explicit Parser(const std::vector<Token> &tokens) : tokens_(tokens), length_(tokens.size()) {}
  template<class T>
  std::shared_ptr<T> Run() {
    uint32_t pos = 0;
    try {
      return std::make_shared<T>(tokens_, pos, length_);
    } catch (Error &err) {
      throw err;
    }
  }
private:
  std::vector<Token> tokens_;
  uint32_t length_;
};
