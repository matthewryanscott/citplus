// --------------------------------------------------------------------------
// Citadel: Net.H
//
// Stuff for networking

#include "msg.h"

// --------------------------------------------------------------------------
// When networking, Citadel loads information about the other node from the
// NODES.CIT file into memory. This tells it how to connect to the other
// node, what networking protocol to use, etc. The next two structures define
// what to load. The first is a linked list of groups to map between the
// nodes. The second is everything else, and the root of the linked list.

enum NETTYPES
	{
	NET_DCIT10, 		NET_DCIT11, 		NET_DCIT15,
	NET_DCIT16, 		NET_6_9,			NET_6_9a,
	NET_HENGE,			NET_C86,			NET_6969,

	NET_NUM
	};

enum NCV_Type
	{
	// #VERBOSE 0
	NCV_FILE69IN,

	// #VERBOSE 1
	NCV_FILE69INFULL,	NCV_NOACCESS,

	// #VERBOSE 2
	NCV_ROOMCREATED,	NCV_ROOMNOTCREATED, 	NCV_NETIDNOTFOUND,
	NCV_NONETIDONSYSTEM,

	NCV_MAX
	};

enum NCC_Type
	{
	NCC_ALL, NCC_USER, NCC_GROUP, NCC_ROOM,

	NCC_MAX
	};

struct nodeGroupList
	{
	struct ngl *next;
	label here;
	label there;
	};

