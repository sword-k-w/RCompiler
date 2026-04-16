#include "IR/function_map.h"

void FunctionMap::AddBuiltinFunction(const std::string &ret_type, const std::string &func_name,
  const std::vector<std::pair<std::string, std::string> > &param) {
  auto ret = std::make_shared<IRArrayNode>();
  if (ret_type != "void") {
    ret->SetBaseType(ret_type);
  }
  auto func = std::make_shared<IRFunctionNode>(ret, func_name);
  for (auto &[type, name] : param) {
    auto par_type = std::make_shared<IRArrayNode>();
    par_type->SetBaseType(type);
    auto par = std::make_shared<IRParameterNode>(par_type, name);
    func->AddParameter(par);
  }

  builtin_func_.emplace_back(func);
  mp_[func_name] = func.get();
}

FunctionMap::FunctionMap() {
  // add builtin function
  AddBuiltinFunction("void", "print", {{"ptr", "%0"}});
  AddBuiltinFunction("void", "println", {{"ptr", "%0"}});
  AddBuiltinFunction("void", "printInt", {{"i32", "%0"}});
  AddBuiltinFunction("void", "printlnInt", {{"i32", "%0"}});
  AddBuiltinFunction("ptr", "getString", {});
  AddBuiltinFunction("i32", "getInt", {});
  AddBuiltinFunction("ptr", "builtin_memset", {{"ptr", "%0"}, {"i32", "%1"}, {"i32", "%2"}});
  AddBuiltinFunction("ptr", "builtin_memcpy", {{"ptr", "%0"}, {"ptr", "%1"}, {"i32", "%2"}});
}

void FunctionMap::Add(const std::string &name, IRFunctionNode *node) {
  mp_[name] = node;
}

IRFunctionNode *FunctionMap::Query(const std::string &name) {
  return mp_[name];
}
