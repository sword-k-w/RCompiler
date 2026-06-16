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

### Codegen

TODO: const handle is not complete

#### t reg usage

- 0: arith, neg, br, j, load, store, gete, getep, comp, call, sel
- 1: arith, neg, alloca, j, load, store, gete, getep, comp, sel (pure scratch)
- 2: arith, getep, comp
- 3: const cache (pre-loaded large constant)
- 4: const cache (pre-loaded large constant)
- 5: long jumps (lui+addi+jalr for large functions)
- 6: out-of-range immediate fallback

#### Stack frame layout

s-regs at the bottom, a-reg/ra at the top, variables in between:

```
High addresses (sp + total_stack):
  ┌──────────────────────────────┐
  │ ra / a0-a7 saves             │ ← SaveRegister (top)
  ├──────────────────────────────┤ ← top of stack_size_
  │ local variables & spills     │
  ├──────────────────────────────┤ ← bottom of variable area
  │ s1 / s2 / ... saves         │ ← prologue (sp + 0, 8, ...)
  └──────────────────────────────┘
Low addresses (sp):
```

- `s_save = 8 * |used_s_regs|`, `total_stack = stack_size_ + s_save`
- `total_stack` is rounded up to the nearest multiple of 16 for RISC-V ABI compliance.
- Memory addresses are assigned after reg_alloc: only variables still in memory get stack slots.
- t1 is pure scratch (dead after each instruction), no save needed.

### mem2reg

phis in the same block are calculated in parallel!