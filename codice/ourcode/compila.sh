#! /bin/bash

cp symbolTable.h ./build/
cp inclusioni.h ./build/
bison -d parser.y -o ./build/parser.tab.c
flex -o ./build/scanner.c scanner.l
cd build
gcc -c parser.tab.c
gcc -c scanner.c
gcc -o NEWPHP2C parser.tab.o scanner.o -lm
