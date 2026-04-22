#include "codegen/instruction.h"
#include <iostream>

void PrintIA(std::ostream &os, const std::string &type, const std::string &rd, const std::string &rs1, int32_t imm) {
  if (imm >= -2048 && imm < 2048) {
    os << "\t" << type << "\t" << rd << ", " << rs1 << ", " << imm << '\n';
  } else {
    os << "\tli t6, " << imm << '\n';
    std::string new_type = type;
    new_type.pop_back();
    os << "\t" << new_type << "\t" << rd << ", " << rs1 << ", t6\n";
  }
}

void PrintIStar(std::ostream &os, const std::string &type, const std::string &rd, const std::string &rs1, int32_t imm) {
  if (imm >= -16 && imm < 16) {
    os << "\t" << type << "\t" << rd << ", " << rs1 << ", " << imm << '\n';
  } else {
    os << "\t li t6, " << imm << '\n';
    std::string new_type = type;
    new_type.pop_back();
    os << "\t" << new_type << "\t" << rd << ", " << rs1 << ", t6\n";
  }
}

void PrintMem(std::ostream &os, const std::string &type, const std::string &r, const std::string &rs1, int32_t imm) {
  if (imm >= -2048 && imm < 2048) {
    os << "\t" << type << "\t" << r << ", " << imm << "(" << rs1 << ")\n";
  } else {
    os << "\t li t6, " << imm << '\n';
    os << "\t add t6, t6, " << rs1 << '\n';
    os << "\t" << type << "\t" << r << ", 0(t6)\n";
  }
}
