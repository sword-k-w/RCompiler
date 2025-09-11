#pragma once

#include <string>
#include <map>
#include <memory>
#include <vector>
#include "parser/class_declaration.h"

enum TypeValueType {
  kLeafType, kArrayType, kStructType, kEnumType, kPointerType
};

struct ConstValue;

struct ArrayValueInfo {
  std::vector<std::shared_ptr<ConstValue>> values_;
  uint32_t length_;
  ArrayValueInfo() = delete;
  ArrayValueInfo(const std::vector<std::shared_ptr<ConstValue>> &, const uint32_t &);
};

struct StructValueInfo {
  std::map<std::string, std::shared_ptr<ConstValue>> variant_;
};

struct ConstValue {
  TypeValueType type_;
  ASTNode *type_source_; // for enumeration and struct
  uint32_t u32_value_;
  std::string str_value_;
  std::string type_name_;
  std::shared_ptr<ArrayValueInfo> array_value_info_;
  std::shared_ptr<StructValueInfo> struct_value_info_;
  bool mut_;
  std::shared_ptr<ConstValue> pointer_info_;
};

struct Type {
  TypeValueType type_;
  ASTNode *source_; // for enumeration and struct
  std::string type_name_;
  std::shared_ptr<std::pair<std::shared_ptr<Type>, uint32_t>> array_type_info_;
  bool mut_;
  std::shared_ptr<Type> pointer_type_;
};

void TypeCast(Type *, ConstValue *);

void SameTypeCheck(Type *, ConstValue *);

std::string ExpectType(Type *);