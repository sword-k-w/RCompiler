#pragma once

#include "parser/class_declaration.h"
#include <string>
#include <cstdint>

class ASTNode {
public:
  ASTNode() = delete;
  explicit ASTNode(const std::string_view &);
  void CheckLength(const uint32_t &, const uint32_t &) const;
  // virtual void accept(Visitor *) = 0;
private:
  const std::string name_;
};
