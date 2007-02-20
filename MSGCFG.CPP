// --------------------------------------------------------------------------
// Citadel: MsgCfg.CPP
//
// Message-base configuration code.

#include "ctdl.h"
#pragma hdrstop

#include "auxtab.h"
#include "msg.h"
#include "config.h"
#include "extmsg.h"


// --------------------------------------------------------------------------
// Contents
//
// msgInit()        builds message table from msg.dat
// zapMsgFile()     initializes msg.dat
// slidemsgTab()    frees slots at beginning of msg table

// --------------------------------------------------------------------------
// msgInit(): Scans over MSG.DAT builds the table.

#ifdef WINCIT
    #define InitProgressIndicator()                             \
        HWND hPB = GetDlgItem(hConfigDlg, 101);                 \
                                                                \
        if (hPB)                                                \
            {                                                   \
            PostMessage(hPB, PBM_SETRANGE, 0, MAKELPARAM(0, SizeInBytes / 1024));\
            }

    #define UpdateProgressIndicator()                           \
        if (hPB)                                                \
            {                                                   \
            PostMessage(hPB, PBM_SETPOS, ftell(MsgDat) / 1024, 0);\
            }

    #define DoneWithProgressIndicator()                         \
        if (hPB)                                                \
            {                                                   \
            PostMessage(hPB, PBM_SETPOS, SizeInBytes / 1024, 0);\
            }

#else
    #define InitProgressIndicator()                             \
        int loc = 0, nowloc;                                    \
        const long neatoconstant = (SizeInBytes / 16);          \
        const int twirllen = strlen(cfg.twirly);                \
                                                                \
        msgDisp(getcfgmsg(182), cfg.Lmsg_nym);                  \
        doccr();                                                \
                                                                \
        strrev(cfg.twirly);                                     \
                                                                \
        outCon('|');                                            \
                                                                \
        for (nowloc = 0; nowloc < 64; nowloc++)                 \
            {                                                   \
            outCon(cfg.fuelbarempty);                           \
            }                                                   \
                                                                \
        outCon('|');                                            \
        position(logiRow, logiCol - 65);

    #define UpdateProgressIndicator()                           \
        KBReady();                                              \
                                                                \
        nowloc = (int) (ftell(MsgDat) / neatoconstant);     \
        while (loc < nowloc)                                    \
            {                                                   \
            outCon(cfg.fuelbarfull);                            \
            loc++;                                              \
            }                                                   \
                                                                \
        outCon(cfg.twirly[(int) (CurrentMessageNumber % twirllen)]);    \
        position(logiRow, logiCol - 1);

    #define DoneWithProgressIndicator()                         \
        while (loc < 64)                                        \
            {                                                   \
            outCon(cfg.fuelbarfull);                            \
            loc++;                                              \
            }                                                   \
                                                                \
        doccr();                                                \
        label Newest, Oldest;                                   \
        CopyStringToBuffer(Newest, ltoac(Table.GetNewestMessage()));       \
        CopyStringToBuffer(Oldest, ltoac(OldestMessage));       \
                                                                \
        cPrintf(getcfgmsg(180),                                 \
                ltoac(Table.GetNewestMessage() - OldestMessage + 1),       \
                (Table.GetNewestMessage() - OldestMessage + 1 == 1) ?      \
                cfg.Lmsg_nym : cfg.Lmsgs_nym, Oldest, Newest);  \
                                                                \
        doccr();                                                \
        doccr();                                                \
                                                                \
        strrev(cfg.twirly);
#endif


void MessageTableC::Clear(void)
    {
    OldestMsgTab = 0;

    // Shouldn't need this loop, but it's a good idea to do it anyway, probably
    for (m_index Index = 0; Index < Size; Index++)
        {
        GetFlags(Index)->SetInuse(FALSE);
        }
    }

