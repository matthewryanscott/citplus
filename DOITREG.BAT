@echo off
if "%debugging%" == "0" goto :no
if exist version.obj del version.obj
echo.|time>oops
make -DCFLAGS="-2- -O2 -a -ml -N -d -w -Y -c" -DDEFS="-DREGULAR" -DLINKER="linkdos" -DCOMPILER="bcc" -DPROGRAM="CTDL.EXE"
echo.|time>>oops
find /v "Copyright" <oops |find /v ".cpp:"|find /v "Avail"|more
:no
