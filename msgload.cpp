// --------------------------------------------------------------------------
// Citadel: MsgLoad.CPP
//
// Stuff for loading messages

#include "ctdl.h"
#pragma hdrstop

#include "msg.h"
#include "filerdwr.h"

// --------------------------------------------------------------------------
// Contents
//

// --------------------------------------------------------------------------
// getMsgStr(): reads a NULL terminated string from msg file
//
// returns:
//  TRUE if all is cool.
//  FALSE if an 0xff is found.

// --------------------------------------------------------------------------
// getMsgChar(): Reads a character from msg file, curent position.

Bool MessageDatC::GetByte(char *Byte)
    {
    if (fread(Byte, sizeof(*Byte), 1, MsgDat) != 1)
        {
        fseek(MsgDat, 0, SEEK_SET);
        return (fread(Byte, sizeof(*Byte), 1, MsgDat) == 1);
        }

    return (TRUE);
    }

Bool MessageStoreC::GetString(char *String, int BufferLength)
    {
    return (GetString(String, BufferLength, FALSE));
    }

// Reads entire string (terminated by 0x00 or 0xff), saves up to BufferLength-1 chars in String, terminates with 0x00
Bool MessageStoreC::GetString(char *String, int BufferLength, Bool FilterControlB)
    {
    assert(BufferLength);

    char c;
    int NextCharIndex = 0;

    do
        {
        if (!GetByte(&c))
            {
            String[NextCharIndex] = 0;
            return (FALSE);
            }

        if ((!FilterControlB || c != 2) && (NextCharIndex < (BufferLength - 1)))
            {
            String[NextCharIndex++] = c;
            }
        } while (c != 0 && c != -1);

    String[NextCharIndex] = 0;
    return (TRUE);
    }

ReadMessageStatus MessageStoreC::LoadAll(Message *Msg)
    {
    // ReadHeader() does a ClearAll()
    ReadMessageStatus Status = LoadHeader(Msg);

    if (Status == RMS_OK)
        {
        if (!GetString(Msg->GetTextPointer(), MAXTEXT))
            {
            Status = RMS_BADMESSAGE;
            }
        }

    return (Status);
    }


// --------------------------------------------------------------------------
// Message::ReadHeader(): Read a message header off disk into RAM.
//
// Input:
//  ReadMessageCode HowToRead: How to do it
//  FILE *NetFile: NULL if reading normal or for duplication checking; file

ReadMessageStatus MessageStoreC::LoadHeader(Message *Msg)
    {
    return (LoadHeader(Msg, FALSE));
    }