void MessageDatC::BuildTable(void)
    {
    Message *Msg = new Message;

    if (Msg)
        {
        // This Lock() is extra. There better never be a case where we're building the table while another
        // thread's working on this object!
        Lock();

        Table.Clear();

        InitProgressIndicator();

        // Find a message to start with. There has to be a message...
        ReadMessageStatus Status;

        fseek(MsgDat, 0l, SEEK_SET);

        do
            {
            Status = MessageStoreC::LoadAll(Msg);

            if (Status == RMS_NOMESSAGE)
                {
                // As I said, there has to be a message...
                crashout(getcfgmsg(181));
                }
            } while (Status != RMS_OK);

        // We have a message. Good. Put it into the table.
        const m_index FirstMessageFound = atol(Msg->GetLocalID());

        OldestMessage = FirstMessageFound;
        Table.Start(FirstMessageFound);
        Table.Index(Msg);

        // And keep track of where it ended, so we know were to write next.
        catLoc = ftell(MsgDat);

        // Now, we have a table to build onto. Have at it.
        for (;;)
            {
#ifdef WINCIT
            letWindowsMultitask();
#else
            m_index CurrentMessageNumber = 0;
#endif

            UpdateProgressIndicator();

            do
                {
                Status = MessageStoreC::LoadAll(Msg);

                if (Status == RMS_NOMESSAGE)
                    {
                    break;
                    }
                } while (Status != RMS_OK);

            if (Status == RMS_NOMESSAGE)
                {
                break;
                }

            // Okay: we have succeeded in reading another message. Get its
            // number and see if we can use it.
#ifdef WINCIT
            const m_index CurrentMessageNumber = atol(Msg->GetLocalID());
#else
            CurrentMessageNumber = atol(Msg->GetLocalID());
#endif

            if (CurrentMessageNumber == FirstMessageFound)
                {
                // whee! back to where we started.
                break;
                }

            if (CurrentMessageNumber > Table.GetNewestMessage())
                {
                catLoc = ftell(MsgDat);

                // put it on the end of the table
                long HowManyToScroll = CurrentMessageNumber - (Table.GetOldestMessage() + cfg.nmessages - 1);

                if (HowManyToScroll > 0)
                    {
                    Table.Crunch(HowManyToScroll);
                    }

#ifndef WINCIT
                Table.SetNewestMessage(CurrentMessageNumber);
#endif

                Table.Index(Msg);
                }
            else if (CurrentMessageNumber < OldestMessage)
                {
                // new oldest message
                m_index HowManyToInsert = min(
                        (long) (Table.GetOldestMessage() - CurrentMessageNumber),
                        (long) (cfg.nmessages - (Table.GetNewestMessage() - Table.GetOldestMessage() + 1))
                        );

                if (HowManyToInsert > 0)
                    {
                    if (HowManyToInsert > cfg.nmessages)
                        {
                        crashout("asdf");
                        }

                    Table.Slide((m_slot) HowManyToInsert);
                    }

                OldestMessage = CurrentMessageNumber;

                // if we have made enough room for this message, index it.. if not, ignore it as a missing message
                if (CurrentMessageNumber >= Table.GetOldestMessage())
                    {
                    Table.Index(Msg);
                    }
                }
            else if (CurrentMessageNumber < Table.GetOldestMessage())
                {
                // this is a missing message. do nothing with it.
                }
            else
                {
                // this is somewhere in the table...

                // IsValid() cannot fail: we have already checked the
                // range of CurrentMessageNumber ourselves.
                assert(Table.IsValid(CurrentMessageNumber));

                if (!Table.GetFlags(CurrentMessageNumber)->IsInuse())
                    {
                    // this slot is not inuse yet...
                    Table.Index(Msg);
                    }
                else
                    {
                    // slot already in use: just ignore it.
                    }
                }
            }

        DoneWithProgressIndicator();

        // make sure all are in good rooms.
        for (m_index ID = Table.GetOldestMessage(); ID <= Table.GetNewestMessage(); ID++)
            {
            if (Table.GetRoomNum(ID) >= cfg.maxrooms)
                {
                ChangeRoom(ID, (r_slot) (Table.GetFlags(ID)->IsMail() ? MAILROOM : DUMP));
                }
            }

        delete Msg;

        Unlock();
        }
    else
        {
        crashout(getmsg(188), getcfgmsg(167));
        }
    }

