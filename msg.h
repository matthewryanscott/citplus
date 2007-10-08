// --------------------------------------------------------------------------
// Citadel: Msg.H
//
// All about messages.

#include "auxtab.h"
#include "group.h"
#include "log.h"
#include "room.h"

enum ReadMessageStatus
    {
    RMS_OK,     RMS_NOMESSAGE,  RMS_BADMESSAGE,
    };


enum IncorporateStatusE
	{
	IS_OK,			IS_DUPLICATE,	IS_NOAUTHOR,	IS_EXPIRED,		IS_NOACCESS,
	IS_OUTOFMEMORY,	IS_SCRIPT,		IS_SAVENETMAIL,	IS_NOSAVENETMAIL,IS_NETCOMMAND,
	IS_MAILINGLIST,	IS_USERNOTFOUND,

	IS_NUM
	};


// --------------------------------------------------------------------------
// These are flags used in the message table, which is an index into MSG.DAT.
// This table is stored in MSG.TAB.

class msgflags
    {
    Bool    MAIL        : 1;
    Bool    RECEIVED    : 1;    // Was it received?
    Bool    REPLY       : 1;    // Reply sent?
    Bool    PROBLEM     : 1;    // Problem User message?
    Bool    MADEVIS     : 1;    // Made Visible?
    Bool    LIMITED     : 1;    // Limited-access message?
    Bool    MODERATED   : 1;    // Moderated message?
    Bool    reserved    : 1;    // Extra bit (was Released)
    Bool    COPY        : 1;    // Is message a copy?
    Bool    NET         : 1;    // Networked mail en-route?
    Bool    FORWARDED   : 1;    // Forwarded E-Mail?
    Bool    NETWORKED   : 1;    // Networked Message?
    Bool    MASSEMAIL   : 1;    // E-Mail to a group?
    Bool    CENSORED    : 1;    // Censored message?
    Bool    LOCAL       : 1;    // Local message?
    Bool    INUSE       : 1;    // Slot being used by a valid message?

public:
    void SetMail(Bool New)          {   MAIL = !!New;       }
    Bool IsMail(void) const         {   return (MAIL);      }

    void SetReceived(Bool New)      {   RECEIVED = !!New;   }
    Bool IsReceived(void) const     {   return (RECEIVED);  }

    void SetReply(Bool New)         {   REPLY = !!New;      }
    Bool IsReply(void) const        {   return (REPLY);     }

    void SetProblem(Bool New)       {   PROBLEM = !!New;    }
    Bool IsProblem(void) const      {   return (PROBLEM);   }

    void SetMadevis(Bool New)       {   MADEVIS = !!New;    }
    Bool IsMadevis(void) const      {   return (MADEVIS);   }

    void SetLimited(Bool New)       {   LIMITED = !!New;    }
    Bool IsLimited(void) const      {   return (LIMITED);   }

    void SetModerated(Bool New)     {   MODERATED = !!New;  }
    Bool IsModerated(void) const    {   return (MODERATED); }

    void SetCopy(Bool New)          {   COPY = !!New;       }
    Bool IsCopy(void) const         {   return (COPY);      }

    void SetNet(Bool New)           {   NET = !!New;        }
    Bool IsNet(void) const          {   return (NET);       }

    void SetForwarded(Bool New)     {   FORWARDED = !!New;  }
    Bool IsForwarded(void) const    {   return (FORWARDED); }

    void SetNetworked(Bool New)     {   NETWORKED = !!New;  }
    Bool IsNetworked(void) const    {   return (NETWORKED); }

    void SetMassemail(Bool New)     {   MASSEMAIL = !!New;  }
    Bool IsMassemail(void) const    {   return (MASSEMAIL); }

    void SetCensored(Bool New)      {   CENSORED = !!New;   }
    Bool IsCensored(void) const     {   return (CENSORED);  }

    void SetLocal(Bool New)         {   LOCAL = !!New;      }
    Bool IsLocal(void) const        {   return (LOCAL);     }

    void SetInuse(Bool New)         {   INUSE = !!New;      }
    Bool IsInuse(void) const        {   return (INUSE);     }
    };


// --------------------------------------------------------------------------
// And this is the actual message table. Note that we only have a structure
// for the Auxmem and Windows version of Citadel: the table is implemented as
// simple pointers to ints in the Regular version. This is because of
// segments. Because we do not want to suffer the overhead of huge pointers,
// we want to make sure that we do not go over 64K on an array. By breaking
// the message table up into ints, we can have 32K messages and stay within a
// segment. Under Win32, everything is a 32-bit pointer, so there's no worry
// about pointer size.
//
// This table is not really what it seems: because it is so big, we have let
// readability suffer and re-use some of the fields to store certain elements.
// The reuse is as following:
//
// Copied messages: The offset to the original message is stored in both the
//                  mttohash field and the mtauthhash field. We can get away
//                  with this because copied messages never have authors or
//                  recipients.
//
//                  The implementation of this is hidden behind the getCopy()
//                  macro for reading the value. You have to deal with
//                  writing the data yourself. Cope.
//
// Origin ID:       We store only the low 16 bits of the origin ID. This is
//					plenty enough for accuracy.

struct MsgTabEntryS
    {
    msgflags Flags;
    long    Loc;
    r_slot  RoomNum;
    short   ToHash;
    short   AuthHash;
    ushort  OriginID;
#if defined(AUXMEM) || defined(WINCIT)
    m_index PrevRoomMsg;
    m_index NextRoomMsg;

    DEBUGCODE(Bool Checked : 1;)
#endif

    m_index GetCopyOffset(void)
        {
        return (long_JOIN(ToHash, AuthHash));
        }
    };

