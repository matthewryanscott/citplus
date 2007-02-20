// --------------------------------------------------------------------------
// Citadel: LibOvl.CPP
//
// Table create/load/save stuff.

#include "ctdl.h"
#pragma hdrstop

#include "account.h"
#include "libovl.h"
#include "net.h"
#include "group.h"
#include "room.h"
#include "log.h"
#include "hall.h"
#include "tallybuf.h"
#include "term.h"
#include "extmsg.h"
#include "statline.h"
#include "termwndw.h"

// --------------------------------------------------------------------------
// Contents
//
// getGroup()           loads groupBuffer
// putGroup()           stores groupBuffer to disk
//
// getHall()            loads hallBuffer
// putHall()            stores hallBuffer to disk
//
// writeTables()        writes all system tables to disk
//
// allocateTables()     allocate table space

Bool writeBordersDat(void)
    {
    Bool Good = TRUE;
    FILE *fl;

    changedir(cfg.homepath);

    if ((fl = fopen(bordersDat, FO_WB)) != NULL)
        {
        if ((int) fwrite(borders, 81, cfg.maxborders, fl) != cfg.maxborders)
            {
#ifndef WINCIT
            mPrintf(getmsg(83), bordersDat);
#endif
            Good = FALSE;
            }

        fclose(fl);
        }
    else
        {
#ifndef WINCIT
        mPrintf(getmsg(78), bordersDat);
#endif
        Good = FALSE;
        }

    return (Good);
    }

Bool readBordersDat(void)
    {
    Bool Good = TRUE;
    FILE *fl;

    changedir(cfg.homepath);

    if ((fl = fopen(bordersDat, FO_RB)) != NULL)
        {
        int i;

        if ((int) fread(borders, 81, cfg.maxborders, fl) != cfg.maxborders)
            {
#ifndef WINCIT
            cPrintf(getmsg(83), bordersDat);
#endif
            Good = FALSE;
            }

        fclose(fl);

        // Make sure none of them overrun their space
        for (i = 0; i < cfg.maxborders; i++)
            {
            borders[(i + 1) * 81 - 1] = 0;
            }
        }
    else
        {
#ifndef WINCIT
        cPrintf(getmsg(78), bordersDat);
#endif
        Good = FALSE;
        }

    return (Good);
    }

void LogTable::Save(void) const
    {
    assert(this);
    VerifyHeap();

    if (IsValid())
        {
        char FileName[80];
        FILE *fd;

        sprintf(FileName, sbs, cfg.homepath, lgTab);

        if ((fd = fopen(FileName, FO_WB)) != NULL)
            {
            for (l_slot ls = 0; ls < TableSize; ls++)
                {
                Table[ls].Save(fd);
                }

            fclose(fd);
            }

        VerifyHeap();
        }
    }

Bool LogTable::Load(void)
    {
    assert(this);

    if (IsValid())
        {
        char FileName[80];
        FILE *fd;

        sprintf(FileName, sbs, cfg.homepath, lgTab);

        if ((fd = fopen(FileName, FO_RB)) == NULL)
            {
            return (FALSE);
            }

        VerifyHeap();

        for (l_slot ls = 0; ls < TableSize; ls++)
            {
            if (!Table[ls].Load(fd))
                {
                VerifyHeap();
                fclose(fd);
                unlink(lgTab);
                return (FALSE);
                }
            }

        VerifyHeap();
        fclose(fd);
        unlink(lgTab);
        return (TRUE);
        }
    else
        {
        return (FALSE);
        }
    }


void RoomTable::Save(void)
    {
    assert(this);
    VerifyHeap();

    if (IsValid())
        {
        char FileName[80];
        FILE *fd;

        sprintf(FileName, sbs, cfg.homepath, rmTab);

        if ((fd = fopen(FileName, FO_WB)) != NULL)
            {
            for (r_slot rs = 0; rs < TableSize; rs++)
                {
                (*this)[rs].Save(fd);
                }

            fclose(fd);
            }

        VerifyHeap();
        }
    }


