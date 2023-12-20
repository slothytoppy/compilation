#!/usr/bin/bash

if ls ./bin > /dev/null 2>&1 ; then 
gcc compilation.c -o bin/compilation
gcc test.c -o bin/test
else 
  mkdir ./bin
fi
if ls ./examples > /dev/null 2>&1 ; then
  touch test
else 
  mkdir ./examples
fi