#if defined(AUXMEM) || defined(WINCIT)
class MessageTableC
    {
    MsgTabEntryS Bogus;

	m_slot Size;

    m_index NewestMsg;          // 32-bit ID# of last message in system
    m_index OldestMsgTab;       // oldest message ID # in message table

    m_index *FirstMessageInRoom;        // array of table indexes
    m_index *LastMessageInRoom;         // array of table indexes

    #ifdef WINCIT
        MsgTabEntryS *Table;            // the message table
    #else
        auxTabList  *mtList;            // base of our linked list
        int         msgBlocksInHeap;    // how many we have

        #define MSGTABPERPAGE (AUXPAGESIZE / sizeof(MsgTabEntryS))
    #endif
	
public:
    MsgTabEntryS *GetTabEntry(m_index Index)
        {
        assert (Index != M_INDEX_ERROR);

        m_slot Slot = Index - OldestMsgTab;

        MsgTabEntryS *ToRet;
        if (Slot >= 0 && Slot < cfg.nmessages)
            {
#ifdef WINCIT
            ToRet = &(Table[Slot]);
#else
            ToRet = (MsgTabEntryS *) (LoadAuxmemBlock(Slot, &mtList, MSGTABPERPAGE, sizeof(MsgTabEntryS)));
#endif
            }
        else
            {
            ToRet = &Bogus;
            }

        return (ToRet);
        }

    MessageTableC(void)
        {
#ifdef WINCIT
        Table = NULL;
#else
        mtList = NULL;
        msgBlocksInHeap = 0;
#endif

		Bogus.Flags;
		Bogus.Loc = -1;
		Bogus.RoomNum = CERROR;
		Bogus.ToHash = 0;
		Bogus.AuthHash = 0;
		Bogus.OriginID = CERROR;
		Bogus.PrevRoomMsg = CERROR;
		Bogus.NextRoomMsg = CERROR;

        FirstMessageInRoom = LastMessageInRoom = NULL;
        }

    ~MessageTableC(void)
        {
        delete [] FirstMessageInRoom;
        delete [] LastMessageInRoom;
#ifdef WINCIT
        delete [] Table;
#else
        DeleteAuxmemList(&mtList, &msgBlocksInHeap);
#endif
        }

    // These two should only be called when we have exclusive access...
    void SetFirstMessageInRoom(r_slot Room, m_index Msg)
        {
        assert(FirstMessageInRoom);
        FirstMessageInRoom[Room] = Msg;
        }

    void SetLastMessageInRoom(r_slot Room, m_index Msg)
        {
        assert(LastMessageInRoom);
        LastMessageInRoom[Room] = Msg;
        }

    m_index GetFirstMessageInRoom(r_slot Room)
        {
        assert(FirstMessageInRoom);
        return (FirstMessageInRoom[Room]);
        }

    m_index GetLastMessageInRoom(r_slot Room)
        {
        assert(LastMessageInRoom);
        return (LastMessageInRoom[Room]);
        }

    Bool SetRooms(r_slot New);

#else	// Regular version
class MessageTableC
    {
    msgflags *Flags;                    // every message gets flags
    int     *LocLO;                     // offset where message starts
    int     *LocHI;                     // offset where message starts
    r_slot  *RoomNum;                   // room # of message
    int     *ToHash;                    // hash of recipient or group
    int     *AuthHash;                  // hash of author of message
    uint    *OriginID;                  // LO net id, forwardee hash,
                                        //      copy offset

	m_slot Size;

    m_index NewestMsg;          // 32-bit ID# of last message in system
    m_index OldestMsgTab;       // oldest message ID # in message table

public:
    void GetTabEntry(m_index Index, MsgTabEntryS *E)
        {
        assert (Index != M_INDEX_ERROR);

        m_slot Slot = Index - OldestMsgTab;

        if (Slot >= 0 && Slot < cfg.nmessages)
            {
            E->Flags = Flags[Slot];
            E->Loc = long_JOIN(LocLO[Slot], LocHI[Slot]);
            E->RoomNum = RoomNum[Slot];
            E->ToHash = ToHash[Slot];
            E->AuthHash = AuthHash[Slot];
            E->OriginID = OriginID[Slot];
            }
        else
            {
            memset(E, 0, sizeof(*E));
            }
        }

    MessageTableC(void)
        {
        Flags = NULL;
        LocLO = NULL;
        LocHI = NULL;
        RoomNum = NULL;
        ToHash = NULL;
        AuthHash = NULL;
        OriginID = NULL;
        }

    ~MessageTableC(void)
        {
        delete [] Flags;
        delete [] LocLO;
        delete [] LocHI;
        delete [] RoomNum;
        delete [] ToHash;
        delete [] AuthHash;
        delete [] OriginID;
        }
#endif

    m_index GetCopyOffset(m_index Index)
        {
        return (long_JOIN(GetToHash(Index), GetAuthHash(Index)));
        }

#if defined(AUXMEM) || defined(WINCIT)
    void Verify(void);

    msgflags *GetFlags(m_index Index)
        {
        return (&(GetTabEntry(Index)->Flags));
        }

    ulong GetLocation(m_index Index)
        {
        return (GetTabEntry(Index)->Loc);
        }

    r_slot GetRoomNum(m_index Index)
        {
        return (GetTabEntry(Index)->RoomNum);
        }

    int GetToHash(m_index Index)
        {
        return (GetTabEntry(Index)->ToHash);
        }

    int GetAuthHash(m_index Index)
        {
        return (GetTabEntry(Index)->AuthHash);
        }

    ushort GetOriginID(m_index Index)
        {
        return (GetTabEntry(Index)->OriginID);
        }

    m_index GetPrevRoomMsg(m_index Index)
        {
        return (GetTabEntry(Index)->PrevRoomMsg);
        }

    m_index GetNextRoomMsg(m_index Index)
        {
        return (GetTabEntry(Index)->NextRoomMsg);
        }

    void SetFlags(m_index Index, msgflags New)
        {
        GetTabEntry(Index)->Flags = New;
        }

    void SetLocation(m_index Index, ulong New)
        {
        GetTabEntry(Index)->Loc = New;
        }

    void SetRoomNum(m_index Index, r_slot New)
        {
        GetTabEntry(Index)->RoomNum = New;
        }

    void SetToHash(m_index Index, int New)
        {
        GetTabEntry(Index)->ToHash = (short) New;
        }

    void SetAuthHash(m_index Index, int New)
        {
        GetTabEntry(Index)->AuthHash = (short) New;
        }

    void SetOriginID(m_index Index, uint New)
        {
        GetTabEntry(Index)->OriginID = (short) New;
        }

    void SetPrevRoomMsg(m_index Index, m_index New)
        {
        GetTabEntry(Index)->PrevRoomMsg = New;
        }

    void SetNextRoomMsg(m_index Index, m_index New)
        {
        GetTabEntry(Index)->NextRoomMsg = New;
        }
#else
    msgflags *GetFlags(m_index Index)
        {
        m_slot Slot = (m_slot) (Index - OldestMsgTab);

        if (Slot >= 0 && Slot < cfg.nmessages)
            {
            return (&Flags[Slot]);
            }
        else
            {
            return (NULL);
            }
        }

    ulong GetLocation(m_index Index)
        {
        m_slot Slot = (m_slot) (Index - OldestMsgTab);

        if (Slot >= 0 && Slot < cfg.nmessages)
            {
            return (long_JOIN(LocLO[Slot], LocHI[Slot]));
            }
        else
            {
            return (0);
            }
        }

    r_slot GetRoomNum(m_index Index)
        {
        m_slot Slot = (m_slot) (Index - OldestMsgTab);

        if (Slot >= 0 && Slot < cfg.nmessages)
            {
            return (RoomNum[Slot]);
            }
        else
            {
            return (0);
            }
        }

    int getToHash(m_index Index)
        {
        m_slot Slot = (m_slot) (Index - OldestMsgTab);

        if (Slot >= 0 && Slot < cfg.nmessages)
            {
            return (ToHash[Slot]);
            }
        else
            {
            return (0);
            }
        }

    int GetAuthHash(m_index Index)
        {
        m_slot Slot = (m_slot) (Index - OldestMsgTab);

        if (Slot >= 0 && Slot < cfg.nmessages)
            {
            return (AuthHash[Slot]);
            }
        else
            {
            return (0);
            }
        }

    ushort GetOriginID(m_index Index)
        {
        m_slot Slot = (m_slot) (Index - OldestMsgTab);

        if (Slot >= 0 && Slot < cfg.nmessages)
            {
            return (OriginID[Slot]);
            }
        else
            {
            return (0);
            }
        }

    void SetFlags(m_index Index, msgflags New)
        {
        m_slot Slot = (m_slot) (Index - OldestMsgTab);

        if (Slot >= 0 && Slot < cfg.nmessages)
            {
            Flags[Slot] = New;
            }
        }

    void SetLocation(m_index Index, ulong New)
        {
        m_slot Slot = (m_slot) (Index - OldestMsgTab);

        if (Slot >= 0 && Slot < cfg.nmessages)
            {
            LocLO[Slot] = long_LO(New);
            LocHI[Slot] = long_HI(New);
            }
        }

    void SetRoomNum(m_index Index, r_slot New)
        {
        m_slot Slot = (m_slot) (Index - OldestMsgTab);

        if (Slot >= 0 && Slot < cfg.nmessages)
            {
            RoomNum[Slot] = New;
            }
        }

    void SetToHash(m_index Index, int New)
        {
        m_slot Slot = (m_slot) (Index - OldestMsgTab);

        if (Slot >= 0 && Slot < cfg.nmessages)
            {
            ToHash[Slot] = New;
            }
        }

    void SetAuthHash(m_index Index, int New)
        {
        m_slot Slot = (m_slot) (Index - OldestMsgTab);

        if (Slot >= 0 && Slot < cfg.nmessages)
            {
            AuthHash[Slot] = New;
            }
        }

    void SetOriginID(m_index Index, uint New)
        {
        m_slot Slot = (m_slot) (Index - OldestMsgTab);

        if (Slot >= 0 && Slot < cfg.nmessages)
            {
            OriginID[Slot] = New;
            }
        }
#endif

    void Crunch(m_slot howmany);
    void Slide(m_slot howmany);

    Bool Load(FILE *fd);
    void Save(FILE *fd);

	void Clear(void);

	m_slot GetSize(void) const
		{
		return (Size);
		}

    Bool Resize(m_slot NewSize);

	void Start(m_index Start)
		{
		NewestMsg = OldestMsgTab = Start;
		}

    Bool IsValid(m_index ID)
        {
        return (ID >= OldestMsgTab && ID <= NewestMsg);
        }

	m_index GetOldestMessage(void) const
		{
		return (OldestMsgTab);
		}

	m_index GetNewestMessage(void) const
		{
		return (NewestMsg);
		}

    void SetNewestMessage(m_index whee)
        {
        NewestMsg = whee;
        }

    // In Wincit, this should only be called when we have exclusive access
    Bool Index(Message *Msg);
    };


// --------------------------------------------------------------------------
// This next class contains everything that we can know about a message.
// However, it is not stored to MSG.DAT as most information is stored to
// other .DAT files: only those fields that are used are stored to the file.
// For character strings (which is almost all there is), only the string up
// to the terminating nul (ASCII 0) byte is stored. That is, although the
// subject can be up to 81 bytes long including the terminating nul
// character, only the first six bytes of it would be stored if it is "Stuff"
// (the five bytes of "Stuff" plus the terminating nul character). How and
// why this is done is explained in MSGMAKE.CPP, explaining the putMessage
// function.


// --------------------------------------------------------------------------
// Note that we cannot save a 0xFF to MSG.DAT except at the start of a
// message. That is why we don't define any attribute as 128, and instead
// stop at 64.

#define ATTR_RECEIVED   1
#define ATTR_REPLY      2
#define ATTR_MADEVIS    4
#define ATTR_COMPRESSED 8
#define ATTR_CENSORED   16
#define ATTR_BIGROOM    32
#define ATTR_MORE       64
#define ATTR2_LOCAL     1


// --------------------------------------------------------------------------
// For storing and passing unknown message fields.

#define MAXUNKLEN 255
struct unkLst
    {
    unkLst *next;
    char whatField;
    char theValue[MAXUNKLEN+1];
    };

class Message;

class MessageStoreC
	{
	// These are called by Store()
	virtual Bool SaveByte(char Byte) = 0;
	virtual Bool SaveString(const char *String) = 0;

	// And these by Load...()
	virtual Bool GetByte(char *Byte) = 0;

	virtual long GetStoreOffset(void)
		{
		return (0);
		}
	
protected:
	Bool GetString(char *String, int BufferLength);
	Bool GetString(char *String, int BufferLength, Bool FilterControlB);

	ReadMessageStatus LoadAll(Message *Msg);
	ReadMessageStatus LoadHeader(Message *Msg);
	ReadMessageStatus LoadHeader(Message *Msg, Bool DupeCheck);

	Bool Store(Message *Msg);

	Bool NetPacket;		// network packets are saved without room numbers, can have 0 for LocalID
	Bool Compressable;	// Does this message store allow for compression?

public:
	MessageStoreC(void)
		{
		NetPacket = FALSE;
		Compressable = FALSE;
		}

	~MessageStoreC(void)
		{
		// dunno
		}
	};

#include "net86.h"

class Message
    {
    char    Text[MAXTEXT];      // buffer text is edited in
                                // make sure this is first, and it's
                                // size is MAXTEXT always. Otherwise,
                                // ClearHeader fails.

    unkLst  *firstUnk;          // Unknown message field stuff
    strList *Comments;          // Comments.

    long    HeadLoc;            // Start of message
    r_slot  RoomNumber;         // room # of message
    uchar   Attribute;          // 6 attribute bits + flag for more
    uchar   Attribute2;         // 1 attribute bit

    label   MoreFlags;          // # field...
    label   EncryptionKey;      // the key for encryption...

    label   Author;             // name of author
    label   RealName;           // real name of author
    label   Surname;            // surname
    label   Title;              // Title...
    label   Forward;            // forwarding address
    label   OriginNodeName;     // name of origin system
    label   OriginRegion;       //  "       "   region
    label   OriginCountry;      //  "       "   country
    label   TwitRegion;         // twit region
    label   TwitCountry;        // twit country
    label   OriginPhoneNumber;  // origin phone, like "(206) 542-7825"
    label   Cit86Country;       // For netting to Citadel-86
    label   DestinationAddress; // destination address

    label   CreationRoom;       // creation room
    char    Signature[91];      // signature field
    char    UserSignature[91];  // user signature field
    char    Subject[81];        // subject
    label   OriginSoftware;     // origin software
    label   SourceID;           // message ID on system of origin
    char    FromPath[PATHSIZE+1];// where did the message come from?

    label   ToUser;             // private message to
    label   PublicToUser;       // public message to
    label   ToNodeName;         // name of destination system
    label   ToRegion;           //  "           "       region
    label   ToCountry;          //  "           "       country
    label   ToPhoneNumber;      // NOT USED
    char    ToPath[PATHSIZE+1]; // forced routing via path
    label   LocalID;            // local number of message
    label   CopyOfMessage;      // message ID of copy this msg is of
    label   Group;              // group name for limited access msg
    label   CreationTime;       // creation time
    label   EZCreationTime;     // creation time
    label   ReplyToMessage;     // message ID of message replied to
    label   X;                  // twit/moderated message header
    char    FileLink[64];       // file linked message
    char    LinkedApplication[128];// application linked message
    label   SourceRoomName;     // room name on the source system


    // These are not saved to MSG.DAT, but used internally.
    Bool    referenceFlag;      // TRUE if this is a duplicate
    m_index referenceId;        // ID of original, if above is TRUE
    uchar   referenceAttr;      // Attribute of original, if ^^ TRUE
    r_slot  referenceRoom;      // Room of original, if ^^ TRUE

    Bool    onceWasCompressed;  // Ever has been?

	friend ReadMessageStatus MessageStoreC::LoadHeader(Message *Msg, Bool DupeCheck);
	friend Bool MessageStoreC::Store(Message *Msg);
	friend ReadMessageStatus Net86PacketC::LoadAll(Message *Msg);
	friend Bool Net86PacketC::Store(Message *Msg);

    void SetCompressed(Bool New)
        {
        assert(this);
        if (New)
            {
            onceWasCompressed = TRUE;
            Attribute |= ATTR_COMPRESSED;
            }
        else
            {
            Attribute &= ~ATTR_COMPRESSED;
            }
        }


    Bool IsCompressed(void) const
        {
        assert(this);
        return (Attribute & ATTR_COMPRESSED);
        }

public:
    Message(const Message &Original);

    Message(void)
        {
        assert(this);
        firstUnk = NULL;        // Because ClearAll() tries to dispose it.
        Comments = NULL;        // same thing.
        ClearAll();
        }

    ~Message(void)
        {
        assert(this);
        VerifyHeap();
        disposeUnknownList();
        disposeComments();
        VerifyHeap();
        }

    Message& operator =(const Message &Original);

    void FixupNetwork(NodesCitC *N);

    void ClearAll(void)
        {
        assert(this);
        VerifyHeap();
        disposeUnknownList();
        disposeComments();
        memset(this, 0, sizeof(Message));
        VerifyHeap();
        }

    void ClearHeader(void)
        {
        assert(this);
        VerifyHeap();
        disposeUnknownList();
        disposeComments();
        memset(((char *) this) + MAXTEXT, 0, sizeof(Message) - MAXTEXT);
        VerifyHeap();
        }

    Bool IsReceived(void) const
        {
        assert(this);
        return (Attribute & ATTR_RECEIVED);
        }

    Bool IsRepliedTo(void) const
        {
        assert(this);
        return (Attribute & ATTR_REPLY);
        }

    Bool IsMadeVisible(void) const
        {
        assert(this);
        return (Attribute & ATTR_MADEVIS);
        }

    Bool Compress(void)
//    Bool Compress(WC_TW)
        {
        if (cfg.msgCompress && !IsCompressed())
            {
            if (compress(GetTextPointer()))
//            if (tw()compress(GetTextPointer()))
                {
                SetCompressed(TRUE);
                }
            }

        return (IsCompressed());
        }

    void Decompress(void)
//    void Decompress(WC_TW)
        {
        if (IsCompressed())
            {
            decompress((uchar *) GetTextPointer());
//            tw()decompress((uchar *) GetTextPointer());
            SetCompressed(FALSE);
            }
        }

    Bool WasEverCompressed(void) const
        {
        return (onceWasCompressed);
        }

    Bool IsCensored(void) const
        {
        assert(this);
        return (Attribute & ATTR_CENSORED);
        }

    Bool IsBigRoom(void) const
        {
        assert(this);
        return (Attribute & ATTR_BIGROOM);
        }

    Bool IsLocal(void) const
        {
        assert(this);
        return (Attribute2 & ATTR2_LOCAL);
        }

    void SetReceived(Bool New)
        {
        assert(this);
        if (New)
            {
            Attribute |= ATTR_RECEIVED;
            }
        else
            {
            Attribute &= ~ATTR_RECEIVED;
            }
        }

    void SetRepliedTo(Bool New)
        {
        assert(this);
        if (New)
            {
            Attribute |= ATTR_REPLY;
            }
        else
            {
            Attribute &= ~ATTR_REPLY;
            }
        }

    void SetMadeVisible(Bool New)
        {
        assert(this);
        if (New)
            {
            Attribute |= ATTR_MADEVIS;
            }
        else
            {
            Attribute &= ~ATTR_MADEVIS;
            }
        }

    void SetCensored(Bool New)
        {
        assert(this);
        if (New)
            {
            Attribute |= ATTR_CENSORED;
            }
        else
            {
            Attribute &= ~ATTR_CENSORED;
            }
        }

    void SetBigRoom(Bool New)
        {
        assert(this);
        if (New)
            {
            Attribute |= ATTR_BIGROOM;
            }
        else
            {
            Attribute &= ~ATTR_BIGROOM;
            }
        }

    void SetLocal(Bool New)
        {
        assert(this);
        if (New)
            {
            Attribute2 |= ATTR2_LOCAL;
            }
        else
            {
            Attribute2 &= ~ATTR2_LOCAL;
            }

        if (Attribute2)
            {
            Attribute |= ATTR_MORE;
            }
        else
            {
            Attribute &= ~ATTR_MORE;
            }
        }

    void SetTextWithFormat(const char *NewText, ...)
        {
        assert(this);
        va_list ap;

        va_start(ap, NewText);
        vsprintf(Text, NewText, ap);
        va_end(ap);
        }

    void AppendText(const char *NewText)
        {
        assert(this);
        strncat(Text, NewText, MAXTEXT - strlen(Text) - 1);
        Text[MAXTEXT - 1] = 0;
        }

    unkLst *GetFirstUnknown(void)
        {
        assert(this);
        return (firstUnk);
        }

    const char *GetComment(long Index) const
        {
        strList *Wow = (strList *) getLLNum(Comments, Index);

        if (Wow)
            {
            return (Wow->string);
            }
        else
            {
            return (NULL);
            }
        }

    long GetCommentCount(void) const
        {
        assert(this);
        return (getLLCount(Comments));
        }

    unkLst *addUnknownList(void)
        {
        assert(this);
        return ((unkLst *) addLL((void **) &firstUnk, sizeof(*firstUnk)));
        }

    Bool AddComment(const char *New)
        {
        assert(this);
        assert(New);
        assert(*New);

        strList *sl = (strList *) addLL((void **) &Comments, sizeof(*Comments) + strlen(New));

        if (sl)
            {
            strcpy(sl->string, New);
            return (TRUE);
            }
        else
            {
            return (FALSE);
            }
        }

    void disposeUnknownList(void)
        {
        assert(this);
#ifdef VISUALC
        disposeLL((void **)&firstUnk);
#else
        disposeLL((void **)&firstUnk);
//        disposeLL(&((void *) firstUnk));
#endif
        }

    void disposeComments(void)
        {
        assert(this);
#ifdef VISUALC
        disposeLL((void **)&Comments);
#else
        disposeLL((void **)&Comments);
//        disposeLL(&((void *) Comments));
#endif
        }

    // try not to use these next five. they are evil.
    char *GetTextPointer(void)
        {
        assert(this);
        return (Text);
        }

    char GetAttribute(void)
        {
        assert(this);
        return (Attribute);
        }

    char GetAttribute2(void)
        {
        assert(this);
        return (Attribute2);
        }

    void SetAttribute(char New)
        {
        assert(this);
        assert(New != -1);
        Attribute = New;
        }

    void SetAttribute2(char New)
        {
        assert(this);
        assert(New != -1);
        Attribute2 = New;
        }

    // end of evil.

    const char *GetText(void) const
        {
        assert(this);
        return (Text);
        }

    long GetHeadLoc(void) const
        {
        assert(this);
        return (HeadLoc);
        }

    r_slot GetRoomNumber(void) const
        {
        assert(this);
        return (RoomNumber);
        }

    const char *GetMoreFlags(void) const
        {
        assert(this);
        return (MoreFlags);
        }

    Bool IsMoreFlag(const char Flag) const
        {
        assert(this);
        assert(Flag);
        return (!!strchr(MoreFlags, Flag));
        }

    Bool IsReceiptConfirmationRequested(void) const
        {
        assert(this);
        return (IsMoreFlag(getmsg(28)[0]));
        }

    Bool IsEncrypted(void) const
        {
        assert(this);
        return (IsMoreFlag(getmsg(28)[1]));
        }

    const char *GetEncryptionKey(void) const
        {
        assert(this);
        return (EncryptionKey);
        }

    const char *GetAuthor(void) const
        {
        assert(this);
        return (Author);
        }

    const char *GetRealName(void) const
        {
        assert(this);
        return (RealName);
        }

    const char *GetSurname(void) const
        {
        assert(this);
        return (Surname);
        }

    const char *GetTitle(void) const
        {
        assert(this);
        return (Title);
        }

    const char *GetForward(void) const
        {
        assert(this);
        return (Forward);
        }

    const char *GetOriginNodeName(void) const
        {
        assert(this);
        return (OriginNodeName);
        }

    const char *GetOriginRegion(void) const
        {
        assert(this);
        return (OriginRegion);
        }

    const char *GetOriginCountry(void) const
        {
        assert(this);
        return (OriginCountry);
        }

    const char *GetTwitRegion(void) const
        {
        assert(this);
        return (TwitRegion);
        }

    const char *GetTwitCountry(void) const
        {
        assert(this);
        return (TwitCountry);
        }

    const char *GetOriginPhoneNumber(void) const
        {
        assert(this);
        return (OriginPhoneNumber);
        }

    const char *GetCit86Country(void) const
        {
        assert(this);
        return (Cit86Country);
        }

    const char *GetDestinationAddress(void) const
        {
        assert(this);
        return (DestinationAddress);
        }

    const char *GetCreationRoom(void) const
        {
        assert(this);
        return (CreationRoom);
        }

    const char *GetSignature(void) const
        {
        assert(this);
        return (Signature);
        }

    const char *GetUserSignature(void) const
        {
        assert(this);
        return (UserSignature);
        }

    const char *GetSubject(void) const
        {
        assert(this);
        return (Subject);
        }

    const char *GetOriginSoftware(void) const
        {
        assert(this);
        return (OriginSoftware);
        }

    const char *GetSourceID(void) const
        {
        assert(this);
        return (SourceID);
        }

    const char *GetFromPath(void) const
        {
        assert(this);
        return (FromPath);
        }

    const char *GetToUser(void) const
        {
        assert(this);
        return (ToUser);
        }

    const char *GetPublicToUser(void) const
        {
        assert(this);
        return (PublicToUser);
        }

    const char *GetToNodeName(void) const
        {
        assert(this);
        return (ToNodeName);
        }

    const char *GetToRegion(void) const
        {
        assert(this);
        return (ToRegion);
        }

    const char *GetToCountry(void) const
        {
        assert(this);
        return (ToCountry);
        }

    const char *GetToPhoneNumber(void) const
        {
        assert(this);
        return (ToPhoneNumber);
        }

    const char *GetToPath(void) const
        {
        assert(this);
        return (ToPath);
        }

    const char *GetLocalID(void) const
        {
        assert(this);
        return (LocalID);
        }

    const char *GetCopyOfMessage(void) const
        {
        assert(this);
        return (CopyOfMessage);
        }

    const char *GetGroup(void) const
        {
        assert(this);
        return (Group);
        }

    const char *GetCreationTime(void) const
        {
        assert(this);
        return (CreationTime);
        }

    const char *GetEZCreationTime(void) const
        {
        assert(this);
        return (EZCreationTime);
        }

    const char *GetReplyToMessage(void) const
        {
        assert(this);
        return (ReplyToMessage);
        }

    const char *GetX(void) const
        {
        assert(this);
        return (X);
        }

    const char *GetFileLink(void) const
        {
        assert(this);
        return (FileLink);
        }

    const char *GetLinkedApplication(void) const
        {
        assert(this);
        return (LinkedApplication);
        }

    const char *GetSourceRoomName(void) const
        {
        assert(this);
        return (SourceRoomName);
        }

    void SetText(const char *New)
        {
        assert(this);
        CopyStringToBuffer(Text, New);
        }

    void SetHeadLoc(long New)
        {
        assert(this);
        HeadLoc = New;
        }

    void SetRoomNumber(r_slot New)
        {
        assert(this);
        RoomNumber = New;

        if (RoomNumber > 254)
            {
            SetBigRoom(TRUE);
            }
        }

    Bool SetMoreFlag(const char Flag, Bool New);

    Bool SetReceiptConfirmationRequested(Bool New)
        {
        assert(this);
        return (SetMoreFlag(getmsg(28)[0], New));
        }

    Bool SetEncrypted(Bool New)
        {
        assert(this);
        return (SetMoreFlag(getmsg(28)[1], New));
        }

    void SetEncryptionKey(const char *New)
        {
        assert(this);
        CopyStringToBuffer(EncryptionKey, New);
        }

    void SetAuthor(const char *New)
        {
        assert(this);
        CopyStringToBuffer(Author, New);
        }

    void SetAuthor(const LogEntry *User)
        {
        assert(this);
        User->GetName(Author, sizeof(Author));
        }

    void SetRealName(const char *New)
        {
        assert(this);
        CopyStringToBuffer(RealName, New);
        }

    void SetRealName(const LogEntry *User)
        {
        assert(this);
        User->GetRealName(RealName, sizeof(RealName));
        }

    void SetSurname(const char *New)
        {
        assert(this);
        CopyStringToBuffer(Surname, New);
        }

    void SetSurname(const LogEntry *User)
        {
        assert(this);
        User->GetSurname(Surname, sizeof(Surname));
        }

    void SetTitle(const char *New)
        {
        assert(this);
        CopyStringToBuffer(Title, New);
        }

    void SetTitle(const LogEntry *User)
        {
        assert(this);
        User->GetTitle(Title, sizeof(Title));
        }

    void SetForward(const char *New)
        {
        assert(this);
        CopyStringToBuffer(Forward, New);
        }

    void SetOriginNodeName(const char *New)
        {
        assert(this);
        CopyStringToBuffer(OriginNodeName, New);
        }

    void SetOriginRegion(const char *New)
        {
        assert(this);
        CopyStringToBuffer(OriginRegion, New);
        }

    void SetOriginCountry(const char *New)
        {
        assert(this);
        CopyStringToBuffer(OriginCountry, New);
        }

    void SetTwitRegion(const char *New)
        {
        assert(this);
        CopyStringToBuffer(TwitRegion, New);
        }

    void SetTwitCountry(const char *New)
        {
        assert(this);
        CopyStringToBuffer(TwitCountry, New);
        }

    void SetOriginPhoneNumber(const char *New)
        {
        assert(this);
        CopyStringToBuffer(OriginPhoneNumber, New);
        }

    void SetCit86Country(const char *New)
        {
        assert(this);
        CopyStringToBuffer(Cit86Country, New);
        }

    void SetDestinationAddress(const char *New)
        {
        assert(this);
        CopyStringToBuffer(DestinationAddress, New);
        }

    void SetCreationRoom(const char *New)
        {
        assert(this);
        CopyStringToBuffer(CreationRoom, New);
        }

    void SetCreationRoom(r_slot Room)
        {
        assert(this);
        RoomTab[Room].GetName(CreationRoom, sizeof(CreationRoom));
        }

    void SetCreationRoom(void)
        {
        assert(this);
        RoomTab[GetRoomNumber()].GetName(CreationRoom, sizeof(CreationRoom));
        }

    void SetSignature(const char *New)
        {
        assert(this);
        CopyStringToBuffer(Signature, New);
        }

    void SetUserSignature(const char *New)
        {
        assert(this);
        CopyStringToBuffer(UserSignature, New);
        }

    void SetUserSignature(const LogEntry *User)
        {
        assert(this);
        User->GetSignature(UserSignature, sizeof(UserSignature));
        }

    void SetSubject(const char *New)
        {
        assert(this);
        CopyStringToBuffer(Subject, New);
        }

    void SetOriginSoftware(const char *New)
        {
        assert(this);
        CopyStringToBuffer(OriginSoftware, New);
        }

    void SetSourceID(const char *New)
        {
        assert(this);
        assert(isNumeric(New));
        CopyStringToBuffer(SourceID, New);
        }

    void SetFromPath(const char *New)
        {
        assert(this);
        CopyStringToBuffer(FromPath, New);
        }

    void SetToUser(const char *New)
        {
        assert(this);
        CopyStringToBuffer(ToUser, New);
        }

    void SetPublicToUser(const char *New)
        {
        assert(this);
        CopyStringToBuffer(PublicToUser, New);
        }

    void SetToNodeName(const char *New)
        {
        assert(this);
        CopyStringToBuffer(ToNodeName, New);
        }

    void SetToRegion(const char *New)
        {
        assert(this);
        CopyStringToBuffer(ToRegion, New);
        }

    void SetToCountry(const char *New)
        {
        assert(this);
        CopyStringToBuffer(ToCountry, New);
        }

    void SetToPhoneNumber(const char *New)
        {
        assert(this);
        CopyStringToBuffer(ToPhoneNumber, New);
        }

    void SetToPath(const char *New)
        {
        assert(this);
        CopyStringToBuffer(ToPath, New);
        }

    void SetLocalID(const char *New)
        {
        assert(this);
        assert(isNumeric(New));

        CopyStringToBuffer(LocalID, New);
        }

    void SetLocalID(m_index New)
        {
        assert(this);

        label Buffer;
        ltoa(New, LocalID, 10);
        }

    void SetCopyOfMessage(const char *New)
        {
        assert(this);
        assert(isNumeric(New));

        CopyStringToBuffer(CopyOfMessage, New);
        }

    void SetGroup(const char *New)
        {
        assert(this);
        CopyStringToBuffer(Group, New);
        }

    void SetGroup(g_slot GroupSlot)
        {
        assert(this);
        GroupData[GroupSlot].GetName(Group, sizeof(Group));
        }

    void SetCreationTime(const char *New)
        {
        assert(this);
        assert(isNumeric(New));

        CopyStringToBuffer(CreationTime, New);
        }

    void SetEZCreationTime(const char *New)
        {
        assert(this);
        CopyStringToBuffer(EZCreationTime, New);
        }

    void SetReplyToMessage(const char *New)
        {
        assert(this);
        assert(isNumeric(New));

        CopyStringToBuffer(ReplyToMessage, New);
        }

    void SetX(const char *New)
        {
        assert(this);
        CopyStringToBuffer(X, New);
        }

    void SetFileLink(const char *New)
        {
        assert(this);
        CopyStringToBuffer(FileLink, New);
        }

    void SetLinkedApplication(const char *New)
        {
        assert(this);
        CopyStringToBuffer(LinkedApplication, New);
        }

    void SetSourceRoomName(const char *New)
        {
        assert(this);
        CopyStringToBuffer(SourceRoomName, New);
        }

    Bool IsViewDuplicate(void)
        {
        assert(this);
        return (referenceFlag);
        }

    void SetViewDuplicate(Bool New)
        {
        assert(this);
        referenceFlag = !!New;
        }

    m_index GetOriginalID(void) const
        {
        assert(this);
        return (referenceId);
        }

    void SetOriginalID(m_index New)
        {
        assert(this);
        referenceId = New;
        }

    uchar GetOriginalAttribute(void) const
        {
        assert(this);
        return (referenceAttr);
        }

    void SetOriginalAttribute(uchar New)
        {
        assert(this);
        referenceAttr = New;
        }

    r_slot GetOriginalRoom(void) const
        {
        assert(this);
        return (referenceRoom);
        }

    void SetOriginalRoom(r_slot New)
        {
        assert(this);
        referenceRoom = New;
        }

    Bool IsNetworkedMail(void) const
        {
        return (*GetToNodeName() || *GetDestinationAddress());
        }

    Bool IsMail(void) const
        {
        return (*GetToUser());
        }

    Bool IsMassEMail(void) const
        {
        return (IsMail() && *GetGroup());
        }

    Bool IsNetworkedMailForHere(void) const
        {
        return (// It is mail
                IsNetworkedMail() &&

                    (
                    // Message is to this node name
                    SameString(GetToNodeName(), cfg.nodeTitle) ||

                    // or to this address
                    SameString(GetDestinationAddress(), cfg.Address)
                    )
            );
        }

    Bool IsAnonymous(void) const
        {
        return (!*GetAuthor() || SameString(GetAuthor(), "****"));
        }

	const char *GetOriginAddress(char *Buffer) const;
    };

struct MessageDebugInfoS
	{
	Bool	IsValid;
	Bool	IsMail;
	Bool	IsReceived;
	Bool    IsReply;
	Bool    IsProblem;
	Bool    IsMadevis;
	Bool    IsLimited;
	Bool    IsModerated;
	Bool    IsCopy;
	Bool    IsNet;
	Bool    IsForwarded;
	Bool    IsNetworked;
	Bool    IsMassemail;
	Bool    IsCensored;
	Bool    IsLocal;
	Bool    IsInuse;

	r_slot	RoomNum;
	ulong	Location;
	ushort	OriginID;

	m_index	CopyOffset;
	int		ToHash;
	int		AuthHash;

	#if defined(WINCIT) || defined(AUXMEM)
		m_index	PrevRoomMsg;
		m_index	NextRoomMsg;
	#endif
	};


class MessageDatC: MessageStoreC
	{
#ifdef WINCIT
	HANDLE OurMutex;
#endif

	FILE	*MsgDat;
	char	MsgFilePath[128];
    long    catLoc;             // where to begin writing next message
	long	SizeInBytes;

	MessageTableC Table;

    m_index OldestMessage;		// 32-bit ID# of first message in system

	virtual Bool SaveByte(char Byte);
	virtual Bool SaveString(const char *String);

	virtual long GetStoreOffset(void)
		{
		assert(MsgDat);

		if (MsgDat)
			{
			return (ftell(MsgDat));
			}
		else 
			{
			return (0);
			}
		}

	Bool RawSaveByte(char Byte);	// No PrepareToOverwrite() call!

	virtual Bool GetByte(char *Byte);

	void PrepareToOverwrite(int bytes);
	
	void SeekMessage(m_index Index)
		{
		assert(MsgDat);
		fseek(MsgDat, Table.GetLocation(Index), SEEK_SET);
		}

	void Lock(void)
		{
#ifdef WINCIT
		WaitForSingleObject(OurMutex, INFINITE);
#endif
		}

	void Unlock(void)
		{
#ifdef WINCIT
		ReleaseMutex(OurMutex);
#endif
		}

	Bool StoreCommon(Message *Msg);

public:
	MessageDatC(void)
		{
		MsgDat = NULL;
		*MsgFilePath = 0;
		catLoc = 0;
		SizeInBytes = 0;
		OldestMessage = 0;
		Compressable = TRUE;

#ifdef WINCIT
		OurMutex = CreateMutex(NULL, FALSE, NULL);
#endif
		}

	~MessageDatC(void)
		{
		if (MsgDat)
			{
			fclose(MsgDat);
			}

#ifdef WINCIT
		if (OurMutex != INVALID_HANDLE_VALUE)
			{
			CloseHandle(OurMutex);
			}
#endif
		}

	const char *GetFilename(void)
		{
		return (MsgFilePath);
		}

	int GetPercentFull(void)
		{
		if (OldestMessage == 1L || (Table.GetLocation(OldestMessage) == 0))
			{
			return ((int) ( (catLoc * 25L ) / (SizeInBytes / 4l) ));
			}
		else
			{
			return (100);
			}
		}

	long GetBytesUsed(void)
		{
		if (OldestMessage == 1L || (Table.GetLocation(OldestMessage) == 0))
			{
			return (catLoc);
			}
		else
			{
			return (SizeInBytes);
			}
		}

	long GetAverageMessageLength(void)
		{
		return (GetBytesUsed() / CountMessages());
		}

	m_index CountMessages(void)
		{
		return (NewestMessage() - OldestMessage + 1);
		}

	void Resize(long NewSizeInBytes);

	Bool BuildIndex(m_slot Slots);
	void ChangeRoom(m_index ID, r_slot roomno);
	void ChangeAttribute(m_index ID, uchar Attr);

	void BuildTable(void);

    Bool GetTabEntry(m_index Index, MsgTabEntryS *E)
		{
		Lock();

		Bool RetVal = TRUE;

		if (IsValid(Index))
			{
			#if defined(AUXMEM) || defined(WINCIT)
				MsgTabEntryS *T = Table.GetTabEntry(Index);
				*E = *T;
			#else
				Table.GetTabEntry(Index, E);
			#endif
			}
		else
			{
			memset(E, 0, sizeof(*E));
			RetVal = FALSE;
			}

		Unlock();

		return (RetVal);
		}


	void SetTableSize(m_slot Slots)
		{
		Lock();
		Table.Resize(Slots);
		Unlock();
		}

	void SetRooms(r_slot Slots)
		{
		Lock();
		Table.SetRooms(Slots);
		Unlock();
		}

#if defined(AUXMEM) || defined(WINCIT)
    m_index GetFirstMessageInRoom(r_slot Room)
        {
        return (Table.GetFirstMessageInRoom(Room));
        }

    m_index GetLastMessageInRoom(r_slot Room)
        {
        return (Table.GetLastMessageInRoom(Room));
        }
#endif
//this is roya's comment on the GetLastMessageInRoom function. i think it's great!
	void InitializeMessageFile(long NewSizeInBytes);
	Bool LoadTable(void);
	void SaveTable(void);
	Bool CreateMessageFile(const char *Directory);
	Bool OpenMessageFile(const char *Directory);	// this function also sets SizeInBytes
	void CloseMessageFile(void)
		{
		Lock();

		assert(MsgDat);

		if (MsgDat)
			{
			fclose(MsgDat);
			MsgDat = NULL;
			}

		Unlock();
		}

	void Verify(void)
		{
		Lock();
		Table.Verify();
		Unlock();
		}

	Bool Store(Message *Msg);
	Bool ResizeStore(Message *Msg);

	Bool StoreAsStub(Message *Msg, const char *Type, Bool StoreFull);

	ReadMessageStatus LoadAll(m_index Index, Message *Msg);
	ReadMessageStatus LoadHeader(m_index Index, Message *Msg);
	ReadMessageStatus LoadDupeCheck(m_index Index, Message *Msg);

	Bool IsValid(m_index Index)
		{
		return (Table.IsValid(Index));
		}

	m_index OldestMessageInFile(void) const
		{
		return (OldestMessage);
		}

	m_index OldestMessageInTable(void) const
		{
		return (Table.GetOldestMessage());
		}

	m_index NewestMessage(void) const
		{
		return (Table.GetNewestMessage());
		}

	void EnsureRoomEmpty(r_slot Room);
	Bool RoomIsEmpty(r_slot Room);

	void FillDebugInfo(MessageDebugInfoS *MDI, m_index Index);
	void CountMessageTypes(m_index *Public, m_index *Networked, m_index *Private, m_index *Moderated, m_index *Group,
			m_index *Copied, m_index *MassEMail, m_index *Censored, m_index *Local, m_index *Problem);

#ifdef WINCIT
    IncorporateStatusE IncorporateMessage(Message *Msg, r_slot Room, const LogEntry *Log, const char *LocalTempPath,
			l_index SkipLog, NodesCitC *Node);
#else
    IncorporateStatusE IncorporateMessage(Message *Msg, r_slot Room, const LogEntry *Log, l_index SkipLog, NodesCitC *Node);
#endif



	// Stuff from the table!
	Bool IsMail(m_index Index)				{ return (Table.GetFlags(Index)->IsMail()); }
	Bool IsReceived(m_index Index)  		{ return (Table.GetFlags(Index)->IsReceived()); }
	Bool IsReply(m_index Index)     		{ return (Table.GetFlags(Index)->IsReply()); }
	Bool IsProblem(m_index Index)   		{ return (Table.GetFlags(Index)->IsProblem()); }
	Bool IsMadevis(m_index Index)   		{ return (Table.GetFlags(Index)->IsMadevis()); }
	Bool IsLimited(m_index Index)   		{ return (Table.GetFlags(Index)->IsLimited()); }
	Bool IsModerated(m_index Index) 		{ return (Table.GetFlags(Index)->IsModerated()); }
	Bool IsCopy(m_index Index)      		{ return (Table.GetFlags(Index)->IsCopy()); }
	Bool IsNet(m_index Index)       		{ return (Table.GetFlags(Index)->IsNet()); }
	Bool IsForwarded(m_index Index) 		{ return (Table.GetFlags(Index)->IsForwarded()); }
	Bool IsNetworked(m_index Index) 		{ return (Table.GetFlags(Index)->IsNetworked()); }
	Bool IsMassemail(m_index Index) 		{ return (Table.GetFlags(Index)->IsMassemail()); }
	Bool IsCensored(m_index Index)  		{ return (Table.GetFlags(Index)->IsCensored()); }
	Bool IsLocal(m_index Index)     		{ return (Table.GetFlags(Index)->IsLocal()); }
	Bool IsInuse(m_index Index)     		{ return (Table.GetFlags(Index)->IsInuse()); }
		
	r_slot GetRoomNum(m_index Index)		{ return (Table.GetRoomNum(Index)); }
    ulong GetLocation(m_index Index)		{ return (Table.GetLocation(Index)); }
    int GetToHash(m_index Index)			{ return (Table.GetToHash(Index)); }
    int GetAuthHash(m_index Index)			{ return (Table.GetAuthHash(Index)); }
    ushort GetOriginID(m_index Index)		{ return (Table.GetOriginID(Index)); }
    m_index GetPrevRoomMsg(m_index Index)	{ return (Table.GetPrevRoomMsg(Index)); }
    m_index GetNextRoomMsg(m_index Index)	{ return (Table.GetNextRoomMsg(Index)); }
    m_index GetCopyOffset(m_index Index)	{ return (Table.GetCopyOffset(Index)); }
	};		

extern MessageDatC MessageDat;

// MsgAddr.cpp
#ifdef WINCIT
Bool ResolveMessageAddressingIssues(Message *Msg, TermWindowC *TW, ModemConsoleE W);
#else
Bool ResolveMessageAddressingIssues(Message *Msg, ModemConsoleE W);
#endif

// MsgMake.cpp
void systemMessage(Message *Msg);
Bool putAndNoteMessage(Message *Msg, Bool censor);

// MsgMod.cpp
Bool censor_message(Message *Msg);
void censormsg(Message *Msg);

