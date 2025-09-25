#include "semantic/type/type.h"
#include "common/error.h"
#include "common/config.h"
#include <iostream>

ArrayValueInfo::ArrayValueInfo(const std::vector<std::shared_ptr<ConstValue>> &type_values, const uint32_t &length) : values_(type_values), length_(length) {}

bool ExpectI32(Type *type) {
  if (type->type_ != kLeafType || type->type_name_ != "i32" && type->type_name_ != "$" && type->type_name_ != "@") {
    return false;
  }
  return true;
}

bool ExpectU32(Type *type) {
  if (type->type_ != kLeafType || type->type_name_ != "u32" && type->type_name_ != "$") {
    return false;
  }
  return true;
}

bool ExpectIsize(Type *type) {
  if (type->type_ != kLeafType || type->type_name_ != "isize" && type->type_name_ != "$" && type->type_name_ != "@") {
    return false;
  }
  return true;
}

bool ExpectUsize(Type *type) {
  if (type->type_ != kLeafType || type->type_name_ != "size" && type->type_name_ != "$") {
    return false;
  }
  return true;
}

bool ExpectStr(Type *type) {
  if (type->type_ != kPointerType || type->pointer_type_->type_ != kLeafType || type->pointer_type_->type_name_ != "str") {
    return false;
  }
  return true;
}

bool ExpectI32(ConstValue *value) {
  if (value->type_ != kLeafType || value->type_name_ != "i32" && value->type_name_ != "$" && value->type_name_ != "@") {
    return false;
  }
  return true;
}

bool ExpectU32(ConstValue *value) {
  if (value->type_ != kLeafType || value->type_name_ != "u32" && value->type_name_ != "$") {
    return false;
  }
  return true;
}

bool ExpectIsize(ConstValue *value) {
  if (value->type_ != kLeafType || value->type_name_ != "isize" && value->type_name_ != "$" && value->type_name_ != "@") {
    return false;
  }
  return true;
}

bool ExpectUsize(ConstValue *value) {
  if (value->type_ != kLeafType || value->type_name_ != "size" && value->type_name_ != "$") {
    return false;
  }
  return true;
}

bool ExpectStr(ConstValue *value) {
  if (value->type_ != kPointerType || value->pointer_info_->type_ != kLeafType || value->pointer_info_->type_name_ != "str") {
    return false;
  }
  return true;
}

std::shared_ptr<Type> ConstValue::GetType() {
  auto type = std::make_shared<Type>();
  type->type_ = type_;
  type->type_name_ = type_name_;
  if (type_ == kArrayType) {
    type->array_type_info_ = std::make_pair(array_value_info_->values_[0]->GetType(), array_value_info_->length_);
  } else if (type_ == kStructType || type_ == kEnumType) {
    type->source_ = type_source_;
  } else if (type_ == kPointerType) {
    type->pointer_mut_ = pointer_mut_;
    type->pointer_type_ = pointer_info_->GetType();
  }
  return type;
}

void TypeCast(Type *type, ConstValue *value) {
  try {
    if (type->type_ != kLeafType || value->type_ != kLeafType) {
      throw Error("SecondChecker : can't cast a non-leaf type");
    }
    if (type->type_name_ == "str" || value->type_name_ == "str") {
      throw Error("SecondChecker : can't cast a str type");
    }
    if (type->type_name_ == "char") {
      throw Error("SecondChecker : can't cast into char type");
    }
    if (type->type_name_ == "bool") {
      throw Error("SecondChecker : can't cast into bool type");
    }
    value->type_name_ = type->type_name_;
  } catch (Error &) { throw; }
}

void SameTypeCheck(Type *type, ConstValue *value) {
  try {
    if (type->type_ != value->type_) {
      throw Error("different type");
    }
    if (type->type_ == kPointerType) {
      if (type->pointer_mut_ != value->pointer_mut_) {
        throw Error("different type");
      }
      return TypeCast(type->pointer_type_.get(), value->pointer_info_.get());
    }
    if (type->type_ == kStructType || type->type_ == kEnumType) {
      if (type->source_ != value->type_source_) {
        throw Error("different type");
      }
      return;
    }
    if (type->type_ == kArrayType) {
      if (type->array_type_info_.second != value->array_value_info_->length_) {
        throw Error("different type");
      }
      for (uint32_t i = 0; i < type->array_type_info_.second; ++i) {
        SameTypeCheck(type->array_type_info_.first.get(), value->array_value_info_->values_[i].get());
      }
      return;
    }

    if (MergeLeafType(type->type_name_, value->type_name_).empty()) {
      throw Error("different type");
    }
  } catch (Error &) { throw; }
}

void SameTypeCheck(Type *type1, Type *type2) {
  try {
    if (type1->type_ == kNeverType || type2->type_ == kNeverType) {
      return;
    }
    if (type1->type_ != type2->type_) {
      throw Error("different type");
    }
    if (type1->type_ == kPointerType) {
      if (type1->pointer_mut_ != type2->pointer_mut_) {
        throw Error("different type");
      }
      SameTypeCheck(type1->pointer_type_.get(), type2->pointer_type_.get());
    }
    if (type1->type_ == kStructType || type1->type_ == kEnumType) {
      if (type1->source_ != type2->source_) {
        throw Error("different type");
      }
    }
    if (type1->type_ == kArrayType) {
      if (type1->array_type_info_.second != type2->array_type_info_.second) {
        throw Error("different type");
      }
      for (uint32_t i = 0; i < type1->array_type_info_.second; ++i) {
        SameTypeCheck(type1->array_type_info_.first.get(), type2->array_type_info_.first.get());
      }
      return;
    }
    if (MergeLeafType(type1->type_name_, type2->type_name_).empty()) {
      throw Error("different type");
    }
  } catch (Error &) { throw; }
}

std::pair<Type *, uint32_t> AutoDereference(Type *type) {
  if (type->type_ != kPointerType) {
    return std::make_pair(type, 0);
  }
  auto tmp = AutoDereference(type->pointer_type_.get());
  return std::make_pair(tmp.first, tmp.second + 1);
}