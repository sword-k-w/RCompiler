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
* bind impl to struct (if trait impl, copy the whole AST substree that is default in trait and is not implemented in impl)
* check enumeration

### Second Round

* analyse type and calculate the value of const.

### Third Round

* type check
* check trait type

### not do yet

* trait : default copy

## IR

constant : replace it with value directly

IR code = struct definitions + functions

function = blocks

block = instructions

### Name

- function : foo -> function..foo
- struct : s -> struct.s
- method : s.foo -> function..s.foo

### Idea

Each variable/expression, use ptr to store its value. Corresponding IR name is the name of ptr. Left values use their own pointer; right value use temporary pointer.