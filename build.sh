#!/usr/bin/bash

if ls ~/gh/compilation/bin > /dev/null 2>&1 ; then 
gcc -Wall -Wextra ~/gh/compilation/example.c -o ~/gh/compilation/bin/example
else 
  mkdir ~/gh/compilation/bin
fi
if ls ~/gh/compilation/examples > /dev/null 2>&1 ; then
  touch ~/gh/compilation/examples/test.c
else 
  mkdir ./examples
fi
