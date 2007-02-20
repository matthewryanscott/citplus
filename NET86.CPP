// --------------------------------------------------------------------------
// Citadel: Net86.CPP
//
// Citadel-86 style networing routines.

#include "ctdl.h"
#pragma hdrstop

#include "room.h"
#include "tallybuf.h"
#include "auxtab.h"
#include "log.h"
#include "net.h"
#include "filerdwr.h"
#include "extmsg.h"
#include "statline.h"


// --------------------------------------------------------------------------
// Contents
//
// net86_incorporate()
// net86_fetch()


// --------------------------------------------------------------------------
// net86_incorporate()

Bool Net86PacketC::SaveByte(char Byte)
    {
    assert(NetFile);
    return (fputc(Byte, NetFile) != EOF);
    }

Bool Net86PacketC::SaveString(const char *String)
    {
    assert(NetFile);
    assert(String);

    return (PutStr(NetFile, String));
    }

Bool Net86PacketC::GetByte(char *Byte)
    {
    assert(NetFile);

    if (feof(NetFile))
        {
        *Byte = 0;
        return (FALSE);
        }
    else
        {
        *Byte = (char) fgetc(NetFile);
        return (TRUE);
        }
    }


void TERMWINDOWMEMBER net86_incorporate(ModemConsoleE W)
    {
    Message *NewMsg = new Message;
    Message *DupeTest = new Message;

    if (NewMsg && DupeTest)
        {
        int our_new = 0, our_expired = 0, our_duplicate = 0;
        char errorflag = FALSE;
        Bool first = TRUE;

        SetDoWhat(NETWORKING);

        changedir(cfg.transpath);

        const char *PacketMask;

        if (*node->GetExtractPacket())
            {
            PacketMask = node->GetExtractPacket();
            }
        else
            {
            PacketMask = getnetmsg(129);
            }

        directoryinfo *files = filldirectory(PacketMask, SORT_DATE,
                OldAndNew, FALSE);

        if (files)
            {
            for (int FileCounter = 0; *files[FileCounter].Name; FileCounter++)
                {
                char *packetname = files[FileCounter].Name;

                FILE *msgfd;

                if ((msgfd = fopen(packetname, FO_RB)) != NULL)
                    {
                    wDoCR(W);
                    wPrintf(W, getnetmsg(102), cfg.Lmsgs_nym, node->GetName());
                    wDoCR(W);
                    wDoCR(W);
                    wPrintf(W, getnetmsg(66));
                    wDoCR(W);

                    int total_new = 0;
                    int total_expired = 0;
                    int total_duplicate = 0;
                    int total_rooms = 0;

                    KBReady();

                    r_slot prevroomNo = SHRT_MAX;

                    Net86PacketC Net86Packet(msgfd, node->GetName());

                    while (Net86Packet.LoadAll(NewMsg) == RMS_OK)
                        {
                        label netid, here, there;
                        Bool bad = FALSE;
                        Bool More, foundit;
                        r_slot roomNo;

                        // The #ROOM lines in NODES.CIT match their room name
                        // to our NetIDs.
                        for (More = get_first_room(here, there), foundit = FALSE; More && !foundit;
                                More = get_next_room(here, there))
                            {
                            if (SameString(there, NewMsg->GetCreationRoom()))
                                {
                                CopyStringToBuffer(netid, here);
                                foundit = TRUE;

                                // because get_next_room() closes the file
                                // opened by get_first_room() when it is done.
                                while (get_next_room(here, there));
                                }
                            }

                        if (foundit)
                            {
                            roomNo = IdExists(netid, TRUE);
                            }
                        else
                            {
                            roomNo = CERROR;
                            }

                        if (roomNo == CERROR)
                            {
                            if (node->IsVerbose(NCV_NONETIDONSYSTEM))
                                {
                                net69_error(FALSE, errorflag, node->GetName(), getnetmsg(95), netid);
                                errorflag = TRUE;
                                }
                            }

                        if (first && roomNo != CERROR)
                            {
                            prevroomNo = roomNo;
                            first = FALSE;
                            }

                        if (roomNo != prevroomNo && roomNo != CERROR)
                            {
                            label Buffer;
                            wPrintf(W, getnetmsg(131), deansi(RoomTab[prevroomNo].GetName(Buffer, sizeof(Buffer))));

                            if (our_new)
                                {
                                wPrintf(W, getnetmsg(132), our_new);
                                }
                            else
                                {
                                wPrintf(W, getnetmsg(133), our_new);
                                }

                            wPrintf(W, getnetmsg(134));
                            if (our_expired)
                                {
                                wPrintf(W, getnetmsg(132), our_expired);
                                }
                            else
                                {
                                wPrintf(W, getnetmsg(133), our_expired);
                                }

                            wPrintf(W, getnetmsg(134));
                            if (our_duplicate)
                                {
                                wPrintf(W, getnetmsg(132), our_duplicate);
                                }
                            else
                                {
                                wPrintf(W, getnetmsg(133), our_duplicate);
                                }
                            wDoCR(W);

                            total_new += our_new;
                            total_expired += our_expired;
                            total_duplicate += our_duplicate;
                            total_rooms++;
                            our_new = 0;
                            our_expired = 0;
                            our_duplicate = 0;
                            StatusLine.Update(WC_TWp);

                            prevroomNo = roomNo;
                            }

                        if (roomNo == CERROR)
                            {
                            bad = TRUE;
                            }

                        if (SameString(cfg.nodeTitle, NewMsg->GetOriginNodeName()))
                            {
                            bad = TRUE;
                            our_duplicate++;
                            }

                        // Throw away authorless messages
                        if (!*NewMsg->GetAuthor())
                            {
                            bad = TRUE;
                            our_duplicate++;
                            }

                        // Expired?
                        if (!bad && cfg.expire &&
                                (labs(atol(NewMsg->GetCreationTime()) - time(NULL)) > (cfg.expire * SECSINDAY)))
                            {
                            bad = TRUE;
                            our_expired++;
                            }

                        // is public
                        if (!bad)
                            {
                            const ushort oid = (ushort) atol(NewMsg->GetSourceID());

                            for (m_index ID = MessageDat.NewestMessage(); ID >= MessageDat.OldestMessageInTable() && !bad;
                                    ID--)
                                {
                                if (oid == MessageDat.GetOriginID(ID))
                                    {
                                    MessageDat.LoadDupeCheck(ID, DupeTest);

                                    if (    // If same author or node
                                            (
                                            SameString(NewMsg->GetAuthor(), DupeTest->GetAuthor())

                                            ||

                                            SameString(NewMsg->GetOriginNodeName(), DupeTest->GetOriginNodeName())
                                            )

                                            &&

                                            // and same time
                                            SameString(NewMsg->GetCreationTime(), DupeTest->GetCreationTime())

                                            &&

                                            // and same source id
                                            SameString(NewMsg->GetSourceID(), DupeTest->GetSourceID())
                                            )
                                        {
                                        bad = TRUE;
                                        our_duplicate++;
                                        }
                                    }
                                }
                            }

                        if (!bad)
                            {                   // its good, save it
                            NewMsg->SetRoomNumber(roomNo);
                            NewMsg->SetCreationRoom();
                            putAndNoteMessage(NewMsg, TRUE);
                            our_new++;
                            }
                        }

                    if (prevroomNo != CERROR)
                        {
                        label Buffer;
                        wPrintf(W, getnetmsg(131), deansi(RoomTab[prevroomNo].GetName(Buffer, sizeof(Buffer))));

                        if (our_new)
                            {
                            wPrintf(W, getnetmsg(132), our_new);
                            }
                        else
                            {
                            wPrintf(W, getnetmsg(133), our_new);
                            }
                        wPrintf(W, getnetmsg(134));

                        if (our_expired)
                            {
                            wPrintf(W, getnetmsg(132), our_expired);
                            }
                        else
                            {
                            wPrintf(W, getnetmsg(133), our_expired);
                            }
                        wPrintf(W, getnetmsg(134));

                        if (our_duplicate)
                            {
                            wPrintf(W, getnetmsg(132), our_duplicate);
                            }
                        else
                            {
                            wPrintf(W, getnetmsg(133), our_duplicate);
                            }
                        wDoCR(W);

                        total_new += our_new;
                        total_expired += our_expired;
                        total_duplicate += our_duplicate;
                        total_rooms++;
                        our_new = 0;
                        our_expired = 0;
                        our_duplicate = 0;
                        StatusLine.Update(WC_TWp);
                        }

                    wPrintf(W, getnetmsg(135));
                    wDoCR(W);
                    wPrintf(W, getnetmsg(136));

                    if (total_rooms)
                        {
                        wPrintf(W, getnetmsg(132), total_rooms);
                        }
                    else
                        {
                        wPrintf(W, getnetmsg(133), total_rooms);
                        }
                    wPrintf(W, getnetmsg(134));

                    if (total_new)
                        {
                        wPrintf(W, getnetmsg(132), total_new);
                        }
                    else
                        {
                        wPrintf(W, getnetmsg(133), total_new);
                        }
                    wPrintf(W, getnetmsg(134));


                    if (total_expired)
                        {
                        wPrintf(W, getnetmsg(132), total_expired);
                        }
                    else
                        {
                        wPrintf(W, getnetmsg(133), total_expired);
                        }
                    wPrintf(W, getnetmsg(134));

                    if (total_duplicate)
                        {
                        wPrintf(W, getnetmsg(132), total_duplicate);
                        }
                    else
                        {
                        wPrintf(W, getnetmsg(133), total_duplicate);
                        }
                    wDoCR(W);

                    fclose(msgfd);
                    changedir(cfg.transpath);
                    unlink(packetname);
                    }
                else
                    {
                    wDoCR(W);
                    wPrintf(W, getmsg(78), packetname);
                    wDoCR(W);
                    }
                }

            delete [] files;
            }
        }
    else
        {
        OutOfMemory(89);
        }

    delete NewMsg;
    delete DupeTest;
    }


