// --------------------------------------------------------------------------
// Citadel: Net6969.H
//
// Stuff for Net 6969

#include "net.h"

#ifdef WINCIT

class Net6969SessionC;

#define Net6969MessageIOOutBufferSize 10240
#define Net6969MessageIOInBufferSize 10240

class Net6969MessageIOC: MessageStoreC
    {
    Net6969SessionC *N;

    virtual Bool SaveByte(char Byte);
    virtual Bool SaveString(const char *String);

    virtual Bool GetByte(char *Byte);


public:
    Net6969MessageIOC(Net6969SessionC *NewN)
        {
        NetPacket = TRUE;
        N = NewN;
        }

    ~Net6969MessageIOC(void)
        {
        }

    // It's a good idea to remember to call Net6969SessionC::FlushOutBuffer() when you're done storing
    Bool Store(Message *Msg);
    ReadMessageStatus LoadAll(Message *Msg)
        {
        return (MessageStoreC::LoadAll(Msg));
        }
    };


// Each Net6969Session maintains a list of NetIDs that the remote system has requested. When something determines that
// a NetID has data to be sent to the remote system, it sets CheckSend to TRUE. Then the sending thread knows to send
// stuff. CheckSend is set to TRUE when a NetID is added to the list and when a new message is left in the room.
struct netIdList
    {
    netIdList *next;
    Bool CheckSend;
    char NetID[1];             // The string being stored.
    };

// Miscellaneous Net6969 Packets stored in memory.
class Net6969PacketC
    {
    label   Command;
    size_t  DataLength;
    void    *Data;

public:
    Net6969PacketC(const label NewCommand, size_t NewDataLength, const void *NewData)
        {
        CopyStringToBuffer(Command, NewCommand);
        DataLength = NewDataLength;
        Data = new char[NewDataLength];
        if (Data)
            {
            memcpy(Data, NewData, NewDataLength);
            }
        }

    ~Net6969PacketC()
        {
        delete [] Data;
        }

    Bool IsValid(void) const
        {
        return (DataLength && Data);
        }

    const char *GetCommand(void) const
        {
        return (Command);
        }

    size_t GetDataLength(void) const
        {
        return (DataLength);
        }

    const void *GetData(void) const
        {
        return (Data);
        }
    };

struct Net6969PacketListS
    {
    Net6969PacketListS  *Next;
    Net6969PacketC      *Packet;
    };

