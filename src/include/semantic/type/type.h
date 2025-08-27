#pragma once

#include <string>
#include <map>
#include <memory>

enum TypeType {
  kLeafType, kArrayType, kStructType, kEnumType, kPointerType
};

struct Type;

struct ArrayTypeInfo {
  std::shared_ptr<Type> type_;
  uint32_t length_;
};

struct StructTypeInfo {
  std::map<std::string, std::shared_ptr<Type>> variant_;
};

struct EnumTypeInfo {
  std::map<std::string, std::shared_ptr<std::string>> variant_;
};

struct Type {
  TypeType type_;
  std::shared_ptr<std::string> name_;
  std::shared_ptr<ArrayTypeInfo> array_type_info_;
  std::shared_ptr<StructTypeInfo> struct_type_info_;
  std::shared_ptr<EnumTypeInfo> enum_type_info_;
  bool mut_;
  std::shared_ptr<Type> pointer_type_;
};
