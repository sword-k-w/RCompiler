#ifndef STRUCT_H
#define STRUCT_H

class StructFieldsNode : public ASTNode {

};

class StructStructNode : public ASTNode {

};

class TupleFieldsNode : public ASTNode {

};

class TupleStructNode : public ASTNode {

};

class StructNode : public ASTNode {
public:
  StructNode() = delete;
  StructNode(const std::vector<Token> &, uint32_t &, const uint32_t &);
private:
  StructStructNode *struct_struct_ = nullptr;
  TupleStructNode *tuple_struct_ = nullptr;
};

#endif //STRUCT_H
