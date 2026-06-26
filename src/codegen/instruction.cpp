#include "codegen/instruction.h"
#include <iostream>
#include <cassert>

// Helper: get the register holding the given immediate, or empty string if not cached.
static std::string GetCachedReg(int64_t imm, const ConstCache *cache) {
  if (!cache) return "";
  auto it = cache->find(imm);
  if (it != cache->end()) return it->second;
  return "";
}

void PrintIA(std::ostream &os, const std::string &type, const std::string &rd, const std::string &rs1, int32_t imm,
             const ConstCache *cache) {
  if (imm >= -2048 && imm < 2048) {
    os << "\t" << type << "\t" << rd << ", " << rs1 << ", " << imm << '\n';
  } else {
    std::string cached = GetCachedReg(imm, cache);
    std::string treg = cached.empty() ? "t6" : cached;
    if (cached.empty()) {
      os << "\tli t6, " << imm << '\n';
    }
    std::string new_type = type;
    new_type.pop_back();
    os << "\t" << new_type << "\t" << rd << ", " << rs1 << ", " << treg << "\n";
  }
}

void PrintIStar(std::ostream &os, const std::string &type, const std::string &rd, const std::string &rs1, int32_t imm,
                const ConstCache *cache) {
  if (imm >= -16 && imm < 16) {
    os << "\t" << type << "\t" << rd << ", " << rs1 << ", " << imm << '\n';
  } else {
    std::string cached = GetCachedReg(imm, cache);
    std::string treg = cached.empty() ? "t6" : cached;
    if (cached.empty()) {
      os << "\tli t6, " << imm << '\n';
    }
    std::string new_type = type;
    new_type.pop_back();
    os << "\t" << new_type << "\t" << rd << ", " << rs1 << ", " << treg << "\n";
  }
}

void PrintMem(std::ostream &os, const std::string &type, const std::string &r, const std::string &rs1, int32_t imm,
              const ConstCache *cache) {
  if (imm >= -2048 && imm < 2048) {
    os << "\t" << type << "\t" << r << ", " << imm << "(" << rs1 << ")\n";
  } else {
    std::string cached = GetCachedReg(imm, cache);
    if (cached.empty()) {
      os << "\tli t6, " << imm << '\n';
      os << "\tadd t6, t6, " << rs1 << '\n';
      os << "\t" << type << "\t" << r << ", 0(t6)\n";
    } else {
      // Use cached register as source, but write result to t6 to avoid clobbering the cache.
      os << "\tadd t6, " << cached << ", " << rs1 << '\n';
      os << "\t" << type << "\t" << r << ", 0(t6)\n";
    }
  }
}

std::pair<std::string, std::string> LoadStoreType(const std::string &type) {
  if (type == "i1") {
    return std::make_pair("lbu", "sb");
  }
  if (type == "i32") {
    return std::make_pair("lw", "sw");
  }
  assert(type == "ptr");
  return std::make_pair("ld", "sd");
}
