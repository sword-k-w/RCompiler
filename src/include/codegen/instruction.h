#pragma once

#include <string>
#include <unordered_map>
#include <cstdint>

// Optional constant cache: maps large constant value -> register name (t3/t4).
// When provided, PrintMem/PrintIA/PrintIStar use the cached register instead
// of emitting `li t6, <imm>` every time.
using ConstCache = std::unordered_map<int64_t, std::string>;

void PrintIA(std::ostream &, const std::string &, const std::string &, const std::string &, int32_t,
             const ConstCache *cache = nullptr);

void PrintIStar(std::ostream &, const std::string &, const std::string &, const std::string &, int32_t,
                const ConstCache *cache = nullptr);

void PrintMem(std::ostream &, const std::string &, const std::string &, const std::string &, int32_t,
              const ConstCache *cache = nullptr);

std::pair<std::string, std::string> LoadStoreType(const std::string &);