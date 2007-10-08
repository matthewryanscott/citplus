 # Citadel+ Makefile

.autodepend

.cpp.obj:
    $(COMPILER) $(CFLAGS) $(DEFS) $< >>oops

ALL : ADDDAT.EXE CTDLDAT.EXE CTDL.DAT $(PROGRAM)

ADDDAT.EXE : ADDDAT.C
    $(COMPILER) -Le:\bc45\lib -Ig:\bc5\include -ml adddat.c >>oops

CTDLDAT.EXE : CTDLDAT.Cpp COMP.CPP COMP.H compx.cpp compx.h
        $(COMPILER) -Le:\bc45\lib -Ig:\bc5\include -p -O2 -a -N -d -w ctdldat comp compx >>oops
        erase comp.obj
        erase compx.obj

CTDL.DAT : CTDLDATA.DEF CTDLDAT.EXE CTDL.MSG ANSI.HLP CTDLNET.MSG CTDLSYS.MSG\
    MDMRESLT.SMP F4SCREEN.MSG CTDLNETC.MSG DOHELP.HLP INTRO.HLP LOGIN.HLP\
    CTDLEC.MSG GROUPS.HLP PROMPT.BLB COLORS.BLB CTDLTRAP.MSG CTDLNODE.MSG\
    PIXIE.MSG ROOMSYS.HLP FILES.HLP SPECIAL.HLP PROTOCOL.SMP MESSAGES.HLP\
    HALLS.HLP CTDLSCR.MSG AABUYME.MSG NETWORK.HLP ACCOUNT.HLP GRPDATA.SMP\
    CTDLVLOG.MSG CTDLOOM.MSG CTDLHLP.MSG CTDLML.MSG AIDE.HLP EXTERNAL.SMP\
    QUEUE.HLP ROOMS.HLP SPELL.HLP NODES.SMP HELP.HLP SYSOP.HLP CONFIG_d.HLP\
    CRON.SMP CTDLKNWN.MSG CTDLFS.MSG CTDLAIDQ.MSG CTDLXMDM.MSG CTDLTE.MSG\
    CFG.MSG CTDLAIDE.MSG CTDLREAD.MSG DOORS.HLP CTDLENTR.MSG CTDLCMDd.MSG\
    TIME.BLB CTDLIF.MSG COPYRITE.MSG CTDLDBG.MSG CTDLCON.MSG CTDLTRAN.MSG\
    ANTI.MSG CTDLRMSG.MSG CTDLLI.MSG TERMCAP.SMP
    ctdldat $(DEFS)

ctdl.obj : ctdl.cpp
    $(COMPILER) -v $(CFLAGS) $(DEFS) ctdl.cpp >>oops

SER.OBJ : SER.ASM
    tasm /w2 /ml $(DEFS) SER.ASM; >>oops

AUXMEM.OBJ : AUXMEM.ASM
    tasm /w2 /ml $(DEFS) AUXMEM.ASM; >>oops

MOUSE.OBJ : MOUSE.ASM
    tasm /w2 /ml $(DEFS) MOUSE.ASM; >>oops

RLMASM.OBJ : RLMASM.ASM
    tasm /w2 /ml $(DEFS) RLMASM.ASM; >>oops

TIMERA.OBJ : TIMERA.ASM
    tasm /w2 /ml $(DEFS) TIMERA.ASM; >>oops

hufbit.OBJ : hufbit.ASM
    tasm /w2 /ml hufbit.ASM; >>oops

wind.obj : wind.asm
    tasm /w2 /ml wind.asm; >>oops

$(PROGRAM) : \
    account.obj acctcfg.obj andy.obj aplfile.obj aplic.obj\
    auxmem.obj auxtab.obj boolexpr.obj carrier.obj chat.obj commands.obj\
    comp.obj compx.obj config.obj conovl.obj console.obj consysop.obj\
    cron.obj cronfile.obj crypt.obj csysdosu.obj csysgrp.obj csyshall.obj\
    csyshelp.obj csysmon.obj csysroom.obj csysuser.obj ctdl.obj\
    cwinalrt.obj cwinctl.obj cwindows.obj cwinio.obj cwinrsmv.obj\
    cwinte.obj cwmenu.obj cyclelog.obj discard.obj doaide.obj doenter.obj\
    doinv.obj domenu.obj domisc.obj doread.obj dormhall.obj dosysop.obj\
    doupdown.obj down.obj edit.obj error.obj extedit.obj external.obj\
    extmsg.obj filecit.obj filecmd.obj\
    filemake.obj filerdwr.obj files.obj fmtovl.obj format.obj fscfg.obj\
    globver.obj group.obj group2.obj grpedit.obj grpmembr.obj grpovl.obj\
    grprdwr.obj hall.obj halledit.obj hallovl.obj help.obj huf.obj\
    hufbit.obj infofile.obj init.obj inpovl.obj input.obj libovl.obj\
    llsup.obj log.obj log2.obj log3.obj logacc.obj logedit.obj logext.obj\
    login.obj loginnew.obj logout.obj logovl.obj logrdwr.obj maillist.obj\
    maketab.obj mci.obj meminit.obj misc.obj misc2.obj misc3.obj\
    mmakehi.obj moreinfo.obj modem.obj mouse.obj movehall.obj msg.obj\
    msgaddr.obj msgcfg.obj msgload.obj msgmake.obj msgmod.obj msgovl.obj\
    msgread.obj msgread2.obj msgscrpt.obj msgtab.obj music.obj\
    ncmddbg.obj net.obj net1.obj net69.obj net6969.obj net69mnu.obj net86.obj\
    netcmd.obj netdc15.obj netidcit.obj netmail.obj netmsgi.obj\
    netmsgo.obj netnode.obj nomouse.obj openfile.obj outfile.obj outovl.obj\
    output.obj outwin.obj pathdat.obj port.obj readcfg.obj readext.obj\
    readinfo.obj readlog.obj resize.obj reszroom.obj rlm.obj rlmasm.obj\
    room2.obj room3.obj roomedit.obj roomlow.obj roommake.obj\
    roomshow.obj runcron.obj scrfcfg.obj scrfcmd.obj scrfdbg.obj\
    scrffile.obj scrfgrp.obj scrfhall.obj scrflog.obj scrfmsg.obj\
    scrfnet.obj scrfroom.obj scrfunc.obj scrfusr.obj script.obj\
    scriptdb.obj scriptld.obj scrlback.obj scrlbovl.obj ser.obj\
    setinfo.obj spellchk.obj status.obj strftime.obj syncdata.obj\
    tallybuf.obj telnetd.obj term.obj termovl.obj timedate.obj\
    timeovl.obj timer.obj timera.obj trap.obj uimisc.obj userout.obj\
    viewlog.obj wind.obj windovl.obj window.obj xmodem.obj zipfile.obj\
    zmodem.obj\
    wincit.res version.obj
    $(LINKER)
