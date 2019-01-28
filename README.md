# foidl

Frank's Orthogonally Independent Domain Language (`foidl`) is a functional programming language.

Inspirational:
* Homoiconicity
* Persistent data
* First class functions
* Lambda calculus (partial, currying, lambdas)
* Arbitrarily Large Numbers

My own twists:
* Typeless - Everything is Any thing for the most part
* Delimiterless (mostly) - Not for the weak of heart but I like to decide using format style


Building foidl
Building the executable, self-hosted, foidl compiler (`foidlc`) is 2 step process:

1. Build the `libfoidl` run time library - This is done with C/C++ and Python bootstrapping.
2. Build the `foidlc`compiler - This is done with Python bootstrapping.

The folder `tests/selfhosted` contains a few contrived foidl examples. Once the foidl compiler is built it will be used to build them. After building the compiler do:
```
cd tests/selfhosted
make
```