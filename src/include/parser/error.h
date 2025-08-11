#ifndef ERROR_H
#define ERROR_H

#include <string>
#include "parser/node_pool.hpp"

class Error {
public:
  Error() = delete;
  Error(const std::string &);
private:
  std::string_view info_;
};

#endif //ERROR_H