Bool RoomTable::Load(void)
    {
    assert(this);

    if (IsValid())
        {
        char FileName[80];
        FILE *fd;

        sprintf(FileName, sbs, cfg.homepath, rmTab);

        if ((fd = fopen(FileName, FO_RB)) == NULL)
            {
            return (FALSE);
            }

        VerifyHeap();

        for (r_slot rs = 0; rs < TableSize; rs++)
            {
            if (!(*this)[rs].Load(fd))
                {
                VerifyHeap();
                fclose(fd);
                unlink(rmTab);
                return (FALSE);
                }
            }

        VerifyHeap();
        fclose(fd);
        unlink(rmTab);
        return (TRUE);
        }
    else
        {
        return (FALSE);
        }
    }


// --------------------------------------------------------------------------
// getHall(): Loads hall data into RAM buffer.

Bool HallBuffer::Load(const char *Name)
    {
    assert(this);
    VerifyHeap();
    Bool Good = TRUE;
    char fpath[80];

    sprintf(fpath, sbs, cfg.homepath, Name);

    FILE *HallFile1;

    if ((HallFile1 = fopen(fpath, FO_RB)) == NULL)
        {
#ifndef WINCIT
        mPrintf(getmsg(78), fpath);
#endif
        Good = FALSE;
        }
    else
        {
        FILE *HallFile2;

        sprintf(fpath, sbs, cfg.homepath, hall2Dat);

        if ((HallFile2 = fopen(fpath, FO_RB)) == NULL)
            {
#ifndef WINCIT
            mPrintf(getmsg(78), fpath);
#endif
            Good = FALSE;
            }
        else
            {
            h_slot HallSlot;

            for (HallSlot = 0; Good && HallSlot < NumHalls; HallSlot++)
                {
                if (!Halls[HallSlot].Load(HallSlot, HallFile1, HallFile2))
                    {
                    Good = FALSE;
                    }
                }

            fclose(HallFile2);
            }

        fclose(HallFile1);
        }

    VerifyHeap();
    return (Good);
    }

Bool HallEntry1::Load(h_slot Slot, FILE *File)
    {
    assert(this);

    GAINEXCLUSIVEACCESS();

    fseek(File, sizeof(long) + Slot * SizeOfDiskRecord(), SEEK_SET);

#ifdef WINCIT
    ushort Temp;

    int Bad = fread(Name, sizeof(Name), 1, File) != 1;
    Bad += fread(&GroupNumber, sizeof(GroupNumber), 1, File) != 1;
    Bad += fread(&Temp, sizeof(Temp), 1, File) != 1;
    Bad += fread(&UNUSED3, sizeof(UNUSED3), 1, File) != 1;
    Bad += fread(DescriptionFile, sizeof(DescriptionFile), 1, File) != 1;
    Bad += fread(GroupExpression, sizeof(GroupExpression), 1, File) != 1;

    if (!Bad)
        {
        Inuse = Temp & 1;
        Owned = (Temp >> 1) & 1;
        Described = (Temp >> 2) & 1;
        Default = (Temp >> 3) & 1;
        EnterRoom = (Temp >> 4) & 1;
        BoolGroup = (Temp >> 5) & 1;
        UNUSED2 = (Temp >> 6) & 1023;
        }
#else
    Bool Bad = FALSE;
    if (fread(this, sizeof(HallEntry1), 1, File) != 1)
        {
        mPrintf(getmsg(83), hallDat);
        Bad = TRUE;
        }
#endif

    Verify();

    RELEASEEXCLUSIVEACCESS();

    return (!Bad);
    }

Bool HallEntry2::Load(h_slot Slot, FILE *File)
    {
    assert(this);
    Bool Good = TRUE;

    fseek(File, Slot * GetSize(), SEEK_SET);

    if (fread(GetNCPointer(), 1, GetSize(), File) != GetSize())
        {
#ifndef WINCIT
        mPrintf(getmsg(83), hall2Dat);
#endif
        Good = FALSE;
        }

    return (Good);
    }

Bool HallEntry3::Load(h_slot Slot, FILE *File)
    {
    assert(this);
    Bool Good = TRUE;

    fseek(File, (cfg.maxhalls + Slot) * GetSize(), SEEK_SET);

    if (fread(GetNCPointer(), 1, GetSize(), File) != GetSize())
        {
#ifndef WINCIT
        mPrintf(getmsg(83), hall2Dat);
#endif
        Good = FALSE;
        }

    return (Good);
    }

Bool HallEntry::Load(h_slot HallSlot, FILE *File1, FILE *File2)
    {
    assert(this);

    return (HallEntry1::Load(HallSlot, File1) && HallEntry2::Load(HallSlot, File2) && HallEntry3::Load(HallSlot, File2));
    }


