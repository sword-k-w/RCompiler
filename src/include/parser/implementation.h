#ifndef IMPLEMENTATION_H
#define IMPLEMENTATION_H

class ImplementationNode : public ASTNode {
public:
  ImplementationNode();
  ImplementationNode(const std::vector<Token> &, uint32_t &, const uint32_t &);
private:

};

#endif //IMPLEMENTATION_H
