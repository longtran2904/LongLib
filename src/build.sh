#!/bin/bash

code="$PWD"
opts=-g
cd build > /dev/null
g++ $opts $code/Demo.c -o Demo.exe
cd $code > /dev/null