class Net6969SessionC
    {
    HANDLE OurMutex;
    l_index Log;
    LogEntry *Node;
    NodesCitC *NodesCit;
    SOCKET Socket;
    Bool KillSession;
    Bool ReceiveActive;
    Bool STARTMESS;
    Message *N6969Msg;
    strList *NetIDsToRequest;
    netIdList *RequestedNetIDs;

    int OutBufferUsed;
    char *OutBuffer;

    int InBufferUsed;
    int InBufferPos;
    char *InBuffer;

    Net6969MessageIOC *MsgIO;

    Net6969PacketListS *PendingPackets;

    void Lock(void)
        {
        WaitForSingleObject(OurMutex, INFINITE);
        }

    void Unlock(void)
        {
        ReleaseMutex(OurMutex);
        }

    Bool SendByte(char Byte);
    Bool SendString(const char *String, Bool WithNul = FALSE);
    Bool SendData(const void *Data, size_t SendLength);

    Bool GetByte(char *Byte);
    Bool GetString(char *Buffer, size_t BufferSize);
    void SkipBytes(long ToSkip);
    Bool FlushOutBuffer(void);

    friend Bool Net6969MessageIOC::SaveByte(char Byte);
    friend Bool Net6969MessageIOC::SaveString(const char *String);
    friend Bool Net6969MessageIOC::GetByte(char *Byte);
    friend Bool Net6969MessageIOC::Store(Message *Msg);

public:
    Net6969SessionC(SOCKET NewSocket, l_index NewLog)
        {
        Log = NewLog;
        Socket = NewSocket;

        Node = new LogEntry(cfg.maxrooms, cfg.maxgroups, cfg.maxjumpback);
        OurMutex = CreateMutex(NULL, FALSE, NULL);

        PendingPackets = NULL;
        KillSession = FALSE;
        ReceiveActive = FALSE;
        STARTMESS = FALSE;
        N6969Msg = NULL;
        RequestedNetIDs = NULL;
        NetIDsToRequest = NULL;

        OutBuffer = new char[Net6969MessageIOOutBufferSize];
        OutBufferUsed = 0;

        InBuffer = new char[Net6969MessageIOInBufferSize];
        InBufferUsed = 0;
        InBufferPos = 0;

        MsgIO = new Net6969MessageIOC(this);

        label Name;
        NodesCit = NULL;
        ReadNodesCit(NULL, &NodesCit, LogTab[Log].GetName(Name, sizeof(Name)), NOMODCON, TRUE);
        }

    ~Net6969SessionC();

    void SetReceive(Bool A)
        {
        ReceiveActive = A;
        }

    Bool IsReceive(void) const
        {
        return (ReceiveActive);
        }

    Bool IsGood(void) const
        {
        return (Node != NULL && MsgIO != NULL && NodesCit != NULL);
        }

    Bool IsChat(NCC_Type ChatType) const
        {
        return (NodesCit && NodesCit->IsChat(ChatType));
        }

    l_index GetLog(void) const
        {
        return (Log);
        }

    SOCKET GetSocket(void) const
        {
        return (Socket);
        }

    void Kill(void)
        {
        KillSession = TRUE;
        }

    Bool IsKilled(void) const
        {
        return (KillSession);
        }

    void DoNetSend(void);
    void DoNetReceive(void);

    void RequestNetID(r_slot Room, const char *NewNetID);

    void AddRequestedNetID(const char *NewNetID);
    void NewMessage(const char *NetID);
    Bool GetNetIDToSend(char *NetID, size_t Length);
    Bool GetNetIDToRequest(char *NetID, size_t Length);

    Bool AddPacket(Net6969PacketC *ToAdd);

    void Debug(TermWindowC *TW);
    void net6969_error(Bool errorflag, const char *node, const char *fmt, ...);
    void cdecl natPrintf(const char *fmt, ...);
    void SaveN6969Mess(const char *group = NULL, r_slot RoomNumber = AIDEROOM);
    };

struct ActiveSessionsS
    {
    ActiveSessionsS *next;
    Net6969SessionC *Session;
    };

enum Net6969CommandsE
    {
    N6969_NETID,    N6969_MESSAGE,  N6969_CHATALL,

    N6969_NUM
    };

class Net6969C
    {
    HANDLE OurMutex;

    ActiveSessionsS *ActiveSessions;

    discardable *DData;

    void Lock(void)
        {
        WaitForSingleObject(OurMutex, INFINITE);
        }

    void Unlock(void)
        {
        ReleaseMutex(OurMutex);
        }

//  void KillSession(l_index ToKill);
    void KillAllSessions(void);

    Bool NetworkActive(l_index LogIndex);
    Net6969SessionC *AddSession(SOCKET NewSocket, l_index NewLog);

    friend Net6969MessageIOC;
public:
    Net6969C(void)
        {
        ActiveSessions = NULL;
        DData = NULL;
        OurMutex = CreateMutex(NULL, FALSE, NULL);
        }

    ~Net6969C();

    Bool Network(NodesCitC *NodeInfo, l_index LogIndex);
    Bool Network(SOCKET Socket, l_index LogIndex);
    void StartListener(void);
    Bool RemoveSession(ActiveSessionsS *S);
    Bool RemoveSession(Net6969SessionC *S);

    Bool InitializeDiscardable(void);

    // do InitializeDiscardable() before either of these.
    Net6969CommandsE GetCommand(const char *String) const;
    const char *CommandText(Net6969CommandsE Cmd) const
        {
        return (((char **) DData->aux)[Cmd]);
        }

    void Propogate(const Message *Msg);
    void Propogate(r_slot RoomNum, const RoomC *Room);

    void ChatAll(const char *ChatText);
    void ChatUser(const char *ChatText, const char *UserName, const char *RemoteNode);
    void ChatGroup(const char *ChatText, const char *GroupName);
    void ChatRoom(const char *ChatText, const char *NetID);

    void Debug(TermWindowC *TW);
    };

extern Net6969C Net6969;

#endif