// --------------------------------------------------------------------------
// putHall(): Stores group data into HALL.DAT.

Bool HallBuffer::Save(const char *Name) const
    {
    assert(this);
    VerifyHeap();
    Bool Good = TRUE;
    char fpath[80];

    sprintf(fpath, sbs, cfg.homepath, Name);

    FILE *HallFile1;

    if ((HallFile1 = fopen(fpath, FO_WB)) == NULL)
        {
#ifndef WINCIT
        mPrintf(getmsg(78), fpath);
#endif
        Good = FALSE;
        }
    else
        {
        FILE *HallFile2;

        sprintf(fpath, sbs, cfg.homepath, hall2Dat);

        if ((HallFile2 = fopen(fpath, FO_WB)) == NULL)
            {
#ifndef WINCIT
            mPrintf(getmsg(78), fpath);
#endif
            Good = FALSE;
            }
        else
            {
            long l = Halls[0].SizeOfDiskRecord();
            fwrite(&l, sizeof(l), 1, HallFile1);

            h_slot HallSlot;

            for (HallSlot = 0; Good && HallSlot < NumHalls; HallSlot++)
                {
                if (!Halls[HallSlot].Save(HallSlot, HallFile1, HallFile2))
                    {
                    Good = FALSE;
                    }
                }

            fclose(HallFile2);
            }

        fclose(HallFile1);
        }

    VerifyHeap();
    return (Good);
    }

Bool HallEntry1::Save(h_slot Slot, FILE *File) const
    {
    assert(this);

    GAINEXCLUSIVEACCESS();

    fseek(File, sizeof(long) + Slot * SizeOfDiskRecord(), SEEK_SET);

#ifdef WINCIT
    ushort Temp = (ushort) (Inuse | (Owned << 1) | (Described << 2) | (Default << 3) | (EnterRoom << 4) |
			(BoolGroup << 5) | (UNUSED2 << 6));

    int Bad = fwrite(Name, sizeof(Name), 1, File) != 1;
    Bad += fwrite(&GroupNumber, sizeof(GroupNumber), 1, File) != 1;
    Bad += fwrite(&Temp, sizeof(Temp), 1, File) != 1;
    Bad += fwrite(&UNUSED3, sizeof(UNUSED3), 1, File) != 1;
    Bad += fwrite(DescriptionFile, sizeof(DescriptionFile), 1, File) != 1;
    Bad += fwrite(GroupExpression, sizeof(GroupExpression), 1, File) != 1;
#else
    Bool Bad = FALSE;
    if (fwrite(this, sizeof(HallEntry1), 1, File) != 1)
        {
        mPrintf(getmsg(661), hallDat);
        Bad = TRUE;
        }
#endif

    RELEASEEXCLUSIVEACCESS();

    return (!Bad);
    }

Bool HallEntry2::Save(h_slot Slot, FILE *File) const
    {
    assert(this);
    Bool Good = TRUE;

    fseek(File, Slot * GetSize(), SEEK_SET);

    if (fwrite(GetPointer(), 1, GetSize(), File) != GetSize())
        {
#ifndef WINCIT
        mPrintf(getmsg(661), hall2Dat);
#endif
        Good = FALSE;
        }

    return (Good);
    }

Bool HallEntry3::Save(h_slot Slot, FILE *File) const
    {
    assert(this);
    Bool Good = TRUE;
    fseek(File, (cfg.maxhalls + Slot) * GetSize(), SEEK_SET);

    if (fwrite(GetPointer(), 1, GetSize(), File) != GetSize())
        {
#ifndef WINCIT
        mPrintf(getmsg(661), hall2Dat);
#endif
        Good = FALSE;
        }

    return (Good);
    }

Bool HallEntry::Save(h_slot HallSlot, FILE *File1, FILE *File2) const
    {
    assert(this);

    return (HallEntry1::Save(HallSlot, File1) && HallEntry2::Save(HallSlot, File2) && HallEntry3::Save(HallSlot, File2));
    }


// --------------------------------------------------------------------------
// getRoomPos(): Loads roomPos data into RAM buffer.