// --------------------------------------------------------------------------
// net86_fetch(void)

Bool TERMWINDOWMEMBER net86_fetch(ModemConsoleE W)
    {
    label   netid;
    FILE    *msgfd;
    int     msgsread;
    int     crcounter = 0;
    char    packetname[20];
    char    errorflag = FALSE;
    int     i;
    int     oldmread = MS.Read;
    MS.Read = 0;

    SetDoWhat(NETWORKING);

    changedir(cfg.transpath);

    Talley->Fill();

    if (*node->GetCreatePacket())
        {
        CopyStringToBuffer(packetname, node->GetCreatePacket());
        }
    else
        {
        sprintf(packetname, getnetmsg(138), CurrentUser->GetPoopcount() % 1000);
        }

    if ((msgfd = fopen(packetname, FO_AB)) == NULL)
        {
        wDoCR(W);
        wPrintf(W, getmsg(78), packetname);
        wDoCR(W);

        if (W == MODEM)
            {
            MS.Read = oldmread;
            }

        return (FALSE);
        }

    wDoCR(W);

    wPrintf(W, getnetmsg(73), cfg.Lmsgs_nym);

    wDoCR(W);
    wDoCR(W);

    wPrintf(W, getnetmsg(76));

    wDoCR(W);

    for (i = get_first_room(netid, c86_there); i; i = get_next_room(netid, c86_there))
        {
        KBReady();

        const r_slot roomNo = IdExists(netid, TRUE);

        if (roomNo != CERROR)
            {
            if (CurrentUser->CanAccessRoom(roomNo))
                {
                if (Talley->NewInRoom(roomNo))
                    {
                    label Buffer;
                    wPrintf(W, getnetmsg(139), deansi(RoomTab[roomNo].GetName(Buffer, sizeof(Buffer))));

                    msgsread = NewRoom(roomNo, NULL, msgfd);

                    wPrintf(W, getnetmsg(140), msgsread);

                    if (!((crcounter + 1) % 2))
                        {
                        wDoCR(W);
                        }

                    crcounter++;
                    }
                }
            else
                {
                if (node->IsVerbose(NCV_NOACCESS))
                    {
                    label Buffer;
                    net69_error(TRUE, errorflag, node->GetName(), getnetmsg(111),
                            RoomTab[roomNo].GetName(Buffer, sizeof(Buffer)));
                    errorflag = TRUE;
                    }
                }
            }
        else
            {
            if (node->IsVerbose(NCV_NONETIDONSYSTEM))
                {
                net69_error(TRUE, errorflag, node->GetName(), getnetmsg(95), netid);
                errorflag = TRUE;
                }
            }
        }

    fclose(msgfd);

    wDoCR(W);
    wDoCR(W);
    wPrintf(W, getnetmsg(74), ltoac(MS.Read), (MS.Read == 1) ? cfg.Lmsg_nym : cfg.Lmsgs_nym);
    wDoCR(W);
    wDoCR(W);

    CurrentUser->MorePoop(1);

    return (TRUE);
    }

