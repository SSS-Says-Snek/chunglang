<p align="center">
  <img src="assets/chunglang-logo.png" width=100>
</p>

<h1 align="center">Chunglang /ˈtʃʌŋleɪŋg/</h1>
<p align="center">A goofy compiled language made with LLVM and C++.</p>

<p align="center">
    <img alt="Static Badge" src="https://img.shields.io/badge/C%2B%2B17-3776ab?style=for-the-badge&logo=c%2B%2B&logoColor=ffffff">
    <img alt="GitHub License" src="https://img.shields.io/github/license/SSS-Says-Snek/chunglang?style=for-the-badge">
    <img alt="GitHub repo size" src="https://img.shields.io/github/repo-size/SSS-Says-Snek/chunglang?style=for-the-badge">
    <img alt="GitHub last commit" src="https://img.shields.io/github/last-commit/SSS-Says-Snek/chunglang?style=for-the-badge">
</p>

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