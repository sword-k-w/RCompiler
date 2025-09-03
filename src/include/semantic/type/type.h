#pragma once

#include <string>
#include <map>
#include <memory>
#include <vector>

enum TypeType {
  kLeafType, kArrayType, kStructType, kEnumType, kPointerType
};

struct TypeValue;

struct ArrayTypeInfo {
  std::vector<std::shared_ptr<TypeValue>> type_values_;
  uint32_t length_;
  ArrayTypeInfo() = delete;
  ArrayTypeInfo(const std::vector<std::shared_ptr<TypeValue>> &, const uint32_t &);
};

struct StructTypeInfo {
  std::map<std::string, std::shared_ptr<TypeValue>> variant_;
};

struct EnumTypeInfo {
  std::map<std::string, std::shared_ptr<std::string>> variant_;
};

struct TypeValue {
  TypeType type_;
  std::shared_ptr<uint32_t> u32_value_;
  std::shared_ptr<std::string> str_value_;
  std::shared_ptr<std::string> name_; // the name of the type
  std::shared_ptr<ArrayTypeInfo> array_type_info_;
  std::shared_ptr<StructTypeInfo> struct_type_info_;
  std::shared_ptr<EnumTypeInfo> enum_type_info_;
  bool mut_;
  std::shared_ptr<TypeValue> pointer_type_;
};

void TypeCast(TypeValue *type, TypeValue *value);

void SameTypeCheck(TypeValue *type_value1, TypeValue *type_value2);