Bool getRoomPos(void)
    {
    Bool Good = TRUE;
    FILE *rmposfl;
    char fpath[80];

    sprintf(fpath, sbs, cfg.homepath, roomposDat);

    if (!citOpen(fpath, CO_RB, &rmposfl))
        {
#ifndef WINCIT
        mPrintf(getmsg(78), fpath);
#endif
        Good = FALSE;
        }
    else
        {
        fseek(rmposfl, 0L, SEEK_SET);

        if ((r_slot) fread(roomPos, sizeof(*roomPos), cfg.maxrooms, rmposfl) != cfg.maxrooms)
            {
#ifndef WINCIT
            mPrintf(getmsg(83), fpath);
#endif
            Good = FALSE;
            }

        fclose(rmposfl);

        // Make sure that each room has exactly one entry
        for (r_slot i = 0; Good && i < cfg.maxrooms; i++)
            {
            r_slot j, num;

            for (j = 0, num = 0; j < cfg.maxrooms; j++)
                {
                if (roomPos[j] == i)
                    {
                    num++;
                    }
                }

            if (num != 1)
                {
                Good = FALSE;
                }
            }
        }

    // Something's wrong: recreate it
    if (!Good)
        {
        for (r_slot i = 0; i < cfg.maxrooms; i++)
            {
            roomPos[i] = i;
            }
        }

    return (Good);
    }


// --------------------------------------------------------------------------
// putRoomPos(): Stores roomPos data into ROOMPOS.DAT.

Bool putRoomPos(void)
    {
    Bool Good = TRUE;
    FILE *rmposfl;
    char fpath[80];

    sprintf(fpath, sbs, cfg.homepath, roomposDat);

    if (!citOpen(fpath, CO_WB, &rmposfl))
        {
#ifndef WINCIT
        mPrintf(getmsg(78), fpath);
#endif
        Good = FALSE;
        }
    else
        {
        fseek(rmposfl, 0L, SEEK_SET);

        if ((r_slot) fwrite(roomPos, sizeof(*roomPos), cfg.maxrooms, rmposfl) != cfg.maxrooms)
            {
#ifndef WINCIT
            mPrintf(getmsg(661), fpath);
#endif
            Good = FALSE;
            }

        fclose(rmposfl);
        }

    return (Good);
    }


// --------------------------------------------------------------------------
// writeTables(): Stores all tables to disk.

void writeTables(void)
    {
    FILE *fd;

    changedir(etcpath);

    if ((fd = fopen(etcTab, FO_WB)) == NULL)
        {
        crashout(getmsg(8), etcTab);
        }

    // write out etc.tab
    fwrite(&cfg, sizeof (config), 1, fd);
    fclose(fd);

    changedir(cfg.homepath);

    LogTab.Save();
    MessageDat.SaveTable();
    RoomTab.Save();

    Cron.WriteTable();

    changedir(etcpath);
    }

#ifdef MULTI
Bool TermWindowC::IsGood(void)
    {
    return ((!cfg.accounting || CurrentUserAccount) && LogOrder && CurrentRoom && (!cfg.maxrooms || CurrentUser) &&
			TermCap && Talley && Talley->IsGood() && ScreenBuffer);
    }

void TermWindowCollectionC::Remove(TermWindowC *Old)
	{
	Lock();

	TermWindowListS *cur;
	int T;

	for (T = 1, cur = List; cur; cur = (TermWindowListS *) getNextLL(cur), T++)
		{
		if (cur->T == Old)
			{
			deleteLLNode((void **) &List, T);

			SendDlgItemMessage(MainDlgH, 101, LB_DELETESTRING, T - 1, 0);

			if (T > 1)
				{
				T--;
				}

			if (InFocus(Old))
				{
				cur = (TermWindowListS *) getLLNum(List, T);

				if (cur)
					{
					SetFocus(cur->T);
					}
				else
					{
					TurnOffTermWindow();
					}
				}

			break;
			}
		}

	Unlock();
	}

TermWindowC::~TermWindowC(void)
    {
	TermWindowCollection.Remove(this);

    if (SystemEventMutex)
        {
        // WaitForSingleObject(SystemEventMutex, INFINITE);    // It's not clear if this is good...
        //jrs - ok, let's not wait forever then...
        WaitForSingleObject(SystemEventMutex, 5000);
        CloseHandle(SystemEventMutex);
        SystemEventMutex = NULL;
        }

    delete Talley;
    delete CurrentUser;
    delete TermCap;
    delete CurrentRoom;
    delete MS.AbortedMessage;
    delete CurrentUserAccount;
    delete [] ScreenBuffer;
    delete [] LogOrder;
    freeNode(&node);
    disposeLL((void **) &PendingSystemEvents);

	rmdir(LocalTempPath);
    }