// --------------------------------------------------------------------------
// zapMsgFile(): Initializes MSG.DAT.

//brent is eating a peanut butter sandwich.

void MessageDatC::InitializeMessageFile(long NewSizeInBytes)
    {
    // Even though there better be no other thread working on this object while we're initializing the file!
    Lock();

    SizeInBytes = NewSizeInBytes;

    long Written;
    size_t ToWrite;
    size_t BufferSize = 63 * 1024;      // Up to 63K buffer
    char *Buffer = new char[BufferSize];

    while (!Buffer)
        {
        BufferSize -= 1024;

        // At least 1K buffer
        if (!BufferSize)
            {
            illegal(getmsg(188), getcfgmsg(153));
            }

        Buffer = new char[BufferSize];
        }

    memset(Buffer, 0, BufferSize);

    // put first message...

    Buffer[0] = 0xFF;   // Message start
    Buffer[1] = '\0';   // Normal attribute
    Buffer[2] = LOBBY;  // Message lives in Lobby>
    Buffer[3] = '1';    // Message number 1...
    Buffer[4] = '\0';   // ...message number 1

    // Author, Room, Date, Message
    sprintf(Buffer + 5, getcfgmsg(142), cfg.nodeTitle, 0, getcfgmsg(144), 0, time(NULL), 0);

    // A little bit out of place, perhaps... how many blocks to write?
    Written = ToWrite = (size_t) min(SizeInBytes, (long) BufferSize);

    // And write the first block.
    if (fwrite(Buffer, 1, ToWrite, MsgDat) != ToWrite)
        {
        illegal(getcfgmsg(149));
        }

    // Re-clear to remove first message from buffer
    memset(Buffer, 0, BufferSize);

#ifdef WINCIT
    const int DivValue = SizeInBytes / 100;
    const int MaxValue = SizeInBytes / DivValue;

    HWND hPB = GetDlgItem(hCreateDlg, 101);

    if (hPB)
        {
        PostMessage(hPB, PBM_SETRANGE, 0, MAKELPARAM(0, MaxValue));
        PostMessage(hPB, PBM_SETPOS, Written / DivValue, 0);
        }
#else
    cPrintf(getcfgmsg(150), ltoac(SizeInBytes / 1024), bn);
    cPrintf(getcfgmsg(143), ltoac(Written / 1024), br);
#endif

    // And write the rest.
    while (Written < SizeInBytes)
        {
        letWindowsMultitask();

        ToWrite = (size_t) min(SizeInBytes - Written, (long) BufferSize);

        if (fwrite(Buffer, 1, ToWrite, MsgDat) != ToWrite)
            {
            illegal(getcfgmsg(149));
            }

        Written += ToWrite;

#ifdef WINCIT
        if (hPB)
            {
            PostMessage(hPB, PBM_SETPOS, Written / DivValue, 0);
            }
#else
            cPrintf(getcfgmsg(143), ltoac(Written / 1024), br);
#endif
        }


#ifdef WINCIT
    if (hPB)
        {
        PostMessage(hPB, PBM_SETPOS, MaxValue, 0);
        }
#else
    doccr();
    doccr();
#endif

    delete [] Buffer;

//  BuildTable();
    Unlock();
    }


#ifdef AUXMEM
// --------------------------------------------------------------------------
// slidemsgTab(): Frees slots at the beginning of the message table.
//                  Auxmem version.
//
// Notes:
//  this is way boo boo slow, but as it is only called once, and that is when
//  building the message table, we can ignore the slowness in favor of
//  program size: even if the function was just a return, building the
//  message table would still take forever...

