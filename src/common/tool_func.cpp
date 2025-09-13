#include "common/tool_func.h"

int32_t ToDigitalValue(char ch) {
  if (ch >= '0' && ch <= '9') {
    return ch - '0';
  }
  return ch - 'A' + 10;
}
