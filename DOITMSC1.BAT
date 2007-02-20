@echo off
cl /AL /O2 /Gf /Ge /c /f- /W4 /DAUXMEM %1.cpp >oops1
type oops1|find /V "C4505"
type oops1|find /V "C4505" >>oops