ReadMessageStatus MessageStoreC::LoadHeader(Message *Msg, Bool DupeCheck)
    {
    Bool MavenAnon = FALSE;

#ifndef WINCIT
    if (NetPacket)
        {
        KBReady();
        }
#endif

    // clear message buffer out
    Msg->ClearAll();

    // look 10000 chars for start message
    int i;
    for (i = 0; i < 10000; ++i)
        {
        char c;

        if (!GetByte(&c))
            {
            return (RMS_NOMESSAGE);
            }

        if (c == -1)
            {
            break;
            }
        }

    if (i == 10000)
        {
        return (RMS_NOMESSAGE);
        }

    if (!NetPacket)
        {
        Msg->SetHeadLoc(GetStoreOffset() - 1l);
        }

    char ReadByte;

    // get message's attribute byte
    GetByte(&ReadByte);

    if (ReadByte == -1)
        {
        return (RMS_BADMESSAGE);
        }

    Msg->SetAttribute(ReadByte);

    Msg->SetCompressed(Msg->IsCompressed());    // Sets onceWasCompressed;

    if (Msg->GetAttribute() & ATTR_MORE)
        {
        GetByte(&ReadByte);

        if (ReadByte == -1)
            {
            return (RMS_BADMESSAGE);
            }

        Msg->SetAttribute2(ReadByte);
        }

    if (!NetPacket)
        {
        // get message's room #
        uchar roomno_lo;
        GetByte((char *) &roomno_lo);

        if (roomno_lo == 0xff)
            {
            return (RMS_BADMESSAGE);
            }

        if (Msg->IsBigRoom())
            {
            uchar roomno_hi;

            GetByte((char *) &roomno_hi);

            if (roomno_hi == 0xff)
                {
                return (RMS_BADMESSAGE);
                }

            Msg->SetRoomNumber((r_slot) (short_JOIN(roomno_lo, roomno_hi) / 2));
            }
        else
            {
            Msg->SetRoomNumber((r_slot) roomno_lo);
            }
        }

    if (!GetString(Msg->LocalID, sizeof(Msg->LocalID)))
        {
        return (RMS_BADMESSAGE);
        }

    // Local IDs have to be numeric, and non-zero if from MSG.DAT
    if (
            !isNumeric(Msg->GetLocalID()) ||

                (
                !NetPacket && !atol(Msg->GetLocalID())
                )
        )
        {
        return (RMS_BADMESSAGE);
        }

    int FoundDupFields = 0;

    do
        {
        GetByte(&ReadByte);

        switch (ReadByte)
            {
            case 'A':
                {
                if (!GetString(Msg->Author, sizeof(Msg->Author), NetPacket))
                    {
                    return (RMS_BADMESSAGE);
                    }

                FoundDupFields++;

                break;
                }

            case 'a':
                {
                MavenAnon = TRUE;

                label Bogus;
                GetString(Bogus, sizeof(Bogus));
                break;
                }

            case 'B':
                {
                if (!GetString(Msg->Subject, sizeof(Msg->Subject), NetPacket))
                    {
                    return (RMS_BADMESSAGE);
                    }

                break;
                }

            case 'C':
                {
                if (!GetString(Msg->CopyOfMessage, sizeof(Msg->CopyOfMessage), NetPacket))
                    {
                    return (RMS_BADMESSAGE);
                    }

                break;
                }

            case 'D':
                {
                if (!GetString(Msg->CreationTime, sizeof(Msg->CreationTime), NetPacket))
                    {
                    return (RMS_BADMESSAGE);
                    }

                FoundDupFields++;

                break;
                }

            case 'd':
                {
                if (NetPacket)
                    {
                    if (!GetString(Msg->EZCreationTime, sizeof(Msg->EZCreationTime), NetPacket))
                        {
                        return (RMS_BADMESSAGE);
                        }
                    }

                break;
                }

            case 'F':
                {
                if (!GetString(Msg->Forward, sizeof(Msg->Forward), NetPacket))
                    {
                    return (RMS_BADMESSAGE);
                    }

                break;
                }

            case 'G':
                {
                if (!GetString(Msg->Group, sizeof(Msg->Group), NetPacket))
                    {
                    return (RMS_BADMESSAGE);
                    }

                break;
                }

            case 'I':
                {
                if (!GetString(Msg->ReplyToMessage, sizeof(Msg->ReplyToMessage), NetPacket))
                    {
                    return (RMS_BADMESSAGE);
                    }

                break;
                }

            case 'J':
                {
                if (!GetString(Msg->TwitRegion, sizeof(Msg->TwitRegion), NetPacket))
                    {
                    return (RMS_BADMESSAGE);
                    }

                break;
                }

            case 'j':
                {
                if (!GetString(Msg->TwitCountry, sizeof(Msg->TwitCountry), NetPacket))
                    {
                    return (RMS_BADMESSAGE);
                    }

                break;
                }

            case 'L':
                {
                if (!NetPacket)
                    {
                    if (!GetString(Msg->FileLink, sizeof(Msg->FileLink), NetPacket))
                        {
                        return (RMS_BADMESSAGE);
                        }
                    }
                else
                    {
                    char Bogus[64];

                    if (!GetString(Bogus, sizeof(Bogus), NetPacket))
                        {
                        return (RMS_BADMESSAGE);
                        }
                    }

                break;
                }

            case 'l':
                {
                if (!NetPacket)
                    {
                    if (!GetString(Msg->LinkedApplication, sizeof(Msg->LinkedApplication), NetPacket))
                        {
                        return (RMS_BADMESSAGE);
                        }
                    }
                else
                    {
                    char Bogus[128];

                    if (!GetString(Bogus, sizeof(Bogus), NetPacket))
                        {
                        return (RMS_BADMESSAGE);
                        }
                    }

                break;
                }

            case 'M': break;    // will be read off disk later

            case 'N':
                {
                if (!GetString(Msg->Title, sizeof(Msg->Title), NetPacket))
                    {
                    return (RMS_BADMESSAGE);
                    }

                break;
                }

            case 'n':
                {
                if (!GetString(Msg->Surname, sizeof(Msg->Surname), NetPacket))
                    {
                    return (RMS_BADMESSAGE);
                    }

                break;
                }

            case 'O':
                {
                if (!GetString(Msg->OriginNodeName, sizeof(Msg->OriginNodeName), NetPacket))
                    {
                    return (RMS_BADMESSAGE);
                    }

                FoundDupFields++;

                break;
                }

            case 'o':
                {
                if (!GetString(Msg->OriginRegion, sizeof(Msg->OriginRegion), NetPacket))
                    {
                    return (RMS_BADMESSAGE);
                    }

                break;
                }

            case 'P':
                {
                if (!GetString(Msg->FromPath, sizeof(Msg->FromPath), NetPacket))
                    {
                    return (RMS_BADMESSAGE);
                    }

                break;
                }

            case 'p':
                {
                if (!GetString(Msg->ToPath, sizeof(Msg->ToPath), NetPacket))
                    {
                    return (RMS_BADMESSAGE);
                    }

                break;
                }

            case 'Q':
                {
                if (!GetString(Msg->OriginCountry, sizeof(Msg->OriginCountry), NetPacket))
                    {
                    return (RMS_BADMESSAGE);
                    }

                break;
                }

            case 'q':
                {
                if (!GetString(Msg->ToCountry, sizeof(Msg->ToCountry), NetPacket))
                    {
                    return (RMS_BADMESSAGE);
                    }

                break;
                }

            case 'R':
                {
                if (!GetString(Msg->CreationRoom, sizeof(Msg->CreationRoom), NetPacket))
                    {
                    return (RMS_BADMESSAGE);
                    }

                break;
                }

            case 'r':
                {
                if (!GetString(Msg->SourceRoomName, sizeof(Msg->SourceRoomName), NetPacket))
                    {
                    return (RMS_BADMESSAGE);
                    }

                break;
                }

            case 'S':
                {
                if (!GetString(Msg->SourceID, sizeof(Msg->SourceID), NetPacket))
                    {
                    return (RMS_BADMESSAGE);
                    }

                FoundDupFields++;

                break;
                }

            case 's':
                {
                if (!GetString(Msg->OriginSoftware, sizeof(Msg->OriginSoftware), NetPacket))
                    {
                    return (RMS_BADMESSAGE);
                    }

                break;
                }

            case 'T':
                {
                if (!GetString(Msg->ToUser, sizeof(Msg->ToUser), NetPacket))
                    {
                    return (RMS_BADMESSAGE);
                    }

                break;
                }

            case 't':
                {
                if (!GetString(Msg->PublicToUser, sizeof(Msg->PublicToUser), NetPacket))
                    {
                    return (RMS_BADMESSAGE);
                    }

                break;
                }

            case 'U':
                {
                if (!GetString(Msg->Cit86Country, sizeof(Msg->Cit86Country), NetPacket))
                    {
                    return (RMS_BADMESSAGE);
                    }

                break;
                }

            case 'X':
                {
                if (!NetPacket)
                    {
                    if (!GetString(Msg->X, sizeof(Msg->X), NetPacket))
                        {
                        return (RMS_BADMESSAGE);
                        }
                    }

                break;
                }

            case 'Z':
                {
                if (!GetString(Msg->ToNodeName, sizeof(Msg->ToNodeName), NetPacket))
                    {
                    return (RMS_BADMESSAGE);
                    }

                break;
                }

            case 'z':
                {
                if (!GetString(Msg->ToRegion, sizeof(Msg->ToRegion), NetPacket))
                    {
                    return (RMS_BADMESSAGE);
                    }

                break;
                }

            case '.':
                {
                if (!GetString(Msg->Signature, sizeof(Msg->Signature), NetPacket))
                    {
                    return (RMS_BADMESSAGE);
                    }

                break;
                }

            case '_':
                {
                if (!GetString(Msg->UserSignature, sizeof(Msg->UserSignature), NetPacket))
                    {
                    return (RMS_BADMESSAGE);
                    }

                break;
                }

            case '!':
                {
                if (!GetString(Msg->RealName, sizeof(Msg->RealName), NetPacket))
                    {
                    return (RMS_BADMESSAGE);
                    }

                break;
                }

            case 'H':
                {
                if (!GetString(Msg->OriginPhoneNumber, sizeof(Msg->OriginPhoneNumber), NetPacket))
                    {
                    return (RMS_BADMESSAGE);
                    }

                break;
                }

            case 'h':
                {
                if (!GetString(Msg->ToPhoneNumber, sizeof(Msg->ToPhoneNumber), NetPacket))
                    {
                    return (RMS_BADMESSAGE);
                    }

                break;
                }

            case '>':
                {
                if (!GetString(Msg->DestinationAddress, sizeof(Msg->DestinationAddress), NetPacket))
                    {
                    return (RMS_BADMESSAGE);
                    }

                break;
                }

            case '%':
                {
                char String[256];

                if (!GetString(String, sizeof(String), NetPacket))
                    {
                    return (RMS_BADMESSAGE);
                    }

                if (*String)
                    {
                    Msg->AddComment(String);
                    }

                break;
                }

            case '#':
                {
                if (!GetString(Msg->MoreFlags, sizeof(Msg->MoreFlags), NetPacket))
                    {
                    return (RMS_BADMESSAGE);
                    }

                break;
                }

            case '\0':
                {
                return (RMS_BADMESSAGE);
                }

            default:
                {
                // try to store unknown field
                unkLst *lul;

                if ((lul = Msg->addUnknownList()) != NULL)
                    {
                    lul->whatField = ReadByte;

                    if (!GetString(lul->theValue, sizeof(lul->theValue), NetPacket))
                        {
                        return (RMS_BADMESSAGE);
                        }
                    }
                else
                    {
                    // cannot save it - discard unknown field
                    if (!GetString(Msg->Text, sizeof(Msg->Text), NetPacket))
                        {
                        return (RMS_BADMESSAGE);
                        }

                    *Msg->Text = 0;
                    }
                }
            }

        if (DupeCheck)
            {
            if (FoundDupFields == 4)
                {
                break;
                }
            }
        } while (ReadByte != 'M');

    if (MavenAnon)
        {
        Msg->SetAuthor(getmsg(360));
        *Msg->UserSignature = 0;
        *Msg->Title = 0;
        *Msg->Surname = 0;
        *Msg->RealName = 0;
        }

    if (!NetPacket)
        {
        Msg->SetOriginalAttribute(Msg->GetAttribute());
        Msg->SetOriginalID(atol(Msg->GetLocalID()));
        Msg->SetOriginalRoom(Msg->GetRoomNumber());
        Msg->SetViewDuplicate(FALSE);   // will be set to TRUE when we need it
        }

    return (RMS_OK);
    }

ReadMessageStatus MessageDatC::LoadAll(m_index Index, Message *Msg)
    {
    Lock();

    SeekMessage(Index);
    ReadMessageStatus RMS = MessageStoreC::LoadAll(Msg);

    Unlock();
    return (RMS);
    }

ReadMessageStatus MessageDatC::LoadHeader(m_index Index, Message *Msg)
    {
    Lock();

    SeekMessage(Index);

    ReadMessageStatus RMS = MessageStoreC::LoadHeader(Msg);

    Unlock();
    return (RMS);
    }

ReadMessageStatus MessageDatC::LoadDupeCheck(m_index Index, Message *Msg)
    {
    Lock();

    SeekMessage(Index);
    ReadMessageStatus RMS = MessageStoreC::LoadHeader(Msg, TRUE);

    Unlock();
    return (RMS);
    }
