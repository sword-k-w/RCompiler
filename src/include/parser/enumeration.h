#ifndef ENUMERATION_H
#define ENUMERATION_H

class EnumerationNode : public ASTNode {
public:
  EnumerationNode() = delete;
  EnumerationNode(const std::vector<Token> &, uint32_t &, const uint32_t &);
private:
};

#endif //ENUMERATION_H