class NodesCitC
	{
	PortSpeedE	Baud;			// speed to call out at
	int 	DialTimeout;		// Seconds
	int 	WaitTimeout;		// Seconds
	int 	FetchTimeout;		// Minutes

	label	Name;				// node name
	label	OldRegion;			// node region
	char	Alias[4];			// node alias
	char	LocID[5];			// node locID
	char	Protocol;			// transfer protocol
	h_slot	AutoHall;			// default autoroom hall
	g_slot	AutoGroup;			// default autoroom group

	char	PreDial[64];
	char	DialOut[50];		// DIAL_PREF + DialOut... send to modem

	ulong	RoomOffset; 		// old-style #ROOM icky stuff

	nodeGroupList *MappedGroups;// mapped groups

	char	LoginMacro[256];	// how to log in

	NETTYPES NetworkType;		// what style of networking
	int NetworkRevisionNumber;	// What revision?

	char	CreatePacket[41];	// how to create a packet
	char	ExtractPacket[41];	// how to extract files from a packet

	int 	AutoRoom;			// #AUTOROOM
	Bool	Verbose[NCV_MAX];	// #VERBOSE
	Bool	Chat[NCC_MAX];		// #CHAT
	label	MailFileName;		// what to call the mail file.

	int 	Request;			// #REQUEST
	Bool	Gateway;			// write out date in 'd' field.
	int 	Fetch;				// fetch type
	int 	NetFail;			// 0: by #TRAP. 1: always. -1: never
	int 	Redial; 			// how many times to attempt a redial
	g_slot	MapUnknownGroup;	// which group to map to if unknown
	int 	OutputPace; 		// how long to wait between chars

	Bool	MoreInfo69; 		// Send MOREINFO.69?

#ifdef WINCIT
	char	IpAddress[256];		// For TCP/IP based networking
	int		IpPort;				// For TCP/IP based networking
#endif

public:
	NodesCitC(void)
		{
		assert(this);
		MappedGroups = NULL;
		Clear();
		}

	~NodesCitC()
		{
		assert(this);
		FreeMappedGroups();
		}

	void FreeMappedGroups(void)
		{
		assert(this);
		VerifyHeap();
		disposeLL((void **) &MappedGroups);
		VerifyHeap();
		}

	void SetMapUnknownGroup(g_slot New)
		{
		assert(this);
		MapUnknownGroup = New;
		}

	void SetRequest(int New)
		{
		assert(this);
		Request = New;
		}

	void SetDefaultVerbose(void)
		{
		assert(this);
		memset(Verbose, 0, sizeof(Bool) * NCV_MAX);
		Verbose[NCV_FILE69IN] = TRUE;
		}

	void SetDefaultChat(void)
		{
		assert(this);
		memset(Chat, 0, sizeof(Bool) * NCC_MAX);
		Chat[NCC_USER] = TRUE;
		}

	void Clear(void)
		{
		assert(this);
		VerifyHeap();

		FreeMappedGroups();
		memset(this, 0, sizeof(*this));

		SetAutoHall(-1);		// Off
		SetAutoGroup(-1);		// Off
		SetMapUnknownGroup(SPECIALSECURITY);
		SetRequest(1);

		SetWaitTimeout(60); 	// Wait up to a minute.
		SetDialTimeout(60); 	// And wait up to a minute.
		SetFetchTimeout(35);	// And wait up to 35 minutes.

		SetDefaultVerbose();
		SetDefaultChat();

		SetBaud(cfg.initbaud);

#ifdef WINCIT
		SetIpPort(6969);
#endif

		VerifyHeap();
		}

	PortSpeedE GetBaud(void) const
		{
		assert(this);
		return (Baud);
		}

	void SetBaud(PortSpeedE New)
		{
		assert(this);
		assert(New >= 0);
		assert(New < PS_NUM);

		Baud = New;
		}

	int GetDialTimeout(void) const
		{
		assert(this);
		return (DialTimeout);
		}

	void SetDialTimeout(int New)
		{
		assert(this);
		assert(New >= 0);

		DialTimeout = New;
		}

	int GetFetchTimeout(void) const
		{
		assert(this);
		return (FetchTimeout);
		}

	void SetFetchTimeout(int New)
		{
		assert(this);
		assert(New >= 0);

		FetchTimeout = New;
		}

	int GetWaitTimeout(void) const
		{
		assert(this);
		return (WaitTimeout);
		}

	void SetWaitTimeout(int New)
		{
		assert(this);
		assert(New >= 0);

		WaitTimeout = New;
		}

	const char *GetName(void) const
		{
		assert(this);
		return (Name);
		}

	void SetName(const char *New)
		{
		assert(this);
		assert(New);
		assert(*New);

		CopyStringToBuffer(Name, New);
		}

	const char *GetOldRegion(void) const
		{
		assert(this);
		return (OldRegion);
		}

	void SetOldRegion(const char *New)
		{
		assert(this);
		assert(New);

		CopyStringToBuffer(OldRegion, New);
		}

	const char *GetAlias(void) const
		{
		assert(this);
		return (Alias);
		}

	void SetAlias(const char *New)
		{
		assert(this);
		assert(New);
		assert(*New);

		CopyStringToBuffer(Alias, New);
		}

	const char *GetLocID(void) const
		{
		assert(this);
		return (LocID);
		}

	void SetLocID(const char *New)
		{
		assert(this);
		assert(New);
		assert(*New);

		CopyStringToBuffer(LocID, New);
		}

	char GetProtocol(void) const
		{
		assert(this);
		return (Protocol);
		}

	void SetProtocol(char New)
		{
		assert(this);
		assert(New);

		Protocol = New;
		}

	h_slot GetAutoHall(void) const
		{
		assert(this);
		return (AutoHall);
		}

	void SetAutoHall(h_slot New)
		{
		assert(this);
		assert(New >= -1);
		assert(New < cfg.maxhalls);

		AutoHall = New;
		}

	g_slot GetAutoGroup(void) const
		{
		assert(this);
		return (AutoGroup);
		}

	void SetAutoGroup(g_slot New)
		{
		assert(this);
		assert(New >= -1);
		assert(New < cfg.maxgroups);

		AutoGroup = New;
		}

	const char *GetDialOut(void) const
		{
		assert(this);
		return (DialOut);
		}

	void SetDialOut(const char *New)
		{
		assert(this);
		assert(New);
		assert(*New);

		CopyStringToBuffer(DialOut, New);
		}

	const char *GetPreDial(void) const
		{
		assert(this);
		return (PreDial);
		}

	void SetPreDial(const char *New)
		{
		assert(this);
		assert(New);

		CopyStringToBuffer(PreDial, New);
		}

	ulong GetRoomOffset(void) const
		{
		assert(this);
		return (RoomOffset);
		}

	void SetRoomOffset(ulong New)
		{
		assert(this);
		RoomOffset = New;
		}

	const char *ThereToHereGroupName(const char *GroupName) const;

	Bool AddMappedGroups(const char *Here, const char *There)
		{
		assert(this);
		assert(Here);
		assert(*Here);

		nodeGroupList *NewEntry = (nodeGroupList *) addLL((void **) &MappedGroups, sizeof(nodeGroupList));

		if (NewEntry)
			{
			CopyStringToBuffer(NewEntry->here, Here);

			if (There && *There)
				{
				CopyStringToBuffer(NewEntry->there, There);
				}
			else
				{
				CopyStringToBuffer(NewEntry->there, Here);
				}

			return (TRUE);
			}
		else
			{
			return (FALSE);
			}
		}

	const char *GetLoginMacro(void) const
		{
		assert(this);
		return (LoginMacro);
		}

	void SetLoginMacro(char *New)
		{
		assert(this);
		assert(New);

		CopyStringToBuffer(LoginMacro, New);
		}


	NETTYPES GetNetworkType(void) const
		{
		assert(this);
		return (NetworkType);
		}

	void SetNetworkType(NETTYPES New)
		{
		assert(this);
		NetworkType = New;
		}

	int GetNetworkRevisionNumber(void) const
		{
		assert(this);
		return (NetworkRevisionNumber);
		}

	void SetNetworkRevisionNumber(int New)
		{
		assert(this);
		NetworkRevisionNumber = New;
		}

	const char *GetCreatePacket(void) const
		{
		assert(this);
		return (CreatePacket);
		}

	void SetCreatePacket(const char *New)
		{
		assert(this);
		assert(New);
		assert(*New);

		CopyStringToBuffer(CreatePacket, New);
		}

	const char *GetExtractPacket(void) const
		{
		assert(this);
		return (ExtractPacket);
		}

	void SetExtractPacket(const char *New)
		{
		assert(this);
		assert(New);
		assert(*New);

		CopyStringToBuffer(ExtractPacket, New);
		}

	Bool IsVerbose(NCV_Type TheType) const
		{
		assert(this);
		return (Verbose[TheType]);
		}

	void SetVerbose(NCV_Type TheType, Bool New)
		{
		assert(this);
		Verbose[TheType] = !!New;
		}

	Bool IsChat(NCC_Type TheType) const
		{
		assert(this);
		return (Chat[TheType]);
		}

	void SetChat(NCC_Type TheType, Bool New)
		{
		assert(this);
		Chat[TheType] = !!New;
		}

	Bool IsAutoRoom(void) const
		{
		assert(this);
		return ((Bool) AutoRoom);
		}

	int GetAutoRoom(void) const
		{
		assert(this);
		return (AutoRoom);
		}

	void SetAutoRoom(int New)
		{
		assert(this);
		AutoRoom = New;
		}

	const char *GetMailFileName(void) const
		{
		assert(this);
		return (MailFileName);
		}

	void SetMailFileName(const char *New)
		{
		assert(this);
		assert(New);
		assert(*New);

		CopyStringToBuffer(MailFileName, New);
		}

	int GetRequest(void) const
		{
		assert(this);
		return (Request);
		}

	Bool IsGateway(void) const
		{
		assert(this);
		return (Gateway);
		}

	void SetGateway(Bool New)
		{
		assert(this);
		Gateway = !!New;
		}

	int GetFetch(void) const
		{
		assert(this);
		return (Fetch);
		}

	void SetFetch(int New)
		{
		assert(this);
		Fetch = New;
		}

	int GetNetFail(void) const
		{
		assert(this);
		return (NetFail);
		}

	void SetNetFail(int New)
		{
		assert(this);
		NetFail = New;
		}

	int GetRedial(void) const
		{
		assert(this);
		return (Redial);
		}

	void SetRedial(int New)
		{
		assert(this);
		Redial = New;
		}

	g_slot GetMapUnknownGroup(void) const
		{
		assert(this);
		return (MapUnknownGroup);
		}

	int GetOutputPace(void) const
		{
		assert(this);
		return (OutputPace);
		}

	void SetOutputPace(int New)
		{
		assert(this);
		assert(New >= 0);
		OutputPace = New;
		}

#ifdef WINCIT
	const char *GetIpAddress(void) const
		{
		assert(this);
		return (IpAddress);
		}

	void SetIpAddress(const char *New)
		{
		assert(this);
		assert(New);
		assert(*New);

		CopyStringToBuffer(IpAddress, New);
		}

	int GetIpPort(void) const
		{
		assert(this);
		return (IpPort);
		}

	void SetIpPort(int New)
		{
		assert(this);
		IpPort = New;
		}
#endif
	};


