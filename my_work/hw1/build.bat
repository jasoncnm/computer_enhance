@echo off

set warnings= -Wno-writable-strings -Wno-format-security -Wno-deprecated-declarations -Wno-switch

clang++ -g sim8086.cpp %warnings% -o sim8086.exe
