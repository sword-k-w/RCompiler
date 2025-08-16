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

* MethodCallExpression = Expression . IDENTIFIER ( CallParams? )

* FieldExpression = Expression . IDENTIFIER