Bool TERMWINDOWMEMBER Citadel86Network(Bool master)
    {
    if (!master)
        {
        cPrintf(getnetmsg(172));
        return (FALSE);
        }

    label DeleteMask;

    if (*node->GetCreatePacket())
        {
        CopyStringToBuffer(DeleteMask, node->GetCreatePacket());
        }
    else
        {
        strcpy(DeleteMask, getnetmsg(176));
        }

    changedir(cfg.transpath);
    ambigUnlink(DeleteMask);

    net86_incorporate(CONSOLE);

    return (TRUE);
    }


// --------------------------------------------------------------------------
// Cit86_PutMessage(): Puts a message to a file (cit86 style).

Bool Net86PacketC::Store(Message *Msg)
    {
    label c86time, c86date;
    long msgno;

#ifndef WINCIT
    KBReady();
#endif

    if (!*Msg->GetSourceID())
        {
        Msg->SetOriginNodeName(cfg.nodeTitle);
        Msg->SetSourceID(Msg->GetLocalID());
        Msg->SetCit86Country(cfg.Cit86Country);
        Msg->SetOriginPhoneNumber(cfg.nodephone);
        }

    if (!*Msg->GetEZCreationTime())
        {
        label Buffer;
        Msg->SetEZCreationTime(net69_gateway_time(atol(Msg->GetCreationTime()), Buffer));
        }

    sscanf(Msg->GetEZCreationTime(), getnetmsg(113), &c86date, &c86time);

    if (!*Msg->GetOriginPhoneNumber())
        {
        Msg->SetOriginPhoneNumber(getnetmsg(114));
        }

    if (*Msg->GetCit86Country())
        {
        sprintf(Msg->OriginRegion, getnetmsg(113), Msg->GetCit86Country(), Msg->GetOriginPhoneNumber());
        }
    else
        {
        sprintf(Msg->OriginRegion, getnetmsg(115), Msg->GetOriginPhoneNumber());
        }

    for (size_t i = 3; i < strlen(Msg->GetCreationRoom()) + 1; i++)
        {
        Msg->CreationRoom[i - 4] = Msg->CreationRoom[i];
        }

    msgno = atoi(Msg->GetSourceID());

    sprintf(Msg->SourceID, getnetmsg(116), long_HI(msgno), long_LO(msgno));

    stripansi(Msg->Author);
    stripansi(Msg->OriginNodeName);
    stripansi(Msg->OriginRegion);
    stripansi(c86_there);
    stripansi(Msg->Text);

    if (SameString(Msg->GetAuthor(), getmsg(360)))
        {
        fputc('D', NetFile); PutStr(NetFile, getmsg(360));
        }
    else
        {
        fputc('A', NetFile); PutStr(NetFile, Msg->GetAuthor());
        fputc('D', NetFile); PutStr(NetFile, c86date);
        }

    fputc('C', NetFile); PutStr(NetFile, c86time);
    fputc('N', NetFile); PutStr(NetFile, Msg->GetOriginNodeName());
    fputc('O', NetFile); PutStr(NetFile, Msg->GetOriginRegion());
    fputc('R', NetFile); PutStr(NetFile, c86_there);
    fputc('S', NetFile); PutStr(NetFile, Msg->GetSourceID());

    if (*cfg.Cit86Domain)
        {
        fputc('X', NetFile); PutStr(NetFile, deansi(cfg.Cit86Domain));
        }

    // put the message field
    fputc('M', NetFile); PutStr(NetFile, Msg->GetText());

    return (TRUE);
    }