// --------------------------------------------------------------------------
// Over time, this should become the exclusive networking model: no longer
// logging in the node as a normal user.  This will make the networking code
// much cleaner and easier to maintain.

#include "log.h"
class NetLogEntryC : public LogEntry
	{
public:
	NetLogEntryC(void) :
		LogEntry(cfg.maxrooms, cfg.maxgroups, 0)
		{
		}

#ifdef WINCIT
	Bool MakeNet69RoomRequestFile(ModemConsoleE ReportTo, TermWindowC *TW);
#else
	Bool MakeNet69RoomRequestFile(ModemConsoleE ReportTo);
#endif
	};

enum NODEKEYWORDS
	{
	NOK_BAUD,			NOK_GROUP,
	NOK_LOGIN,			NOK_PROTOCOL,
	NOK_GATEWAY,		NOK_WAIT_TIMEOUT,	NOK_NODE,
	NOK_PHONE,			NOK_DIAL_TIMEOUT,	NOK_ROOM,
	NOK_NETWORK,
	NOK_ZIP,			NOK_AUTOROOM,
	NOK_VERBOSE,		NOK_REQUEST,		NOK_FETCH,
	NOK_NETFAIL,		NOK_REDIAL, 		NOK_AUTOHALL,
	NOK_AUTOGROUP,		NOK_MAPUNKGROUP,	NOK_OUTPUTPACE,
	NOK_MOREINFO69, 	NOK_DIALOUT,		NOK_PREDIAL,
	NOK_FETCH_TIMEOUT,	NOK_IPADDRESS,		NOK_IPPORT,
	NOK_CHAT,

	NOK_NUM,
	};

