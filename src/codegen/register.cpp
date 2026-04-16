#include "codegen/register.h"

bool SameRegister(uint32_t reg_id, const std::string &reg_name) {
  return reg_name == "x" + std::to_string(reg_id) || reg_name == kRegisterName[reg_id];
}
