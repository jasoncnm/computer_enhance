@echo off

set asm=%1

call ..\code\sim8086 %asm% > test1.asm
call type test1.asm
call nasm test1.asm
call fc test1 %asm%
call del test1.*
