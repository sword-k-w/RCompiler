#include "lexer/lexer.h"
#include <iostream>

Lexer::Lexer(const std::string &input) : input_(input), pos_(0), length_(input.size()) {}

auto Lexer::Run() -> std::vector<Token> const {
  sregex ascii_alpha = sregex::compile("[a-zA-Z]");
  sregex bin_digit = sregex::compile("[0-1]");
  sregex oct_digit = sregex::compile("[0-7]");
  sregex dec_digit = sregex::compile("[0-9]");
  sregex hex_digit = sregex::compile("[0-9a-fA-F]");
  sregex ascii_for_char = sregex::compile("[^'\\\\\xA\xD\x9]");
  sregex ascii_for_string = sregex::compile("[^\"\\\\\xD]");
  sregex ascii_for_raw = sregex::compile("[^\xD]");

  sregex identifier_or_keyword = ascii_alpha >> *_w;

  sregex quote_escape = sregex::compile("\\\\'|\\\\\"");
  sregex ascii_escape = (sregex::compile("\\\\x") >> oct_digit >> hex_digit) |
    sregex::compile("\\\\n") | sregex::compile("\\\\r") | sregex::compile("\\\\t") |
    sregex::compile("\\\\\\\\") | sregex::compile("\\\\0");

  sregex suffix = identifier_or_keyword;
  sregex suffix_no_e = (sregex::compile("[a-df-zA-DF-Z]")) >> *_w;

  sregex char_literal = sregex::compile("'") >>
    (ascii_for_char | quote_escape | ascii_escape) >>
    sregex::compile("'") >> !suffix;

  sregex string_continue = sregex::compile("\\\\\xA");
  sregex string_literal = sregex::compile("\"") >>
    *(ascii_for_string | quote_escape | ascii_escape | string_continue) >>
    sregex::compile("\"") >> !suffix;

  sregex raw_string_content;
  raw_string_content = (sregex::compile("\"") >> *!ascii_for_raw >> sregex::compile("\"")) |
    (sregex::compile("#") >> by_ref(raw_string_content) >> sregex::compile("#"));
  sregex raw_string_literal = sregex::compile("r") >> raw_string_content >> !suffix;

  sregex byte_escape = ascii_escape | sregex::compile("\\\\'") | sregex::compile("\\\\\"");
  sregex byte_literal = sregex::compile("b'") >> (ascii_for_char | byte_escape) >> sregex::compile("'") >> !suffix;

  sregex byte_string_literal = sregex::compile("b\"") >> *(ascii_for_string | byte_escape | string_continue) >>
    sregex::compile("\"") >> !suffix;

  sregex raw_byte_string_content;
  raw_byte_string_content = (sregex::compile("\"") >> *!ascii_for_raw >> sregex::compile("\"")) |
    (sregex::compile("#") >> by_ref(raw_byte_string_content) >> sregex::compile("#"));
  sregex raw_byte_string_literal = sregex::compile("br") >> raw_byte_string_content >> !suffix;

  // sregex c_string_literal = sregex::compile("c\"") >> *(sregex::compile("[^\\\\\xD\x0]") |
  //   (byte_escape - sregex::compile("\\\\0") - sregex::compile("\\\\x00")) | string_continue)
  // >> sregex::compile("\"") >> !suffix;

  sregex raw_c_string_content;
  raw_c_string_content = (sregex::compile("\"") >> *!sregex::compile("[^\\\\\xD\x0]") >> sregex::compile("\"")) |
    (sregex::compile("#") >> by_ref(raw_c_string_content) >> sregex::compile("#"));
  sregex raw_c_string_literal = sregex::compile("cr") >> raw_c_string_content >> !suffix;

  sregex dec_literal = dec_digit >> *(dec_digit | sregex::compile("_"));
  sregex bin_literal = sregex::compile("0b") >> *(bin_digit | sregex::compile("_")) >> bin_digit >>
    *(bin_digit | sregex::compile("_"));
  sregex oct_literal = sregex::compile("0o") >> *(oct_digit | sregex::compile("_")) >> oct_digit >>
    *(oct_digit | sregex::compile("_"));
  sregex hex_literal = sregex::compile("0x") >> *(hex_digit | sregex::compile("_")) >> hex_digit >>
    *(hex_digit | sregex::compile("_"));
  sregex integer_literal = (dec_literal | bin_literal | oct_literal | hex_literal) >> !suffix_no_e;

  sregex float_literal = (dec_literal >> sregex::compile("\\.") >> dec_literal >> !suffix) |
    (dec_literal >> sregex::compile("\\."));
  // remember to handle "not immediately followed by ., _ or an ASCII_ALPHA character"

  sregex punctuation = sregex::compile("[=<>!~\\+\\-\\*/%\\^&\\|@\\.,;:#$\\?_\\{\\}\\[\\]\\(\\)]|<=|==|!=|>=|&&|\\|\\||<<|>>|\\+=|\\-=|\\*=|/=|%=|\\^=|&=|\\|=|<<=|>>=|\\.|\\.\\.|\\.\\.\\.|\\.\\.=|::|\\->|<\\-");

  sregex reserved_guard_string_literal = sregex::compile("#+") >> string_literal;
  sregex reserved_number = (bin_literal >> sregex::compile("[2-9]")) | (oct_literal >> sregex::compile("[8-9]"))
    | ((bin_literal | oct_literal | hex_literal) >> sregex::compile("\\.")) | (sregex::compile("0b"));
  // remember to handle "not immediately followed by ., _ or an ASCII_ALPHA character
  // remember to handle 0?_*<end of input or not HEX_DIGIT>

  sregex line_comment = sregex::compile("//[^\xA]*");
  sregex block_comment;
  sregex block_comment_content;
  block_comment = sregex::compile("/\\*") << *by_ref(block_comment_content) << sregex::compile("\\*/");
  block_comment_content = by_ref(block_comment) | sregex::compile("[^\\*/]") | sregex::compile("\\*[^/]") | sregex::compile("/[^\\*]");

  smatch match;
  std::string a = "r##\"\"##";
  if (regex_search(a, match, raw_string_literal) && match.position() == 0) {
    std::cerr << "1\n";
    std::cerr << match[0] << '\n';
  }
  return {};
}

