# Chunglang /ˈtʃʌŋleɪŋg/
## A goofy language made with LLVM and C++.

Chunglang aims to be a high level, fast general purpose statically compiled programming language 
with modern syntax and programming language features. 

> [!NOTE]
> This project is still extremely experimental and is definitely subject to change.

## Proposed Features

- Pattern matching
- Algebraic data types (including quotients!)
- Garbage collection
- Default immutability
- Generics
- Modules
- Multiparadigm (supporting FP and OOP)

## Install

Currently, chunglang is not on any package manager repository. Thus, 
it must be built from source. The main file to look for is `src/cli.cpp`, which contains Chunglang's command line 

### Build from Source (Only linux-verified)

1. Clone git repository
```bash
git clone https://github.com/SSS-Says-Snek/chunglang
```
2. Download LLVM and Clang from your favorite package manager
3. Compile the Chunglang CLI

```bash
clang++ src/cli.cpp src/library/prelude.cpp src/codegen.cpp src/context.cpp src/file.cpp src/lexer.cpp src/parser.cpp src/stringify.cpp src/token.cpp src/type.cpp src/utf.cpp `llvm-config --cxxflags --ldflags --system-libs --libs all` -Iinclude -fexceptions -o chung
```
4. The CLI is now available at `./chung`

## Use

Compile the test program
```bash
./chung parse test.chung
```

The resulting binary should be located in `./chungbuild/` and should be named `output.out`.