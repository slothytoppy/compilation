#!/usr/bin/bash

if ls ./bin > /dev/null 2>&1 ; then 
gcc example -o bin/example
gcc test.c -o bin/test
else 
  mkdir ./bin
fi
if ls ./examples > /dev/null 2>&1 ; then
  touch test
else 
  mkdir ./examples
fi
