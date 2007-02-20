:link /debug /stack:5000 winmm.lib wsock32.lib gdi32.lib comctl32.lib comdlg32.lib user32.lib @mscresp
:link adddat
:link ctdldat comp compx
:if not exist ctdl.dat ctdldat -n -DWINCIT
:adddat ctdl.exe
:bscmake /o ctdl.bsc *.sbr

:inslink /debug /stack:5000 winmm.lib wsock32.lib gdi32.lib comctl32.lib comdlg32.lib user32.lib @mscresp
:inslink adddat.obj
:inslink ctdldat.obj comp.obj compx.obj 
:if not exist ctdl.dat ctdldat -n -DWINCIT
:adddat ctdl.exe

nmlink /debug /stack:5000 winmm.lib wsock32.lib gdi32.lib comctl32.lib comdlg32.lib user32.lib @mscresp
nmlink /debug adddat 
nmlink /debug ctdldat comp compx 
if not exist ctdl.dat ctdldat -n -DWINCIT
adddat ctdl.exe



