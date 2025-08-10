#ifndef TRAIT_H
#define TRAIT_H

class TraitNode : public ASTNode {
public:
  TraitNode() = delete;
  TraitNode(const std::vector<Token> &, uint32_t &, const uint32_t &);
private:
};

#endif //TRAIT_H
