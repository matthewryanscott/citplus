%s may not equal %s
Read CONFIG.CIT
CONFIG.CIT : ERROR: cannot find #%s keyword!
This is the country, state or province, and county (or whatever
low-level subdivision your country uses) in which your board resides.
The preferred format is hierarchical: country/state/county. For example,
a Seattle, Washington board would have US/WA/KING.

This is the name of the system. It is displayed at the beginning of a
call, and when the .Read Status command is used. When a networked
message from your board is read on another node, this nodename will be
in the header, along with the region.

This is the city or town that your board is in. If you do not live
within the boundaries of any city/town, choose the nearest major
city/town as your region. In networked messages from your board, the
region is displayed after the nodename, in the format "nodename,
region".

This is the user name of the BBS's sysop. This account has access to all
groups and rooms. If the (optional) #FORWARD keyword is set to 1, then all
mail to "Sysop" will be forwarded to the #SYSOP account.

This is a string containing the node's phone number, including area
code. At present, it is used for informational purposes only (users will
see this field in the message header when reading verbosely), so it can
be just a partial number or "private" if the board is a private node.
For the United States and Canada, the preferred format is
"xxx-xxx-xxxx".

This determines which drive and subdirectory the Citadel system files
(CTDL.EXE, all .DAT, .TAB, .CIT files, etc.) will be in. It must be
given as a full pathname, including drive letter.

This is the standardized three letter abbreviation for your node name followed
by a period and the three letter name for your local calling area, as
determined by the code of your local airport. It is used for network purposes.
For example, the address for "The Grand Illusion" in Seattle is "tgi.sea".

This defines the modem data port. Citadel supports COM1 through COM4:
use 1 for port 1, 2 for port 2, 3 for port 3, 4 for port 4. Even though
Citadel supports them, try to avoid COM3 and COM4; on the IBM PC, the
COM3 and COM4 hardware is not well designed.

Enter value for #%s or <CR> to abort:
Enter value for #%s or <CR> for "%s":
Unknown required keyword: %s
%s invalid: cannot change to "%s" directory
Create path?
Could not create directory: %s
Directory created
Could not switch to %s after creation!
message
messages
Saving
credit
credits
CONFIG.CIT (%s) Warning: %s is multiply defined.
Warning: #%s changed to %s.
Cannot allocate space for scroll back buffer
MDATA port can only currently be 1, 2, 3 or 4
%s - Warning: unknown #%s parameter %s.
%s - Warning: Invalid value for keyword #%s: %s.
Too many borders
#MAXHALLS too large for specified #MAXROOMS
Cannot allocate space for accounting information
%s (%s) Warning: %s without preceding %s.
%s (%s) Warning: %s too long; must be fewer than %d characters.
%s (%s) Warning: unknown %s parameter "%s".
Block field bad; must be 0 to 20K
Receive command
Send command
Auto-upload string
Response file download command
Not enough memory for %s "%s"
ANY
%s - Warning: unknown %s %s.
day
Userapp name
Userapp text
Userapp command
%s - Warning: invalid %s hour %d.
 special
replaced word
replace string
Too few arguments for %s keyword.
Door name
Door command
Door %s
Allocating system tables twice.
Holiday name
Month field bad; must be 1 to 12
Date field bad; must be 0 to 31
Week field bad; must be 0 to 5
Day field bad; must be 0 to 7
Day and week field must either both be 0 or neither be 0
Cannot use both date and day/week fields together
A group that all new users are added to, for accounting.
A group reserved for special security uses by Citadel.
The first room callers see.
A room to receive network E-Mail.
Archiver name
Archiver view command
Archiver extract command
Archiver extension
Bad address: %s
Cannot allocate space for tables
Could not load RLM: %s
%s (%s) Warning: unknown keyword %s.
%s - Warning: invalid %s date %d.
Not enough memory for cron table.
Note: #%s changed to %s
Error allocating roomTab
Error allocating msgTab (%s)
Error allocating msgTab
Error allocating logTab
Insufficient FILES in CONFIG.SYS.
Please increase FILES by %d for Citadel to run.
Please set your time and date
PROMPT
CITPROMPT
PROMPT=%s
PROMPT=%s[Cit] 
PROMPT=$n$g[Cit] 
CITADEL=%s%s
Configuring Citadel.
Could not configure.
Cannot find %s; using defaults.
Could not read CONFIG.CIT.
EMS memory initialization failed!
Extended memory initialization failed!
.
Creating default %s file.
Could not create default %s file.
Configuration complete.
Fatal error in configuration. Aborting.
tmp.fil
Converting %s... %s
Must resize %s
 %s not found.
 It will be initialized.