void TermWindowCollectionC::Add(TermWindowC *New)
	{
	Lock();

	TermWindowListS *cur = (TermWindowListS *) addLL((void **) &List, sizeof(TermWindowListS));

	if (cur)
		{
		cur->T = New;
		}

	Unlock();
	}

TermWindowC::TermWindowC(CommunicationPortC *CP, long NewSequence)
    {
    memset(this, 0, sizeof(TermWindowC));

    label semName;
    sprintf(semName, "WincitMutex:%p", this);
    SystemEventMutex = CreateMutex(NULL, FALSE, semName);

	CopyStringToBuffer(WindowCaption, CP->PortName);
	OurConSeq = NewSequence;

#ifdef WINCIT
	LastChat = 1;	// all. cfg.chatmode?
#else
	LastChat = 5;	// console. cfg.chatmode?
#endif
	*LastChatUser = 0;
	*LastChatGroup = 0;

	SendDlgItemMessage(MainDlgH, 101, LB_ADDSTRING, 0, (LPARAM) ((LPSTR) WindowCaption));

	if (cfg.maxrooms)
		{
		CurrentUser = new LogEntry(cfg.maxrooms, cfg.maxgroups, cfg.maxjumpback);
		}

	Talley = new TalleyBufferC(cfg.maxrooms, CurrentUser);

	if (cfg.accounting)
		{
		CurrentUserAccount = new UserAccountInfo;
		}

	CurrentRoom = new RoomC;

	TermCap = new TermCapC(this);

	ScreenBuffer = new CHAR_INFO[cfg.ConsoleRows * cfg.ConsoleCols];// made this configurable, of course
	cls(SCROLL_NOSAVE);

	LogOrder = new l_slot[cfg.MAXLOGTAB];
	if (LogOrder)
		{
		for (l_slot slot = 0; slot < cfg.MAXLOGTAB; slot++)
			{
			LogOrder[slot] = slot;
			}

		LogTab.Sort(&LogOrder);
		}

	if (IsGood())
		{
		// This is overwritten at the start of MainCitadelLoop; it's done there because that's when the TermWindow's
		// thread has started, so we can use the ThreadId to construct a local temp path.
		CopyStringToBuffer(LocalTempPath, cfg.temppath);

		OC.Modem = TRUE;
		OC.Console = TRUE;
		OC.term = this;
		MRO.DotoMessage = NO_SPECIAL;
		OC.Formatting = TRUE;
		OC.UseMCI = cfg.mci;
		showPrompt = TRUE;
		OC.ansiattr = 7;
		CommPort = CP;

		if (!StatusLine.IsVisible())
			{
			StatusLine.Toggle(this);
			}

		time(&LastActiveTime);

		setdefaultconfig(FALSE);

		TermWindowCollection.Add(this);
		}
    }

#else

static void freeFakeTaskInfo(void)
    {
    delete CurrentUser;
    CurrentUser = NULL;

    delete CurrentRoom;
    CurrentRoom = NULL;

    if (Talley)     Talley->~TalleyBufferC();

    delete MS.AbortedMessage;
    MS.AbortedMessage = NULL;

    delete CurrentUserAccount;
    CurrentUserAccount = NULL;

    delete TermCap;
    TermCap = NULL;

    delete CommPort;
    CommPort = NULL;

    freeNode(&node);
    }

Bool initFakeTaskInfo(void)
    {
    Bool good = TRUE;

    // start with fresh task info
    freeFakeTaskInfo();

    if (cfg.accounting)
        {
        if ((CurrentUserAccount = new UserAccountInfo) == NULL)
            {
            good = FALSE;
            }
        }

    if (good && (CurrentRoom = new RoomC) == NULL)
        {
        good = FALSE;
        }

    if (good && cfg.maxrooms)
        {
        CurrentUser = new LogEntry(cfg.maxrooms, cfg.maxgroups, cfg.maxjumpback);

        if (!CurrentUser)
            {
            good = FALSE;
            }
        }

    Talley = new TalleyBufferC();

    if (good)
        {
        TermCap = new TermCapC;

        if (!TermCap)
            {
            good = FALSE;
            }
        }

    if (good)
        {
        CommPort = new SerialPortC;

        if (!CommPort)
            {
            good = FALSE;
            }
        }

    if (!good)
        {
        freeFakeTaskInfo(); // free however much did get allocated
        }
    else
        {
        uchar r, c;
        physReadpos(&r, &c);
        logiRow = r;
        logiCol = c;

        OC.Modem = TRUE;
        OC.Console = TRUE;
        MRO.DotoMessage = NO_SPECIAL;
        OC.Formatting = TRUE;
        OC.UseMCI = cfg.mci;
        showPrompt = TRUE;
        OC.ansiattr = 7;
        }

    return (good);
    }
