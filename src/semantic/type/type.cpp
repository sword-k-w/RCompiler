#include "semantic/type/type.h"
#include "common/error.h"

void TypeCast(TypeValue *type, TypeValue *value) {
  if (type->type_ == kPointerType || value->type_ == kPointerType) {
    throw Error("SecondChecker can't check pointer type now.");
  }
  if (type->mut_ && !value->mut_) {
    throw Error("SecondChecker : a mutable type can't be cast as mutable type");
  }
  value->mut_ = type->mut_;
  if (type->type_ == kStructType) {
    // if (value->type_ != kStructType || value->)
  }
}
