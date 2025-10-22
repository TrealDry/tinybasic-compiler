#!/bin/bash

nasm -g -felf64 out.asm
ld -o out out.o -lc --dynamic-linker /lib64/ld-linux-x86-64.so.2