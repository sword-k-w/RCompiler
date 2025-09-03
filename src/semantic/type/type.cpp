#include "semantic/type/type.h"
#include "common/error.h"

ArrayTypeInfo::ArrayTypeInfo(const std::vector<std::shared_ptr<TypeValue>> &type_values, const uint32_t &length) : type_values_(type_values), length_(length) {}

void TypeCast(TypeValue *type, TypeValue *value) {
  try {
    if (type->mut_ && !value->mut_) {
      throw Error("SecondChecker : a mutable type can't be cast as mutable type");
    }
    value->mut_ = type->mut_;
    if (type->type_ == kPointerType) {
      if (value->type_ != kPointerType) {
        throw Error("SecondChecker : can't cast a non-pointer type as a pointer");
      }
      TypeCast(type->pointer_type_.get(), value->pointer_type_.get());
      return;
    }
    if (type->type_ == kStructType) {
      if (value->type_ != kStructType || *value->name_ != *type->name_) {
        throw Error("SecondChecker : can't cast a different type as a struct");
      }
      return;
    }
    if (type->type_ == kEnumType) {
      if (value->type_ != kEnumType || *value->name_ != *type->name_) {
        throw Error("SecondChecker : can't cast a different type as a enum");
      }
      return;
    }
    if (type->type_ == kArrayType) {
      if (value->type_ != kArrayType) {
        throw Error("SecondChecker : can't cast a different type as an array");
      }
      if (type->array_type_info_->length_ != value->array_type_info_->length_) {
        throw Error("SecondChecker : try cast array but length doesn't match");
      }
      for (uint32_t i = 0; i < type->array_type_info_->length_; ++i) {
        TypeCast(type->array_type_info_->type_values_[i].get(), value->array_type_info_->type_values_[i].get());
      }
      return;
    }
    if (value->type_ != kLeafType) {
      throw Error("SecondChecker : can't cast a non-leaf type as leaf type");
    }
    if (*type->name_ == "str") {
      if (*value->name_ != "str") {
        throw Error("SecondChecker : can't cast a non-str type as str type");
      }
      return;
    }
    if (*type->name_ == "char") {
      if (*value->name_ != "char") {
        throw Error("SecondChecker : can't cast a non-char type as char type");
      }
      return;
    }
    if (*type->name_ == "bool") {
      if (*value->name_ != "bool") {
        throw Error("SecondChecker : can't cast a non-bool type as bool type");
      }
      return;
    }
    value->name_ = type->name_;
  } catch (Error &) {
    throw;
  }
}

void SameTypeCheck(TypeValue *type_value1, TypeValue *type_value2) {
  try {
    if (type_value1->type_ == kPointerType) {
      if (type_value2->type_ != kPointerType) {
        throw Error("SecondChecker : different type");
      }
      TypeCast(type_value1->pointer_type_.get(), type_value2->pointer_type_.get());
      return;
    }
    if (type_value1->mut_ != type_value2->mut_) {
      throw Error("SecondChecker : different type");
    }
    if (type_value1->type_ == kStructType) {
      if (type_value2->type_ != kStructType || *type_value2->name_ != *type_value1->name_) {
        throw Error("SecondChecker : different type");
      }
      return;
    }
    if (type_value1->type_ == kEnumType) {
      if (type_value2->type_ != kEnumType || *type_value2->name_ != *type_value1->name_) {
        throw Error("SecondChecker : different type");
      }
      return;
    }
    if (type_value1->type_ == kArrayType) {
      if (type_value2->type_ != kArrayType) {
        throw Error("SecondChecker : different type");
      }
      if (type_value1->array_type_info_->length_ != type_value2->array_type_info_->length_) {
        throw Error("SecondChecker : different type");
      }
      for (uint32_t i = 0; i < type_value1->array_type_info_->length_; ++i) {
        TypeCast(type_value1->array_type_info_->type_values_[i].get(), type_value2->array_type_info_->type_values_[i].get());
      }
      return;
    }
    if (type_value2->type_ != kLeafType) {
      throw Error("SecondChecker : different type");
    }
    if (*type_value1->name_ != *type_value2->name_) {
      throw Error("SecondChecker : different type");
    }
  } catch (Error &) {
    throw;
  }
}
