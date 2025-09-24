# RCompiler

## parser

### expression handle

main idea: Pratt Parsing

grammar involved:

* BorrowExpression = (& | &&) Expression | (& | &&) mut Expression
* DereferenceExpression = * Expression
* NegationExpression = - Expression | ! expression
* ArithmeticOrLogicalExpression = 
    Expression + Expression
  | Expression - Expression
  | Expression * Expression
  | Expression / Expression
  | Expression % Expression
  | Expression & Expression
  | Expression | Expression
  | Expression ^ Expression
  | Expression << Expression
  | Expression >> Expression
* ComparisonExpression =
    Expression == Expression
  | Expression != Expression
  | Expression > Expression
  | Expression < Expression
  | Expression >= Expression
  | Expression <= Expression
* LazyBooleanExpression = Expression || Expression | Expression && Expression
* TypeCastExpression = Expression as TypeNoBounds
* AssignmentExpression = Expression = Expression
* CompoundAssignmentExpression =
    Expression += Expression
  | Expression -= Expression
  | Expression *= Expression
  | Expression /= Expression
  | Expression %= Expression
  | Expression &= Expression
  | Expression |= Expression
  | Expression ^= Expression
  | Expression <<= Expression
  | Expression >>= Expression
* GroupedExpression = ( Expression )
* IndexExpression = Expression [ Expression ]
* CallExpression = Expression ( CallParams? )
* MethodCallExpression = Expression . PathExprSegment ( CallParams? )
* FieldExpression = Expression . IDENTIFIER
* BreakExpression = break Expression?
* ReturnExpression = return Expression?

leaf expression:

* LiteralExpression
* PathExpression
* ArrayExpression
* StructExpression
* ContinueExpression
* UnderscoreExpression
* ExpressionWithBlock

## Semantic

### First Round

* build scope tree
* collect the name of items. (using 0/1 BFS to ensure names in same scope are added successively(maybe not necessary now))
* bind impl to struct
* check enumeration

### Second Round

* analyse type and calculate the value of const.

### Third Round

* type check