#endif


// --------------------------------------------------------------------------
// allocateTables(): Allocate Log, Msg, Room, Group, and Hall tables.

void allocateTables(void)
    {
    if (!LogTab.Resize(cfg.MAXLOGTAB))
        {
        crashout(getcfgmsg(116));
        }

    MessageDat.SetTableSize(cfg.nmessages);

    if (!RoomTab.Resize(cfg.maxrooms))
        {
        crashout(getcfgmsg(113));
        }

    GroupData.Resize(cfg.maxgroups);

    HallData.Resize(cfg.maxhalls, cfg.maxrooms);

    delete [] roomPos;
    roomPos = NULL;

    delete [] statList;
    statList = NULL;

    if (// task independent stuff
        !GroupData.IsValid() || !HallData.IsValid() || ((roomPos = new r_slot[cfg.maxrooms]) == NULL) ||
        ((statList = new statRecord[cfg.statnum]) == NULL)

#ifndef MULTI
        || !initFakeTaskInfo()
#endif
        )
        {
        crashout(getcfgmsg(107));
        }
    }


// --------------------------------------------------------------------------
// freeTables(): Deallocate msgTab, logTab, etc.

void freeTables(void)
    {
    LogTab.Resize(0);
    RoomTab.Resize(0);
    MessageDat.SetTableSize(0);

    delete [] roomPos;
    roomPos = NULL;

    delete [] statList;
    statList = NULL;

    GroupData.Resize(0);
    HallData.Resize(0, 0);

    delete [] AccountingData;
    AccountingData = NULL;

#ifndef MULTI
    freeFakeTaskInfo();
#endif
    }

// --------------------------------------------------------------------------
// readMsgTab(): Read MSG.TAB off disk.

Bool MessageDatC::LoadTable(void)
    {
    FILE *fd;

    char FName[128];
    sprintf(FName, sbs, cfg.homepath, "msg.tab");   // mgTab

    if ((fd = fopen(FName, FO_RB)) == NULL)
        {
        return (FALSE);
        }

	Bool RetVal = FALSE;

	if (fread(&catLoc, sizeof(catLoc), 1, fd) == 1)
		{
		if (fread(&SizeInBytes, sizeof(SizeInBytes), 1, fd) == 1)
			{
			if (fread(&OldestMessage, sizeof(OldestMessage), 1, fd) == 1)
				{
				if (Table.Load(fd))
					{
					RetVal = TRUE;
					}
				}
			}
		}

    fclose(fd);
    unlink(FName);

    return (RetVal);
    }


Bool MessageTableC::Load(FILE *fd)
    {
	if ((fread(&NewestMsg, sizeof(NewestMsg), 1, fd) != 1) ||    (fread(&OldestMsgTab, sizeof(OldestMsgTab), 1, fd) != 1))
		{
		return (FALSE);
		}

#ifdef WINCIT
    if (fread(Table, sizeof(*Table), cfg.nmessages + 1, fd) != (size_t) (cfg.nmessages + 1))
        {
        return (FALSE);
        }
#endif

#ifdef REGULAR
    if (
			(fread(Flags, sizeof(*Flags), cfg.nmessages, fd) != cfg.nmessages) ||
			(fread(LocLO, sizeof(*LocLO), cfg.nmessages, fd) != cfg.nmessages) ||
			(fread(LocHI, sizeof(*LocHI), cfg.nmessages, fd) != cfg.nmessages) ||
			(fread(RoomNum, sizeof(*RoomNum), cfg.nmessages, fd) != cfg.nmessages) ||
			(fread(ToHash, sizeof(*ToHash), cfg.nmessages, fd) != cfg.nmessages) ||
			(fread(AuthHash, sizeof(*AuthHash), cfg.nmessages, fd) != cfg.nmessages) ||
			(fread(OriginID, sizeof(*OriginID), cfg.nmessages, fd) != cfg.nmessages))
		{
        return (FALSE);
		}
#endif

#ifdef AUXMEM
    for (m_slot i = 0; i < cfg.nmessages; i += MSGTABPERPAGE)
        {
        MsgTabEntryS *lmt = (MsgTabEntryS *) (LoadAuxmemBlock(i, &mtList, MSGTABPERPAGE, sizeof(MsgTabEntryS)));

        if (fread(lmt, AUXPAGESIZE, 1, fd) != 1)
            {
            return (FALSE);
            }
        }
#endif

#if defined(WINCIT) || defined(AUXMEM)
    if (fread(FirstMessageInRoom, sizeof(*FirstMessageInRoom), cfg.maxrooms, fd) != (size_t) cfg.maxrooms)
        {
        return (FALSE);
        }

    if (fread(LastMessageInRoom, sizeof(*LastMessageInRoom), cfg.maxrooms, fd) != (size_t) cfg.maxrooms)
        {
        return (FALSE);
        }
#endif

    return (TRUE);
    }


