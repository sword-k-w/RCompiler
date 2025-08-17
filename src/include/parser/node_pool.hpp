#ifndef NODE_POOL_H
#define NODE_POOL_H

#include <vector>
#include "lexer/lexer.h"
#include "parser/error.h"

class ASTNode;

class NodePool {
public:
  template<class T>
  T *Make(const std::vector<Token> &tokens, uint32_t &pos, const uint32_t &length, const uint32_t &context_precedence) {
    try {
      T *ptr = static_cast<T *>(operator new(sizeof(T)));
      pool_.emplace_back(ptr);
      new(ptr) T(tokens, pos, length, context_precedence);
      return ptr;
    } catch (Error &err) {
      throw err;
    }
  }
  template<class T>
  T *Make(const std::vector<Token> &tokens, uint32_t &pos, const uint32_t &length) {
    try {
      T *ptr = static_cast<T *>(operator new(sizeof(T)));
      pool_.emplace_back(ptr);
      new(ptr) T(tokens, pos, length);
      return ptr;
    } catch (Error &err) {
      throw err;
    }
  }
  template<class T>
  T *Make(const T &other) {
    try {
      T *ptr = static_cast<T *>(operator new(sizeof(T)));
      pool_.emplace_back(ptr);
      new(ptr) T(other);
      return ptr;
    } catch (Error &err) {
      throw err;
    }
  }
  void Clear() {
    while (!pool_.empty()) {
      ASTNode *ptr = pool_.back();
      pool_.pop_back();
      operator delete (ptr);
    }
  }
private:
  std::vector<ASTNode *> pool_;
};

inline NodePool node_pool;

#endif //NODE_POOL_H