void MessageTableC::Slide(m_slot howmany)
    {
    for (m_slot i = cfg.nmessages - 1; i >= howmany; i--)
        {
        memcpy( LoadAuxmemBlock(i, &mtList, MSGTABPERPAGE, sizeof(MsgTabEntryS)),

                LoadAuxmemBlock(i - howmany, &mtList, MSGTABPERPAGE, sizeof(MsgTabEntryS)),

                sizeof(MsgTabEntryS));
        }

    for (m_slot i = 0; i < howmany; i++)
        {
        memset(LoadAuxmemBlock(i, &mtList, MSGTABPERPAGE, sizeof(MsgTabEntryS)), 0, sizeof(MsgTabEntryS));
        }

    OldestMsgTab -= howmany;
    }

#else

#ifdef WINCIT

// --------------------------------------------------------------------------
// slidemsgTab(): Frees slots at the beginning of the message table.
//                  Windows version.

void MessageTableC::Slide(m_slot howmany)
    {
    const m_slot numnuked = cfg.nmessages - howmany;

    memmove(&Table[howmany], Table, numnuked * sizeof(*Table));
    memset(Table, 0, howmany * sizeof(*Table));

    OldestMsgTab -= howmany;
    }

#else

// --------------------------------------------------------------------------
// slidemsgTab(): Frees slots at the beginning of the message table.
//                  Regular version.

void MessageTableC::Slide(m_slot howmany)
    {
    const m_slot numnuked = cfg.nmessages - howmany;

    memmove(&Flags[howmany], Flags, (uint)(numnuked * sizeof(*Flags)));
    memset(Flags, 0, howmany * sizeof(*Flags));

    memmove(&LocLO[howmany], LocLO, (uint)(numnuked * sizeof(*LocLO)));
    memset(LocLO, 0, howmany * sizeof(*LocLO));

    memmove(&LocHI[howmany], LocHI, (uint)(numnuked * sizeof(*LocHI)));
    memset(LocHI, 0, howmany * sizeof(*LocHI));

    memmove(&RoomNum[howmany], RoomNum, (uint)(numnuked * sizeof(*RoomNum)));
    memset(RoomNum, 0, howmany * sizeof(*RoomNum));

    memmove(&ToHash[howmany], ToHash, (uint)(numnuked * sizeof(*ToHash)));
    memset(ToHash, 0, howmany * sizeof(*ToHash));

    memmove(&AuthHash[howmany], AuthHash, (uint)(numnuked * sizeof(*AuthHash)));
    memset(AuthHash, 0, howmany * sizeof(*AuthHash));

    memmove(&OriginID[howmany], OriginID, (uint)(numnuked * sizeof(*OriginID)));
    memset(OriginID, 0, howmany * sizeof(*OriginID));

    OldestMsgTab -= howmany;
    }
#endif
#endif


// this function also sets SizeInBytes
Bool MessageDatC::OpenMessageFile(const char *Directory)
    {
    Lock();

    assert(!MsgDat);

    Bool RetVal;

    if (MsgDat)
        {
        RetVal = TRUE;
        }
    else
        {
        sprintf(MsgFilePath, sbs, Directory, msgDat);
        RetVal = citOpen(MsgFilePath, CO_RPB, &MsgDat);
        if (RetVal)
            {
            SizeInBytes = filelength(fileno(MsgDat));
            }
        }

    Unlock();

    return (RetVal);
    }

Bool MessageDatC::CreateMessageFile(const char *Directory)
    {
    Lock();

    assert(!MsgDat);

    Bool RetVal;

    if (MsgDat)
        {
        RetVal = TRUE;
        }
    else
        {
        sprintf(MsgFilePath, sbs, Directory, msgDat);
        RetVal = citOpen(MsgFilePath, CO_WPB, &MsgDat);
        }

    Unlock();

    return (RetVal);
    }