// --------------------------------------------------------------------------
// writeMsgTab(): Write MSG.TAB.
// No return value, because not important if succeeded: we'll worry about that on next program load...

void MessageDatC::SaveTable(void)
    {
    FILE *fd;

    char FName[128];
    sprintf(FName, sbs, cfg.homepath, "msg.tab");   // mgTab

    if ((fd = fopen(FName, FO_WB)) != NULL)
        {
		fwrite(&catLoc, sizeof(catLoc), 1, fd);
		fwrite(&SizeInBytes, sizeof(SizeInBytes), 1, fd);
		fwrite(&OldestMessage, sizeof(OldestMessage), 1, fd);

		Table.Save(fd);
        fclose(fd);
        }
    }

void MessageTableC::Save(FILE *fd)
    {
	fwrite(&NewestMsg, sizeof(NewestMsg), 1, fd);
	fwrite(&OldestMsgTab, sizeof(OldestMsgTab), 1, fd);

#ifdef WINCIT
    fwrite(Table, sizeof(*Table), cfg.nmessages + 1, fd);
#endif

#ifdef REGULAR
    fwrite(Flags, sizeof(*Flags), cfg.nmessages, fd);
    fwrite(LocLO, sizeof(*LocLO), cfg.nmessages, fd);
    fwrite(LocHI, sizeof(*LocHI), cfg.nmessages, fd);
    fwrite(RoomNum, sizeof(*RoomNum), cfg.nmessages, fd);
    fwrite(ToHash, sizeof(*ToHash), cfg.nmessages, fd);
    fwrite(AuthHash, sizeof(*AuthHash), cfg.nmessages, fd);
    fwrite(OriginID, sizeof(*OriginID), cfg.nmessages, fd);
#endif

#ifdef AUXMEM
    for (m_slot i = 0; i < cfg.nmessages; i += MSGTABPERPAGE)
        {
        MsgTabEntryS *lmt = (MsgTabEntryS *) (LoadAuxmemBlock(i, &mtList, MSGTABPERPAGE, sizeof(MsgTabEntryS)));
        fwrite(lmt, AUXPAGESIZE, 1 , fd);
        }
#endif

#if defined(WINCIT) || defined(AUXMEM)
    fwrite(FirstMessageInRoom, sizeof(*FirstMessageInRoom), cfg.maxrooms, fd);
    fwrite(LastMessageInRoom, sizeof(*LastMessageInRoom), cfg.maxrooms, fd);
#endif
    }


#ifdef AUXMEM
Bool RoomTable::Resize(r_slot SizeOfTable)
    {
    VerifyHeap();

    if (rtList)
        {
        DeleteAuxmemList(&rtList, &roomBlocksInHeap);
        }


    // Note that this never calls the RTable constructor, but
    // AddAuxmemBlock clears memory to 0. The only other thing the RTable
    // constructor does is set up the Windows mutex. As this does not
    // affect the Auxmem version, this is okay.

    if (SizeOfTable)
        {
        for (r_slot i = 0; i < cfg.maxrooms; i += ROOMTABPERPAGE)
            {
            if (!AddAuxmemBlock(&rtList, i, &roomBlocksInHeap, 1))
                {
                return (FALSE);
                }
            }
        }

    TableSize = SizeOfTable;
    return (TRUE);
    }
#endif

