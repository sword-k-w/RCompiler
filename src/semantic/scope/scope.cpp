#include "semantic/scope/scope.h"

Scope::Scope(std::shared_ptr<Scope> parent) : parent_(std::move(parent)) {}
