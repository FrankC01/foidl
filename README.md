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


## Prereqs
* Python 3.6+
* LLVM/Clang 7.0+
* libcurl

### Python libraries to `pip` in
* rply
* colorlog
* llvmlite

## Coverage

| Feature | macos | linux | windows |
| ------- | ----- | ----- | ------- |
| bootstrap | x | x | x |
| self-hosted | x | x | x |
| file channels | x | x | x |
| http channels | | | |
| threads | x | x | x |
| thread-pools | x | x | |



## Building foidl
Building the executable, self-hosted, foidl compiler (`foidlc`) is 2 step process:

1. Build the `libfoidl` run time library - This is done with C/C++ and Python bootstrapping.
```
cd foidlrtl
make clean
make
```
2. Build the `foidlc`compiler - This is done with Python bootstrapping.
```
cd foidlc
make clean
make
```

The folder `tests/selfhosted` contains a few contrived foidl examples. Once the foidl compiler is built it will be used to build them. After building the compiler do:
```
cd tests/selfhosted
make clean
make
```