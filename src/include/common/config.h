#ifndef CONFIG_H
#define CONFIG_H

#include <string>

constexpr uint32_t kTokenTypeCount = 13;

// strict keywords
constexpr std::string_view kAS = "as";
constexpr std::string_view kBREAK = "break";
constexpr std::string_view kCONST = "const";
constexpr std::string_view kCONTINUE = "continue";
constexpr std::string_view kCRATE = "crate";
constexpr std::string_view kELSE = "else";
constexpr std::string_view kENUM = "enum";
constexpr std::string_view kEXTERN = "extern";
constexpr std::string_view kFALSE = "false";
constexpr std::string_view kFN = "fn";
constexpr std::string_view kFOR = "for";
constexpr std::string_view kIF = "if";
constexpr std::string_view kIMPL = "impl";
constexpr std::string_view kIN = "in";
constexpr std::string_view kLET = "let";
constexpr std::string_view kLOOP = "loop";
constexpr std::string_view kMATCH = "match";
constexpr std::string_view kMOD = "mod";
constexpr std::string_view kMOVE = "move";
constexpr std::string_view kMUT = "mut";
constexpr std::string_view kPUB = "pub";
constexpr std::string_view kREF = "ref";
constexpr std::string_view kRETURN = "return";
constexpr std::string_view kSELF_LOWER = "self";
constexpr std::string_view kSELF_UPPER = "Self";
constexpr std::string_view kSTATIC = "static";
constexpr std::string_view kSTRUCT = "struct";
constexpr std::string_view kSUPER = "super";
constexpr std::string_view kTRAIT = "trait";
constexpr std::string_view kTRUE = "true";
constexpr std::string_view kTYPE = "type";
constexpr std::string_view kUNSAFE = "unsafe";
constexpr std::string_view kUSE = "use";
constexpr std::string_view kWHERE = "where";
constexpr std::string_view kWHILE = "while";
constexpr std::string_view kASYNC = "async";
constexpr std::string_view kAWAIT = "await";
constexpr std::string_view kDYN = "dyn";
constexpr std::string_view kSTRICT_KEYWORDS[] = {
  kAS, kBREAK, kCONST, kCONTINUE, kCRATE, kELSE, kENUM, kEXTERN, kFALSE, kFN, kFOR, kIF, kIMPL, kIN, kLET,
  kLOOP, kMATCH, kMOD, kMOVE, kMUT, kPUB, kREF, kRETURN, kSELF_LOWER, kSELF_UPPER, kSTATIC, kSTRUCT, kSUPER,
  kTRAIT, kTRUE, kTYPE, kUNSAFE, kUSE, kWHERE, kWHILE, kASYNC, kAWAIT, kDYN
};

// reserved keywords
constexpr std::string_view kABSTRACT = "abstract";
constexpr std::string_view kBECOME = "become";
constexpr std::string_view kBOX = "box";
constexpr std::string_view kDO = "do";
constexpr std::string_view kFINAL = "final";
constexpr std::string_view kMACRO = "macro";
constexpr std::string_view kOVERRIDE = "override";
constexpr std::string_view kPRIV = "priv";
constexpr std::string_view kTYPEOF = "typeof";
constexpr std::string_view kUNSIZED = "unsized";
constexpr std::string_view kVIRTUAL = "virtual";
constexpr std::string_view kYIELD = "yield";
constexpr std::string_view kTRY = "try";
constexpr std::string_view kGEN = "gen";
constexpr std::string_view kRESERVERD_KEYWORDS[] = {
  kABSTRACT, kBECOME, kBOX, kDO, kFINAL, kMACRO, kOVERRIDE, kPRIV, kTYPEOF, kUNSIZED, kVIRTUAL, kTRY, kGEN
};
#endif //CONFIG_H