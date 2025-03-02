1. The grammar is left-associative so 
succ succ 1 gets parsed as ((succ succ) 1), leading to a type unification error (since types are inferred before evaluation, and succ has a type int -> int, not int -> int -> int
