#pragma once

#include <vector>
#include "lexer/lexer.h"
#include "parser/class_declaration.h"

class ASTNode;

class NodePool {
public:
  template<class T>
  T *Make(const std::vector<Token> &tokens, uint32_t &pos, const uint32_t &length, const uint32_t &context_precedence);

  template<class T>
  T *Make(const std::vector<Token> &tokens, uint32_t &pos, const uint32_t &length);

  template<class T>
  T *Make(const T &other);

  void Clear();

  ~NodePool() {
    Clear();
  }
private:
  std::vector<ASTNode *> pool_;
};

inline NodePool node_pool;