// for network commands
enum
	{
	NC_DEBUG,			NC_ROOMINFO,		NC_NODELIST,
	NC_NODEINFO,		NC_POOPCOUNT,		NC_POOPUSER,
	NC_FINGER,

	NC_NUM
	};


class NetMessagePacketC: MessageStoreC
	{
	FILE *NetFile;
	NodesCitC *Node;

	virtual Bool SaveByte(char Byte);
	virtual Bool SaveString(const char *String);

	virtual Bool GetByte(char *Byte);

public:
	NetMessagePacketC(FILE *NewFile, NodesCitC *NewNode)
		{
		NetFile = NewFile;
		Node = NewNode;
		NetPacket = TRUE;
		}

	ReadMessageStatus LoadAll(Message *Msg)
		{
		ReadMessageStatus RMS = MessageStoreC::LoadAll(Msg);

		if (RMS == RMS_OK)
			{
			Msg->FixupNetwork(Node);
			}

		return (RMS);
		}

	Bool Store(Message *Msg)
		{
		return (MessageStoreC::Store(Msg));
		}
	};


// NETMAIL.CPP
void parse_recipient(const char *str, char *user, char *node, char *region);
Bool save_mail(Message *Msg, Bool Stub, const char *StubMsg, l_index SkipLog);
Bool BeenThere(const char *path, const char *nodename, const char *alias, const char *locID, char *next);
Bool isaddress(const char *str);
r_slot FindRoomForNetMail(const char *str);
void parse_address(const char *str, char *alias, char *locID, Bool option);
const char *makeaddress(const char *alias, const char *locID, char *AddressBuffer);


// NETIDCIT.CPP
Bool IsNetIDInNetIDCit(const char *id);
Bool AddNetIDToNetIDCit(const char *NetID, const char *Comment);
Bool RemoveNetIDFromNetIDCit(const char *NetID);
Bool GetCommentOfNetIDInNetIDCit(const char *NetID, char *Comment, int Len);


// NETNODE.CPP
void freeNode(NodesCitC **node);

// NetMsgO.cpp
Message *ShouldNetworkMessage(m_index ID, r_slot Room, LogEntry *Log);
