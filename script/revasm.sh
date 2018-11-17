#!/bin/sh

gcc -g -m32 -O0 -c $1 -o $1.o
objdump -d -M i386,intel $1.o > $1.txt
cat $1
cat $1.txt
