#pragma once

#include <string>
#include <map>
#include <memory>
#include <vector>
#include "parser/class_declaration.h"

// "$" is integer type including i32, u32, isize, usize
// "@" is signed integer type including i32, isize
enum TypeValueType {
  kLeafType, kArrayType, kStructType, kEnumType, kPointerType, kUnitType, kNeverType, kFunctionCallType
};

struct Type {
  TypeValueType type_;
  ASTNode *source_; // for enumeration, struct and variable
  std::string type_name_;
  bool is_mut_left_ = false;
  std::pair<std::shared_ptr<Type>, uint32_t> array_type_info_;
  bool pointer_mut_ = false;
  std::shared_ptr<Type> pointer_type_;
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
  bool mut_;
  std::shared_ptr<ArrayValueInfo> array_value_info_;
  std::shared_ptr<StructValueInfo> struct_value_info_;
  bool pointer_mut_;
  std::shared_ptr<ConstValue> pointer_info_;
  std::shared_ptr<Type> GetType();
};

bool ExpectI32(Type *);

bool ExpectU32(Type *);

bool ExpectIsize(Type *);

bool ExpectUsize(Type *);

bool ExpectI32(ConstValue *);

bool ExpectU32(ConstValue *);

bool ExpectIsize(ConstValue *);

bool ExpectUsize(ConstValue *);

void TypeCast(Type *, ConstValue *);

void SameTypeCheck(Type *, ConstValue *);

void SameTypeCheck(Type *, Type *);

std::pair<Type *, uint32_t> AutoDereference(Type *);