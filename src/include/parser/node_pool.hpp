#ifndef NODE_POOL_H
#define NODE_POOL_H

#include <vector>
#include "lexer/lexer.h"
#include "parser/error.h"
#include "parser/node/AST_node.h"

class NodePool {
public:
  template<class T>
  T *Make(const std::vector<Token> &tokens, uint32_t &pos, const uint32_t &length) {
    try {
      ASTNode *ptr = static_cast<T *>(operator new(sizeof(T)));
      pool_.emplace_back(ptr);
      new(ptr) T(tokens, pos, length);
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
} node_pool;

#endif //NODE_POOL_H
