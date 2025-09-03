#pragma once

#include "semantic/type/type.h"
#include "parser/class_declaration.h"

class Scope {
public:
  Scope() = delete;
  Scope(std::shared_ptr<Scope>, const std::string &);
  void AddTypeName(const std::string &, ASTNode *);
  void AddValueName(const std::string &, ASTNode *);
  ASTNode *FindTypeName(const std::string &);
  ASTNode *FindValueName(const std::string &);
  std::shared_ptr<std::string> name_;
private:
  std::shared_ptr<Scope> parent_;
  std::map<std::string, ASTNode *> type_namespace_; // the key of map is the identifier
  std::map<std::string, ASTNode *> value_namespace_;
};
