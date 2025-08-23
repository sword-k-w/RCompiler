#pragma once

#include <string>
#include <cstdint>
#include <iostream>
#include <memory>

class ASTNode {
public:
  ASTNode() = delete;
  explicit ASTNode(const std::string_view &);
  void CheckLength(const uint32_t &, const uint32_t &) const;
  // virtual void accept(Visitor *) = 0;
private:
  const std::shared_ptr<std::string> name_;
};
