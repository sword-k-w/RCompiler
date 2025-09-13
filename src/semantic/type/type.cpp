#include "semantic/type/type.h"
#include "common/error.h"

ArrayValueInfo::ArrayValueInfo(const std::vector<std::shared_ptr<ConstValue>> &type_values, const uint32_t &length) : values_(type_values), length_(length) {}

void TypeCast(Type *type, ConstValue *value) {
  try {
    if (type->mut_ && !value->mut_) {
      throw Error("SecondChecker : a non-mutable value can't be cast as mutable type");
    }
    value->mut_ = type->mut_;
    if (type->type_ == kPointerType) {
      if (value->type_ != kPointerType) {
        throw Error("SecondChecker : can't cast a non-pointer type as a pointer");
      }
      TypeCast(type->pointer_type_.get(), value->pointer_info_.get());
      return;
    }
    if (type->type_ == kStructType) {
      if (value->type_ != kStructType || value->type_source_ != type->source_) {
        throw Error("SecondChecker : can't cast a different type as a struct");
      }
      return;
    }
    if (type->type_ == kEnumType) {
      if (value->type_ != kEnumType || value->type_source_ != type->source_) {
        throw Error("SecondChecker : can't cast a different type as a enum");
      }
      return;
    }
    if (type->type_ == kArrayType) {
      if (value->type_ != kArrayType) {
        throw Error("SecondChecker : can't cast a different type as an array");
      }
      if (type->array_type_info_.second != value->array_value_info_->length_) {
        throw Error("SecondChecker : try cast array but length doesn't match");
      }
      for (uint32_t i = 0; i < type->array_type_info_.second; ++i) {
        TypeCast(type->array_type_info_.first.get(), value->array_value_info_->values_[i].get());
      }
      return;
    }
    if (value->type_ != kLeafType) {
      throw Error("SecondChecker : can't cast a non-leaf type as leaf type");
    }
    if (type->type_name_ == "str") {
      if (value->type_name_ != "str") {
        throw Error("SecondChecker : can't cast a non-str type as str type");
      }
      return;
    }
    if (type->type_name_ == "char") {
      if (value->type_name_ != "char") {
        throw Error("SecondChecker : can't cast a non-char type as char type");
      }
      return;
    }
    if (type->type_name_ == "bool") {
      if (value->type_name_ != "bool") {
        throw Error("SecondChecker : can't cast a non-bool type as bool type");
      }
      return;
    }
    value->type_name_ = type->type_name_;
  } catch (Error &) { throw; }
}

void SameTypeCheck(Type *type, ConstValue *value) {
  try {
    if (type->mut_ != value->mut_ || type->type_ != value->type_) {
      throw Error("SecondChecker : different type");
    }
    if (type->type_ == kPointerType) {
      TypeCast(type->pointer_type_.get(), value->pointer_info_.get());
      return;
    }
    if (type->type_ == kStructType) {
      if (type->source_ != value->type_source_) {
        throw Error("SecondChecker : different type");
      }
      return;
    }
    if (type->type_ == kEnumType) {
      if (type->source_ != value->type_source_) {
        throw Error("SecondChecker : different type");
      }
      return;
    }
    if (type->type_ == kArrayType) {
      if (type->array_type_info_.second != value->array_value_info_->length_) {
        throw Error("SecondChecker : different type");
      }
      for (uint32_t i = 0; i < type->array_type_info_.second; ++i) {
        SameTypeCheck(type->array_type_info_.first.get(), value->array_value_info_->values_[i].get());
      }
      return;
    }
    if (type->type_name_ != value->type_name_) {
      throw Error("SecondChecker : different type");
    }
  } catch (Error &) { throw; }
}

std::string ExpectType(Type *type) {
  if (type->type_ == kLeafType) {
    return type->type_name_;
  }
  if (type->type_ == kArrayType) {
    return ExpectType(type->array_type_info_.first.get());
  }
  return nullptr;
}