// --------------------------------------------------------------------------
// Cit86_GetMessage()

ReadMessageStatus Net86PacketC::LoadAll(Message *Msg)
    {
#ifndef WINCIT
    KBReady();
#endif

    // clear message buffer out
    Msg->ClearAll();

    char c;
    do
        {
        c = (uchar) fgetc(NetFile);

        switch (c)
            {
            case 'A':
                {
                label Buffer;

                GetString(Buffer, LABELSIZE);
                Msg->SetAuthor(Buffer);
                break;
                }

            case 'C':
                {
                label Buffer;

                GetString(Buffer, LABELSIZE);
                Msg->SetCreationTime(Buffer);
                break;
                }

            case 'D':
                {
                label Buffer;

                GetString(Buffer, LABELSIZE);
                Msg->SetEZCreationTime(Buffer);
                break;
                }

            case 'M': break;    // will be read off disk later

            case 'N':
                {
                label Buffer;

                GetString(Buffer, LABELSIZE);
                Msg->SetOriginNodeName(Buffer);
                break;
                }

            case 'O':
                {
                label Buffer;

                GetString(Buffer, LABELSIZE);
                Msg->SetOriginPhoneNumber(Buffer);
                break;
                }

            case 'R':
                {
                label Buffer;

                GetString(Buffer, LABELSIZE);
                Msg->SetCreationRoom(Buffer);
                break;
                }

            case 'S':
                {
                label Buffer;

                GetString(Buffer, LABELSIZE);
                Msg->SetSourceID(Buffer);
                break;
                }

            case 'X':
                {
                label Buffer;

                GetString(Buffer, LABELSIZE);
                Msg->SetOriginRegion(Buffer);
                break;
                }


            case 0:
                break;

            default:
                {
                // try to store unknown field
                unkLst *lul;

                if ((lul = Msg->addUnknownList()) != NULL)
                    {
                    lul->whatField = c;
                    GetString(lul->theValue, MAXUNKLEN);
                    }
                else
                    {
                    // cannot save it - discard unknown field
                    GetString(Msg->GetTextPointer(), MAXTEXT);
                    Msg->SetText(ns);
                    }
                }
            }
        } while (c != 'M' && !feof(NetFile));

    if (feof(NetFile))
        {
        return (RMS_BADMESSAGE);
        }

    // get the message field
    GetString(Msg->GetTextPointer(), MAXTEXT);

    // Do a bunch of converting

    // Country
    char Country[3];
    strncpy(Country, Msg->GetOriginPhoneNumber(), 2);
    Country[2] = 0;
    Msg->SetOriginCountry(Country);

    // Path
    if (SameString(Msg->GetOriginNodeName(), OtherNode))
        {
        Msg->SetFromPath(Msg->GetOriginNodeName());
        }
    else
        {
        // last node did not originate, make due with what we got...
        char Buffer[80];

        sprintf(Buffer, getnetmsg(119), Msg->GetOriginNodeName(), OtherNode);

        Msg->SetFromPath(Buffer);
        }

    if (SameString(Msg->GetEZCreationTime(), getmsg(360)))
        {
        // Convert anonymous messages

        label Buffer;
        Msg->SetCreationTime(ltoa(time(NULL), Buffer, 10));
        Msg->SetAuthor(getmsg(360));
        }
    else
        {
        // Convert split date
        label Buffer, Buffer2;

        strcpy(Buffer, Msg->GetEZCreationTime());
        strcat(Buffer, spc);
        strcat(Buffer, Msg->GetCreationTime());
        Msg->SetCreationTime(ltoa(net69_time(Buffer), Buffer2, 10));
        }

    // Convert source id formats
    long hi, lo;
    sscanf(Msg->GetSourceID(), getnetmsg(116), &hi, &lo);

    label SourceID;
    ltoa(long_JOIN(lo, hi), SourceID, 10);
    Msg->SetSourceID(SourceID);

    return (RMS_OK);
    }
