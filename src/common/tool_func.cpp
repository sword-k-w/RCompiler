#include "common/tool_func.h"

int32_t ToDigitalValue(char ch) {
  if (ch >= '0' && ch <= '9') {
    return ch - '0';
  }
  if (ch >= 'A' && ch <= 'F') {
    return ch - 'A' + 10;
  }
  return ch - 'a' + 10;
}
