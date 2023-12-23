#!/usr/bin/bash

if ls ./bin > /dev/null 2>&1 ; then 
gcc -Wall example.c -o bin/example
print("hello")
else 
  mkdir ./bin
fi
if ls ./examples > /dev/null 2>&1 ; then
  touch examples/test.c
else 
  mkdir ./examples
fi
