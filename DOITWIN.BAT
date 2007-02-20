@echo off
rem pkzip -f -ex cit-src
if "%debugging%" == "0" goto :no
if exist version.obj del version.obj
echo.|time>oops
rem :make -DCFLAGS="-p -O2 -a -N -d -w -W -WM -c" -DDEFS="-DWINCIT" -DLINKER="linkwin" -DCOMPILER="bcc32" -DPROGRAM="CTDL.EXE"
rem make -fmakewin -DCFLAGS="-p -IB:\include -a -N -d -W -WM -c" -DDEFS="-DWINCIT -DNAH69" -DLINKER="linkwin" -DCOMPILER="bcc32" -DPROGRAM="CTDL.EXE"
rem make -fmakewin -DCFLAGS="-p -IB:\include -a -N -d -W -WM -c" -DDEFS="-DWINCIT -DHARRY" -DLINKER="linkwin" -DCOMPILER="bcc32" -DPROGRAM="CTDL.EXE"
make -fmakewin -DCFLAGS="-p -IB:\include -a -N -d -W -WM -c" -DDEFS="-DWINCIT" -DLINKER="linkwin" -DCOMPILER="bcc32" -DPROGRAM="CTDL.EXE"
echo.|time>>oops
echo 
find /v "Copyright" <oops |find /i /v ".cpp:"|find /v "Avail"|more
:no
