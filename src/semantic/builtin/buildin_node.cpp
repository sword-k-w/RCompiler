#include "semantic/builtin/builtin_node.h"

BuiltinFunctionNode::BuiltinFunctionNode(const std::string &function_name) : ASTNode("Builtin Function"), function_name_(function_name) {}

void BuiltinFunctionNode::Accept(VisitorBase *ptr) {}
