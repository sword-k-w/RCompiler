#ifndef MEMORY_POOL_HPP
#define MEMORY_POOL_HPP

#include <vector>
#include "common/error.h"

template <class T>
class MemoryPool {

  template<class... Args>
  T *Make(Args &&... args) {
    try {
      T *ptr = static_cast<T *>(operator new(sizeof(T)));
      pool_.emplace_back(ptr);
      new(ptr) T(std::forward<Args>(args)...);
      return ptr;
    } catch (Error &err) {
      throw err;
    }
  }

  void Clear() {
    while (!pool_.empty()) {
      T *ptr = pool_.back();
      pool_.pop_back();
      operator delete (ptr);
    }
  }

  ~MemoryPool() {
    Clear();
  }
private:
  std::vector<T *> pool_;
};

#endif