# TinyBasic Compiler
A Tiny Basic compiler for linux (x86_64 architecture), written for the sake of learning compilation mechanisms and having fun.

## Prerequisites
To build and run project, you need to have the following tools installed:
- `git`
- `cmake`
- `nasm`
- A C++ compiler like `g++`

## How to build
```bash
git clone https://github.com/TrealDry/tinybasic-compiler.git

cd tinybasic-compiler

cmake -DCMAKE_BUILD_TYPE=Release -S . -B ./build

cmake --build ./build
```

## How to use
```bash
./build/tinyb path/to/source.bas
./out               # binary file
```

## Language grammar

```basic
' This is a comment

010 LET A = 1  ' 010 - line number
' Var name = one upper english char
' Var value = 64 bit signed integer

020 PRINT "value = ", A
030 IF A = 1 THEN LET A = 0
' After word THEN comes command line

040 GOTO 60
050 END  ' Exiting the program

060 LET B = 0
070 INPUT A, B  ' Accepts only a number from user
' First writes number to A, then to B

080 GOSUB 200  ' It works similarly to GOTO
' But she uses word RETURN to go back.
090 END

200 LET A = A + 1
210 IF A > 5 THEN RETURN  ' Go to last GOSUB (80 line)
220 GOTO 200
```

More details are written [here](docs/grammar.md)