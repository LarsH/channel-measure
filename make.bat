@echo off
mkdir out
tools\make.exe -C out -f ../src/Makefile %1 %2 %3 %4 %5