A%s%cR%s%cD%ld%cMMessage base initialized.
Clearing: %sK%s
Lobby
Mail
Aide
Dump
A room used by Citadel to notify you of system events.
zapMsgFile: write failed
MESSAGEK = %s%s
MAXROOMS = %s%s
Clearing %s entry %s%s
zapMsgFile()
MAXLOGTAB = %s%s
log
log.tmp
log2.tmp
log3.tmp
log4.tmp
log5.tmp
log6.tmp
Building %s file (%s)
Writing %s table%s
%s (%s) Error: Too few parameters for %s.
Everybody
Special Security
msgInit()
Main
Maintenance
%s position
Cannot write size of file: %s
resizemsgfile()
Building %s table (%s)%s
log#%s%s
%s%s calls.%s
%s valid log entries.%s%s
%s no: %s%s
%s of %s %s in use%s%s
Message #%s
%s %s, #%s to %s.
Cannot find any messages.
0Building %s0 table
Copying log entry %s
Resizing %s
Converting %s
A room for storing deleted and system messages.
Cannot delete %s
Cannot rename %s
#UPSDRIVER
Done
Resizing %s
temp message
Copying %s #%s
.ups
temp room
Converting %s (%s)
temp log3
temp log4
temp log5
temp log6
Recreating %s.
NEWUSER.LOG
autoConvert()
%x %X %p
Yes
No
Could not make %s
room.tmp
roompos.tmp
hall2.tmp
LE*.DAT
.TMP
%s (%s) Error: wrong number of parameters for keyword %s.
{}
LE*.TMP
hall.tmp
grp.tmp
%s resize failed
msg.tmp
#COMDRIVER
Warning: %s driver multiply defined.%s
Not enough memory for %s driver
#VIDDRIVER
#KBDDRIVER
#SNDDRIVER
.ecd
.evd
.ekd
.esd
tmp
readprotocols()
{}
{}
room
rooms
hall
halls
user
users
group
groups
%s (%s) Error: Parameter for %s must be numeric.
{}
-end-
Invalid address in #ADDRESS keyword
Due to used rooms, #MAXROOMS has been changed from %hd to %hd.
ATM0V1
ATDT
your name
%A %x %X %p
ATX4V1MQS0=0S2=128&C1&D2
3*0
%n^A2%r%e
****
ATO
<More>
Sleeping? Call again! :-)
ATH
ATM0H1
ENG-AM.DAT
 3The sysop needs the system. You have only 1%m:%s03 left.0
ALL
%s (%s) Warning: Too many %ss.
QUESTIONS
%s (%s) Warning: Option %s of keyword %s is obsolete.
/-\|
ATA
%-17s (%s)%s
!%s
%-25s
#%s "%s"%s
#%s "%d"%s
\APPLIC
\HELP
\TEMP
\TRANS
\DLPATH
\TRAP.FBI
VMEM.TAB
PRINTER.OUT
Could not load %s entry.
   %s
Could not save %s entry.
%s\\%s.RLM
.DAT
You must wait until configuration is complete before you can do this.
 The sysop has taken over the system: your %m:%s has run out.
You hear the far-off sound of laughter...
