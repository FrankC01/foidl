# foidl

Frank's Orthogonally Independent Domain Language (`foidl`) is a functional programming language.

Caution: For all but the lack of garbage collection the current master is considered alpha at best and not ready for serious prime time consideration.

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
* libcurl (if using extensions)

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
| http channels | x | x | |
| threads | x | x | x |
| thread-pools | x | x | |


## Author's Environment(s)

| OS | Python | rply | colorlog | llvmlite | LLVM | libcurl |
| -- | ------ | ---- | -------- | -------- | ---- | ------- |
| macos 10.14.4 | 3.7.3 | 0.7.7 | 4.0.2 | 0.29.0 | 8.0.0 | 7.54.0 |
| Ubuntu 18.04.1 (VM) | 3.6.7 | 0.7.6 | 4.0.2 | 0.27.0 | 7.1.0 | 7.58.0 |
| Windows 10.0.17134  (VM) | 3.7.0 | 0.7.6 | 3.1.4 | 0.25.0 | 7.0.0 | 7.54.0 |


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

The folder `extends` is the location for any runtime library/language extensions. Once you build the compiler, you can build extensions to be used in user programs. Each extension is it's own folder and will automatically be built with:
```
cd extends
make clean
make all
```

The folder `tests/selfhosted` contains a few contrived foidl examples. Once the foidl compiler is built it will be used to build them. After building the compiler do:
```
cd tests/selfhosted
make clean
make
```