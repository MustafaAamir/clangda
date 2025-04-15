# grammar: 

```
<var> ::= [_A-Za-z]+[A-Za-z]
<nat> ::= [0-9]+
<true> ::= "true"
<false> ::= "false"
<unit> ::= "unit"
<atom> ::= <var> | <nat> | <true> | <false> | <unit> | "(" <atom> ")"
<application> ::= <abstraction> <atom>
<abstraction> ::= "\" <identifier> "." <expr>
<expr> ::= <application> | <abstraction> | <atom> | <application> <atom> | "(" <expr> ")"
<let> ::= "let" <identifier> "=" <expr> "in" <expr>
```

1. The grammar is left-associative so succ succ 1 gets parsed as ((succ succ) 1), leading to a type unification error (since types are inferred before evaluation, and succ has a type int -> int, not int -> int -> int

2. primitives don't work. Don't use them </3

3. Makefile is a mess. leave it be

4. type system doesn't work. expected int, got 'a
