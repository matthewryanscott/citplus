@echo off
pkzip -f -ex cit-src
if "%debugging%" == "0" goto :no
if exist version.obj del version.obj
echo.|time>oops
make -DCFLAGS="-p -Ic:\bcc55\include -2- -O2 -a -ml -N -d -w -Y -c" -DDEFS="-DAUXMEM" -DLINKER="linkdos" -DCOMPILER="bcc" -DPROGRAM="CTDL.EXE"
echo.|time>>oops
echo 
find /v "Copyright" <oops |find /v ".cpp:"|find /v "Avail"|more
:no
