// --------------------------------------------------------------------------
// Citadel: FSCfg.CPP
//
// Full-Screen Configuration stuff


#ifndef WINCIT
#include "ctdl.h"
#pragma hdrstop

#include "fscfg.h"
#include "cwindows.h"
#include "cfgfiles.h"
#include "readcfg.h"
#include "config.h"
#include "hall.h"
#include "log.h"
#include "filemake.h"
#include "miscovl.h"
#include "extmsg.h"
#include "room.h"


// all of the control IDs used in this module
enum
	{
	CTRL_COMPILEDICTSTR,	CTRL_CIT86PATHSTR,
	};

static Bool KeepGoing;

static void fscDump(void)
	{
	dump_fs_messages();
	}

static unsigned int PowersOf2Minus1[] =
	{
	0u, 	1u, 	3u, 	7u, 	15u,	31u,	63u,	127u,
	255u,	511u,	1023u,	2047u,	4095u,	8191u,	16383u, 32767u,
	65535u,
	};

static void WriteChar(char ToWrite, FILE *F)
	{
	static char ByteBuffer;
	static int BitsInByteBuffer;

	if (F)
		{
		if (ToWrite == '\'')
			{
			ToWrite = 27;
			}
		else if (ToWrite)
			{
			ToWrite -= '@';
			}

		for (int BitsWritten = 0; BitsWritten < 5;)
			{
			int BitsToWrite = 8 - BitsInByteBuffer;

			if (BitsToWrite > 5 - BitsWritten)
				{
				BitsToWrite = 5 - BitsWritten;
				}

			ByteBuffer |= ((ToWrite >> BitsWritten) &
					(PowersOf2Minus1[BitsToWrite])) << BitsInByteBuffer;

			BitsInByteBuffer += BitsToWrite;
			BitsWritten += BitsToWrite;

			if (BitsInByteBuffer == 8)
				{
				fwrite(&ByteBuffer, 1, sizeof(char), F);
				BitsInByteBuffer = 0;
				ByteBuffer = 0;
				}
			}

		}
	else
		{
		ByteBuffer = 0;
		BitsInByteBuffer = 0;
		}
	}

static Bool WriteWord(const char *Word, FILE *F)
	{
	Word++;

	do
		{
		WriteChar(*Word, F);
		} while (*(Word++));

	return (TRUE);
	}

static void Cit86Crypte(void *buf, uint len, uint seed, uint cfgSeed)
	{
	uchar *b;
	int s;

	seed = (seed + cfgSeed) & 0xFF;
	b = (uchar *) buf;
	s = seed;

	for (; len; len--)
		{
		*b++ ^= s;
		s = (s + 117) & 0xFF;
		}
	}

static void CreateCitadel86FileName(const char *MainDir, const char *SubDir,
		const char *FName, char *FillPath)
	{
	sprintf(FillPath, (MainDir[strlen(MainDir) - 1] == '\\') ?
			getmsg(472) : sbs, MainDir, SubDir);

	if (*SubDir)
		{
		strcat(FillPath, getfsmsg(32));
		}

	strcat(FillPath, FName);
	strupr(FillPath);
	}


class Cit86MsgBase
	{
	FILE *File;
	uchar MsgData[128];
	int SectOffset, MsgSect;
	Bool HaveData, PastEnd;
	uint CryptSeed;

	void readSector(void)
		{
		if (fread(MsgData, sizeof(MsgData), 1, File) == 1)
			{
			Cit86Crypte(MsgData, sizeof(MsgData), 0, CryptSeed);
			HaveData = TRUE;
			}
		else
			{
			HaveData = FALSE;
			}
		}

public:
	Cit86MsgBase(uint CSeed)
		{
		File = NULL;
		MsgSect = SectOffset = 0;
		HaveData = PastEnd = FALSE;
		CryptSeed = CSeed;
		}

	Bool Open(const char *FName)
		{
		if (File)
			{
			return (FALSE);
			}

		File = fopen(FName, FO_RB);

		return (File != NULL);
		}

	Bool Close(void)
		{
		if (!File)
			{
			return (FALSE);
			}

		fclose(File);
		File = NULL;
		return (TRUE);
		}

	long GetLocation(void)
		{
		return (((long) MsgSect * 128l) + (long) SectOffset);
		}

	Bool IsPastEnd(void)
		{
		if (PastEnd)
			{
			PastEnd = FALSE;
			return (TRUE);
			}
		else
			{
			return (FALSE);
			}
		}

	uchar ReadCharacter(void);

	void Seek(long Byte)
		{
		MsgSect = (int) (Byte / 128l);
		SectOffset = (int) (Byte % 128l);
		HaveData = FALSE;
		}
	};

uchar Cit86MsgBase::ReadCharacter(void)
	{
	if (!HaveData)
		{
		readSector();
		}

	if (SectOffset == 128)
		{
		SectOffset = 0;
		MsgSect++;

		readSector();

		if (!HaveData)
			{
			PastEnd = TRUE;
			MsgSect = 0;

			fseek(File, 0, SEEK_SET);
			readSector();
			}
		}

	return (MsgData[SectOffset++]);
	}

static void ImportCit86Data(CITWINDOW *Parent, const char *PathOnly)
	{
	// First 19200 is Citadel-86 14400; 57600 is Citadel-86 58400
	long Cit86Bauds[] = { 300l, 1200l, 2400l, 4800l, 9600l, 19200l, 19200l,
			38400l, 57600l };

	char FullPath[80];

	CreateCitadel86FileName(PathOnly, ns, getfsmsg(13), FullPath);

	FILE *File = fopen(FullPath, FO_R);

	if (!File)
		{
		CitWindowsError(Parent, getmsg(78), FullPath);
		return;
		}

	int CryptSeed = 0, MaxRooms = 0, LogSize = 0, EnterOK = 0, ReadOK = 0,
			RoomOK = 0, MData = 0, DiskFree = 0, MsgSlots = 0, MailSlots = 0,
			Cit86MaxRooms = 0;
	long MessageK = 0, Baud = 0;
	label HelpArea, LogArea, RoomArea, MsgArea, FloorArea, HoldArea,
			InfoArea, BioArea, NodeName, Sysop;
	char ModSetup[64];

	HelpArea[0] = LogArea[0] = RoomArea[0] = MsgArea[0] = FloorArea[0] =
			HoldArea[0] = InfoArea[0] = BioArea[0] = NodeName[0] =
			ModSetup[0] = Sysop[0] = 0;

	CITWINDOW *Msg = CitWindowsMsg(Parent, getfsmsg(15), getfsmsg(13));

	char line[257];

	while (fgets(line, 256, File) != NULL)
		{
		if (line[0] == '#')
			{
			char *words[256];
			parse_it(words, line);

			if (SameString(words[0], getfsmsg(17)))
				{
				CryptSeed = atoi(words[1]);
				}
			else if (SameString(words[0], getfsmsg(18)))
				{
				MessageK = atol(words[1]);
				}
			else if (SameString(words[0], getfsmsg(19)))
				{
				Cit86MaxRooms = MaxRooms = atoi(words[1]);
				}
			else if (SameString(words[0], getfsmsg(20)))
				{
				LogSize = atoi(words[1]);
				}
			else if (SameString(words[0], getfsmsg(33)))
				{
				EnterOK = atoi(words[1]);
				}
			else if (SameString(words[0], getfsmsg(34)))
				{
				ReadOK = atoi(words[1]);
				}
			else if (SameString(words[0], getfsmsg(35)))
				{
				RoomOK = atoi(words[1]);
				}
			else if (SameString(words[0], getfsmsg(36)))
				{
				MData = atoi(words[1]);
				}
			else if (SameString(words[0], getfsmsg(37)))
				{
				Baud = Cit86Bauds[atoi(words[1])];
				}
			else if (SameString(words[0], getfsmsg(40)))
				{
				DiskFree = atoi(words[1]);
				}
			else if (SameString(words[0], getfsmsg(43)))
				{
				MsgSlots = atoi(words[1]);
				}
			else if (SameString(words[0], getfsmsg(44)))
				{
				MailSlots = atoi(words[1]);
				}
			else if (SameString(words[0], getfsmsg(21)))
				{
				CopyStringToBuffer(HelpArea, words[1]);
				}
			else if (SameString(words[0], getfsmsg(22)))
				{
				CopyStringToBuffer(LogArea, words[1]);
				}
			else if (SameString(words[0], getfsmsg(23)))
				{
				CopyStringToBuffer(RoomArea, words[1]);
				}
			else if (SameString(words[0], getfsmsg(24)))
				{
				CopyStringToBuffer(MsgArea, words[1]);
				}
			else if (SameString(words[0], getfsmsg(25)))
				{
				CopyStringToBuffer(FloorArea, words[1]);
				}
			else if (SameString(words[0], getfsmsg(26)))
				{
				CopyStringToBuffer(HoldArea, words[1]);
				}
			else if (SameString(words[0], getfsmsg(27)))
				{
				CopyStringToBuffer(InfoArea, words[1]);
				}
			else if (SameString(words[0], getfsmsg(28)))
				{
				CopyStringToBuffer(BioArea, words[1]);
				}
			else if (SameString(words[0], getfsmsg(31)))
				{
				CopyStringToBuffer(NodeName, words[1]);
				}
			else if (SameString(words[0], getfsmsg(38)))
				{
				CopyStringToBuffer(ModSetup, words[1]);
				}
			else if (SameString(words[0], getfsmsg(39)))
				{
				CopyStringToBuffer(Sysop, words[1]);
				}
			}
		}

	fclose(File);

	if (Msg)
		{
		destroyCitWindow(Msg, FALSE);
		}

	if (!MessageK)
		{
		CitWindowsError(Parent, getmsg(42), getfsmsg(18), getfsmsg(13));
		return;
		}

	if (!MaxRooms)
		{
		CitWindowsError(Parent, getmsg(42), getfsmsg(19), getfsmsg(13));
		return;
		}

	MaxRooms++; // Make sure we have room for our Dump>

	if (MaxRooms < 16)
		{
		MaxRooms = 16;
		}

	if (!LogSize)
		{
		CitWindowsError(Parent, getmsg(42), getfsmsg(20), getfsmsg(13));
		return;
		}

	if (!MsgSlots)
		{
		CitWindowsError(Parent, getmsg(42), getfsmsg(43), getfsmsg(13));
		return;
		}

	if (!MailSlots)
		{
		CitWindowsError(Parent, getmsg(42), getfsmsg(44), getfsmsg(13));
		return;
		}

	discardable *CfgData = readData(0, 1, 6);

	if (!CfgData)
		{
		CitWindowsError(Parent, getmsg(188), getfsmsg(30));
		return;
		}

	if (!read_cfg_messages())
		{
		CitWindowsError(Parent, getmsg(188), getfsmsg(30));
		discardData(CfgData);
		return;
		}

	Msg = CitWindowsMsg(Parent, getfsmsg(29));

	// Give us a homepath of "." for stuff that needs it...
	CopyStringToBuffer(cfg.homepath, getfsmsg(52));

	File = fopen(citfiles[C_CONFIG_CIT], FO_W);

	if (!File)
		{
		if (Msg)
			{
			destroyCitWindow(Msg, FALSE);
			}

		CitWindowsError(Parent, getmsg(78), citfiles[C_CONFIG_CIT]);

		dump_cfg_messages();
		discardData(CfgData);
		compactMemory(1);

		return;
		}

#define keyword(nmbr) ((const char **) CfgData->aux)[(uint)(nmbr)]
	char stuff[80];

	if (*NodeName)
		{
		fprintf(File, getcfgmsg(271), keyword(K_NODENAME),
				mkDblBck((char *) NodeName, stuff), bn);
		}

	if (*ModSetup)
		{
		fprintf(File, getcfgmsg(271), keyword(K_MODSETUP),
				mkDblBck((char *) ModSetup, stuff), bn);
		}

	if (*Sysop)
		{
		fprintf(File, getcfgmsg(271), keyword(K_SYSOP),
				mkDblBck((char *) Sysop, stuff), bn);
		}

	fprintf(File, getfsmsg(41), keyword(K_MESSAGEK), MessageK, bn);
	fprintf(File, getcfgmsg(272), keyword(K_MAXROOMS), MaxRooms, bn);
	fprintf(File, getcfgmsg(272), keyword(K_MAXLOGTAB), LogSize, bn);

	if (Baud)
		{
		fprintf(File, getfsmsg(41), keyword(K_INIT_BAUD), Baud, bn);
		}

	if (DiskFree)
		{
		fprintf(File, getcfgmsg(272), keyword(K_DISKFREE), DiskFree, bn);
		}

	if (MData)
		{
		fprintf(File, getcfgmsg(272), keyword(K_MDATA), MData, bn);
		}

	fprintf(File, getcfgmsg(272), keyword(K_ENTEROK), EnterOK, bn);
	fprintf(File, getcfgmsg(272), keyword(K_READOK), ReadOK, bn);
	fprintf(File, getcfgmsg(272), keyword(K_ROOMOK), RoomOK, bn);

	fclose(File);

	CreateConfigurationFile(C_EXTERNAL_CIT, 0, FALSE);
	CreateConfigurationFile(C_NODES_CIT, 1, FALSE);
	CreateConfigurationFile(C_GRPDATA_CIT, 2, FALSE);
	CreateConfigurationFile(C_CRON_CIT, 3, FALSE);
	CreateConfigurationFile(C_MDMRESLT_CIT, 4, FALSE);
	CreateConfigurationFile(C_PROTOCOL_CIT, 5, FALSE);
	CreateConfigurationFile(C_TERMCAP_CIT, 6, FALSE);
	CreateConfigurationFile(C_COMMANDS_CIT, 7, FALSE);

	if (Msg)
		{
		destroyCitWindow(Msg, FALSE);
		}

	// Create hall data from Citadel-86's floors...
	HallBuffer NewHalls;
	h_slot MaxFloors;

	CreateCitadel86FileName(PathOnly, FloorArea, getfsmsg(45), FullPath);

	File = fopen(FullPath, FO_RB);

	if (File)
		{
		Msg = CitWindowsMsg(Parent, getfsmsg(46));

// Citadel-86 is byte-aligned
#pragma option -a-
		struct
			{
			char Name[20];
			char Inuse;
			char Moderator[20];
			} Cit86Floor;
#pragma option -a.

		h_slot CurHall = 0;

		// +1 so we have room for Maintenance.
		MaxFloors = (int) (filelength(fileno(File)) /
				(long) sizeof(Cit86Floor)) + 1;

		if (MaxFloors < 16)
			{
			MaxFloors = 16;
			}

		NewHalls.Resize(MaxFloors, MaxRooms);

		if (!NewHalls.IsValid())
			{
			if (Msg)
				{
				destroyCitWindow(Msg, FALSE);
				}

			fclose(File);

			CitWindowsError(Parent, getmsg(188), getfsmsg(47));

			dump_cfg_messages();
			discardData(CfgData);
			compactMemory(1);

			return;
			}

		while(fread(&Cit86Floor, sizeof(Cit86Floor), 1, File) == 1)
			{
			if (Cit86Floor.Inuse)
				{
				NewHalls[CurHall].SetInuse(TRUE);
				NewHalls[CurHall].SetName(Cit86Floor.Name);

				// Lobby in all halls.
				NewHalls[CurHall].SetRoomInHall(1, TRUE);
				NewHalls[CurHall].SetWindowedIntoHall(1, TRUE);
				}
			else
				{
				NewHalls[CurHall].SetInuse(FALSE);
				}

			CurHall++;

			// Skip Maintenance.
			if (CurHall == 1)
				{
				NewHalls[1].SetInuse(TRUE);
				NewHalls[1].SetName(getcfgmsg(169));

				CurHall++;
				}
			}

		fclose(File);

		NewHalls.Save();

		if (Msg)
			{
			destroyCitWindow(Msg, FALSE);
			}
		}
	else
		{
		CitWindowsError(Parent, getmsg(78), FullPath);

		dump_cfg_messages();
		discardData(CfgData);
		compactMemory(1);

		return;
		}

	// Now import rooms...
	CreateCitadel86FileName(PathOnly, RoomArea, getfsmsg(48), FullPath);

	struct RoomDataSave
		{
		uint Generation : 5;
		Bool Public : 1;
		};

	RoomDataSave *RoomData = new RoomDataSave[Cit86MaxRooms];

	if (!RoomData)
		{
		CitWindowsError(Parent, getmsg(188), getfsmsg(47));

		dump_cfg_messages();
		discardData(CfgData);
		compactMemory(1);

		return;
		}

	memset(RoomData, 0, sizeof(RoomDataSave) * Cit86MaxRooms);

	File = fopen(FullPath, FO_RB);

	if (File)
		{
		FILE *File2 = fopen(roomDat, FO_WB);

		if (!File2)
			{
			fclose(File);

			CitWindowsError(Parent, getmsg(78), roomDat);

			delete [] RoomData;
			dump_cfg_messages();
			discardData(CfgData);
			compactMemory(1);

			return;
			}

		FILE *File3 = fopen(getfsmsg(50), FO_W);

		if (!File3)
			{
			fclose(File);
			fclose(File2);

			CitWindowsError(Parent, getmsg(78), getfsmsg(50));

			delete [] RoomData;
			dump_cfg_messages();
			discardData(CfgData);
			compactMemory(1);

			return;
			}

		Msg = CitWindowsMsg(Parent, getfsmsg(49));

// Citadel-86 is byte-aligned
#pragma option -a-
		struct
			{
			uchar Generation;		// Remember these?

			struct
				{
				Bool InUse : 1;
				Bool Public : 1;
				Bool IsDir : 1;
				Bool Permanent : 1;
				Bool Skipped : 1;
				Bool Upload : 1;
				Bool Download : 1;
				Bool Shared : 1;
				Bool Archived : 1;
				Bool Anonymous : 1;
				Bool NoNetDownload : 1;
				Bool Invite : 1;
				Bool AutoNet : 1;
				Bool AllNet : 1;
				Bool ReadOnly : 1;
				Bool Unforgettable : 1;
				Bool Flag0 : 1;
				Bool Flag1 : 1;
				Bool Flag2 : 1;
				Bool Flag3 : 1;
				Bool Flag4 : 1;
				Bool Flag5 : 1;
				Bool Flag6 : 1;
				Bool Flag7 : 1;
				Bool Flag8 : 1;
				Bool Flag9 : 1;
				} Flags;

			char Name[20];

			uint Floor;
			} Cit86Room;
#pragma option -a.

		r_slot CurRoom = 0, Cit86RoomNum = 0;
		RoomC CitPlusRoom;

		long l = CitPlusRoom.GetDiskRecordSize();
		fwrite(&l, sizeof(l), 1, File2);

		while(fread(&Cit86Room, sizeof(Cit86Room), 1, File) == 1)
			{
			Cit86Crypte(&Cit86Room, sizeof(Cit86Room), Cit86RoomNum, CryptSeed);

			RoomData[Cit86RoomNum].Generation = Cit86Room.Generation & 31;
			RoomData[Cit86RoomNum].Public = Cit86Room.Flags.Public;

			CitPlusRoom.SetPublic(Cit86Room.Flags.Public);
			CitPlusRoom.SetInuse(Cit86Room.Flags.InUse);
			CitPlusRoom.SetPermanent(Cit86Room.Flags.Permanent);
			CitPlusRoom.SetShared(Cit86Room.Flags.Shared);
			CitPlusRoom.SetAnonymous(Cit86Room.Flags.Anonymous);

			CitPlusRoom.SetName(Cit86Room.Name);

			CitPlusRoom.SetUnexcludable(Cit86Room.Flags.Unforgettable);

			// Fix up some hall stuff...
			if (Cit86Room.Flags.InUse)
				{
				// Put it in Maintenance
				NewHalls[1].SetRoomInHall(CurRoom, TRUE);

				// And in other hall...
				h_slot TheHall = Cit86Room.Floor;

				// Skip maintenance...
				if (TheHall > 0)
					{
					TheHall++;
					}

				if (TheHall < MaxFloors && TheHall >= 0 &&
						NewHalls[TheHall].IsInuse())
					{
					NewHalls[TheHall].SetRoomInHall(CurRoom, TRUE);
					}

				if (CurRoom == AIDEROOM)
					{
					CitPlusRoom.SetPublic(FALSE);

					NewHalls[TheHall].SetWindowedIntoHall(CurRoom, TRUE);
					NewHalls[1].SetWindowedIntoHall(CurRoom, TRUE);
					}

				// Now get messages from room...
				for (int MCounter = 0; MCounter < MsgSlots; MCounter++)
					{
					// Message for this room.
					fread(&l, sizeof(l), 1, File);

					if (l)
						{
						// msgnum roomnum. (e.g. "1000 3")
						fprintf(File3, getfsmsg(51), l, CurRoom, bn);
						}

					// Skip sector ID.
					fread(&l, sizeof(int), 1, File);
					}
				}
			else
				{
				// Skip messages in room...
				fseek(File, (sizeof(long) + sizeof(int)) * MsgSlots, SEEK_CUR);
				}

			// This is a bit icky.
			fwrite(&CitPlusRoom, sizeof(CitPlusRoom), 1, File2);

			CurRoom++;

			// Make Dump>
			if (CurRoom == DUMP)
				{
				CitPlusRoom.SetInuse(TRUE);
				CitPlusRoom.SetPermanent(TRUE);
				CitPlusRoom.SetShared(FALSE);
				CitPlusRoom.SetAnonymous(FALSE);
				CitPlusRoom.SetPublic(TRUE);

				CitPlusRoom.SetUnexcludable(FALSE);

				CitPlusRoom.SetName(getcfgmsg(147));

				// This is a bit icky.
				fwrite(&CitPlusRoom, sizeof(CitPlusRoom), 1, File2);
				CurRoom++;
				}

			Cit86RoomNum++;
			}

		fclose(File3);
		fclose(File2);
		fclose(File);

		if (Msg)
			{
			destroyCitWindow(Msg, FALSE);
			}

		NewHalls.Save();
		}
	else
		{
		CitWindowsError(Parent, getmsg(78), FullPath);

		delete [] RoomData;
		dump_cfg_messages();
		discardData(CfgData);
		compactMemory(1);

		return;
		}


	// Import log...
	CreateCitadel86FileName(PathOnly, RoomArea, getfsmsg(53), FullPath);

	File = fopen(FullPath, FO_RB);

	if (File)
		{
		FILE *File2 = fopen(getfsmsg(50), FO_A);

		if (!File2)
			{
			fclose(File);

			CitWindowsError(Parent, getmsg(78), getfsmsg(50));

			delete [] RoomData;
			dump_cfg_messages();
			discardData(CfgData);
			compactMemory(1);

			return;
			}

		Msg = CitWindowsMsg(Parent, getfsmsg(55));

		// Log save stuff updates the table...
		cfg.MAXLOGTAB = LogSize;
		if (!LogTab.Resize(cfg.MAXLOGTAB))
			{
			crashout(getcfgmsg(116));
			}

// Citadel-86 is byte-aligned
#pragma option -a-
		struct
			{
			uchar Nulls;

			struct
				{
				Bool Floors : 1;
				Bool LFMask : 1;
				Bool Expert : 1;
				Bool Aide : 1;
				Bool InUse : 1;
				Bool PrintTime : 1;
				Bool OldToo : 1;
				Bool Network : 1;
				Bool Ruggie : 1;
				Bool HalfDuplex : 1;
				Bool Twit : 1;
				Bool Doors : 1;
				Bool Permanent : 1;
				Bool DLPrivs : 1;
				Bool AltRE : 1;
				Bool NoMsgEditorPrompt : 1;
				Bool Flag0 : 1;
				Bool Flag1 : 1;
				Bool Flag2 : 1;
				Bool Flag3 : 1;
				Bool Flag4 : 1;
				Bool Flag5 : 1;
				Bool Flag6 : 1;
				Bool Flag7 : 1;
				Bool Flag8 : 1;
				Bool Flag9 : 1;
				} Flags;

			uchar Width;
			int LDCredits;
			char Name[20];
			char Password[20];
			long LastOn;
			uchar Delay;
			uchar ScreenLength;
			} Cit86Log;
#pragma option -a.

		// 16 groups, 5 jumpback. Why 5? Because I want it to be 5.
		LogEntry *CitPlusLog = new LogEntry(MaxRooms, 16, 5);

		if (!CitPlusLog)
			{
			if (Msg)
				{
				destroyCitWindow(Msg, FALSE);
				}

			CitWindowsError(Parent, getmsg(188), getfsmsg(47));

			fclose(File);
			delete [] RoomData;
			dump_cfg_messages();
			discardData(CfgData);
			compactMemory(1);

			return;
			}

		CopyStringToBuffer(cfg.logextdir, getfsmsg(52));

		cfg.maxrooms = MaxRooms;
		cfg.maxgroups = 16;
		cfg.maxjumpback = 5;
		cfg.MAXLOGTAB = LogSize;
		zapLogFile(FALSE, FALSE);

		l_slot CurLog = 0;

		// set all default defaults...
		CitPlusLog->SetCredits(3600);

		CitPlusLog->SetViewRoomDesc(TRUE);
		CitPlusLog->SetEnterBorders(TRUE);
		CitPlusLog->SetViewTitleSurname(TRUE);
		CitPlusLog->SetViewSubjects(TRUE);
		CitPlusLog->SetViewSignatures(TRUE);
		CitPlusLog->SetMinibin(TRUE);
		CitPlusLog->SetViewRoomInfoLines(TRUE);
		CitPlusLog->SetViewHallDescription(TRUE);
		CitPlusLog->SetViewBorders(TRUE);

		CitPlusLog->SetPromptFormat(getcfgmsg(253));
		CitPlusLog->SetDateStamp(getcfgmsg(204));
		CitPlusLog->SetVerboseDateStamp(getcfgmsg(250));
		CitPlusLog->SetNetPrefix(getcfgmsg(252));
		CitPlusLog->SetMorePrompt(getcfgmsg(256));

		// Everybody's in NULL.
		CitPlusLog->SetInGroup(0, TRUE);

		while(fread(&Cit86Log, sizeof(Cit86Log), 1, File) == 1)
			{
			Cit86Crypte(&Cit86Log, sizeof(Cit86Log), CurLog * 3, CryptSeed);

			CitPlusLog->SetFinger(NULL);

			CitPlusLog->SetNulls(Cit86Log.Nulls);
			CitPlusLog->SetAutoNextHall(!Cit86Log.Flags.Floors);
			CitPlusLog->SetLinefeeds(Cit86Log.Flags.LFMask);
			CitPlusLog->SetExpert(Cit86Log.Flags.Expert);
			CitPlusLog->SetAide(Cit86Log.Flags.Aide);
			CitPlusLog->SetInuse(Cit86Log.Flags.InUse);
			CitPlusLog->SetOldToo(Cit86Log.Flags.OldToo);
			CitPlusLog->SetNetUser(Cit86Log.Flags.Network);
			CitPlusLog->SetProblem(Cit86Log.Flags.Twit);
			CitPlusLog->SetPermanent(Cit86Log.Flags.Permanent);
			CitPlusLog->SetDownload(Cit86Log.Flags.DLPrivs);
			CitPlusLog->SetWidth(Cit86Log.Width);
			CitPlusLog->SetName(Cit86Log.Name);
			CitPlusLog->SetPassword(Cit86Log.Password);
			CitPlusLog->SetCallTime(Cit86Log.LastOn);
			CitPlusLog->SetLinesPerScreen(Cit86Log.ScreenLength);

			CitPlusLog->SetInRoom(DUMP, TRUE);

			for (r_slot Room = 0; Room < Cit86MaxRooms; Room++)
				{
				uchar Generation;

				fread(&Generation, sizeof(Generation), 1, File);

				r_slot RoomNum = Room;

				if (RoomNum >= DUMP)
					{
					RoomNum++;
					}

				if (abs(RoomData[Room].Generation - Generation) == 16)
					{
					CitPlusLog->SetInRoom(RoomNum, TRUE);
					CitPlusLog->SetRoomExcluded(RoomNum, TRUE);
					}
				else if (RoomData[Room].Public)
					{
					CitPlusLog->SetInRoom(RoomNum, TRUE);
					}
				else
					{
					CitPlusLog->SetInRoom(RoomNum,
							(RoomData[Room].Generation == Generation));
					}
				}

			for (int Mail = 0; Mail < MailSlots; Mail++)
				{
				long l;

				// Mail message for this user.
				fread(&l, sizeof(l), 1, File);

				if (l)
					{
					// msgnum roomnum. (e.g. "1000 2")
					fprintf(File2, getfsmsg(51), l, MAILROOM, bn);
					}

				// Skip sector ID.
				fread(&l, sizeof(int), 1, File);
				} 

			for (int Room = 0; Room < Cit86MaxRooms; Room++)
				{
				long Pointer;

				fread(&Pointer, sizeof(Pointer), 1, File);


				r_slot RoomNum = Room;

				if (RoomNum >= DUMP)
					{
					RoomNum++;
					}

				CitPlusLog->SetRoomNewPointer(RoomNum, Pointer);
				}

			// Bring bio info over...
			char BioName[20];
			sprintf(BioName, getfsmsg(56), CurLog);

			CreateCitadel86FileName(PathOnly, BioArea, BioName, FullPath);

			FILE *File3 = fopen(FullPath, FO_RB);

			if (File3)
				{
				char *BioText = new char[MAXTEXT];

				if (BioText)
					{
					fread(BioText, sizeof(char), 1, File3);

					Cit86Crypte(BioText, MAXTEXT, CurLog, CryptSeed);

					CitPlusLog->SetFinger(BioText);

					delete [] BioText;
					}

				fclose(File3);
				}


			CitPlusLog->Save(CurLog, 0);

			CurLog++;
			}

		fclose(File2);

		delete CitPlusLog;

		LogTab.Resize(0);
		fclose(File);

		if (Msg)
			{
			destroyCitWindow(Msg, FALSE);
			}
		}
	else
		{
		CitWindowsError(Parent, getmsg(78), FullPath);

		delete [] RoomData;
		dump_cfg_messages();
		discardData(CfgData);
		compactMemory(1);

		return;
		}


	// Now import messages...
	Cit86MsgBase Cit86Messages(CryptSeed);

	CreateCitadel86FileName(PathOnly, MsgArea, getfsmsg(57), FullPath);

	if (Cit86Messages.Open(FullPath))
		{
		long FirstMsgOffset = -1, FirstMsgNum = LONG_MAX;

		Msg = CitWindowsMsg(Parent, getfsmsg(58));

		while(!Cit86Messages.IsPastEnd())
			{
			if (Cit86Messages.ReadCharacter() == 0xff)
				{
				long ThisMsgOffset = Cit86Messages.GetLocation() - 1;

				char MsgID[20];

				for (int i = 0; i < 19; i++)
					{
					MsgID[i] = Cit86Messages.ReadCharacter();

					if (!MsgID[i])
						{
						break;
						}
					}
				MsgID[19] = 0;

				long ThisMsgNum = atol(MsgID);

				if (ThisMsgNum < FirstMsgNum)
					{
					FirstMsgNum = ThisMsgNum;
					FirstMsgOffset = ThisMsgOffset;
					}
				}
			}

		if (Msg)
			{
			destroyCitWindow(Msg, FALSE);
			}

		if (FirstMsgOffset == -1)
			{
			Cit86Messages.Close();
			CitWindowsError(Parent, getfsmsg(59));

			delete [] RoomData;
			dump_cfg_messages();
			discardData(CfgData);
			compactMemory(1);

			return;
			}

		Msg = CitWindowsMsg(Parent, getfsmsg(60));

		Cit86Messages.Seek(FirstMsgOffset);

		long LastMsgLoaded = -1;

		Bool MoreMessages;

		do
			{
			MoreMessages = FALSE;

			// Load this message
			while (Cit86Messages.ReadCharacter() != 0xff);

			char MsgID[20];

			for (int i = 0; i < 19; i++)
				{
				MsgID[i] = Cit86Messages.ReadCharacter();

				if (!MsgID[i])
					{
					break;
					}
				}
			MsgID[19] = 0;

			long ThisMsgNum = atol(MsgID);

			if (ThisMsgNum > LastMsgLoaded)
				{
				// Save it to MSG.DAT
// xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx

				LastMsgLoaded = ThisMsgNum;
				MoreMessages = TRUE;
				}
			} while (MoreMessages);

		Cit86Messages.Close();

		if (Msg)
			{
			destroyCitWindow(Msg, FALSE);
			}
		}
	else
		{
		CitWindowsError(Parent, getmsg(78), FullPath);

		delete [] RoomData;
		dump_cfg_messages();
		discardData(CfgData);
		compactMemory(1);

		return;
		}

	delete [] RoomData;
	dump_cfg_messages();
	discardData(CfgData);
	compactMemory(1);
	}

static void CompileDictionary(CITWINDOW *Parent, const char *Source)
	{
	char Dest[80];

	strcpy(Dest, Source);

	if (strchr(Dest, '.'))
		{
		strcpy(strchr(Dest, '.'), getfsmsg(1));
		}
	else
		{
		strcat(Dest, getfsmsg(1));
		}

	FILE *in = fopen(Source, FO_R);

	if (in)
		{
		FILE *out;
		out = fopen(Dest, FO_WB);

		if (out)
			{
			CITWINDOW *Msg;
			char Letter = 0;
			char Word[128], LastWord[128];
			long Offsets[26];

			Msg = CitWindowsMsg(Parent, getfsmsg(2));

			LastWord[0] = 0;

			fseek(out, 26 * sizeof(long), SEEK_SET);

			WriteChar(0, NULL);

			long TotalWords = 0, CharsWritten = 0;

			while (fgets(Word, 128, in) != NULL)
				{
				TotalWords++;

				if (Word[strlen(Word) - 1] == '\n')
					{
					Word[strlen(Word) - 1] = 0;
					}

				strupr(Word);

				if (strcmp(Word, LastWord) <= 0)
					{
					if (Msg)
						{
						destroyCitWindow(Msg, FALSE);
						}

					CitWindowsError(Parent, getfsmsg(3), ltoac(TotalWords));
					fclose(in);
					fclose(out);
					unlink(Dest);
					return;
					}

				if (Word[0] != Letter)
					{
					Letter = Word[0];

					if (Msg)
						{
						destroyCitWindow(Msg, FALSE);
						}

					Msg = CitWindowsMsg(Parent, getfsmsg(5), Letter,
							ltoac(TotalWords - 1));

					Offsets[Letter - 'A'] = CharsWritten;

					WriteChar(0, out);
					CharsWritten++;
					}

				if (!WriteWord(Word, out))
					{
					if (Msg)
						{
						destroyCitWindow(Msg, FALSE);
						}

					CitWindowsError(Parent, getfsmsg(4));
					fclose(in);
					fclose(out);
					unlink(Dest);
					return;
					}

				CharsWritten += strlen(Word);
				strcpy(LastWord, Word);
				}

			WriteChar(0, out);
			WriteChar(0, out);

			fseek(out, 0, SEEK_SET);
			fwrite(Offsets, 26, sizeof(long), out);

			fclose(out);

			if (Msg)
				{
				destroyCitWindow(Msg, FALSE);
				}

			CitWindowsNote(Parent, getfsmsg(6),
					ltoac(TotalWords));
			}
		else
			{
			CitWindowsError(Parent, getfsmsg(7));
			}

		fclose(in);
		}
	else
		{
		CitWindowsError(Parent, getfsmsg(8));
		}
	}

static void fscControlHandler(CITWINDOW *wnd, long param)
	{
	CONTROLINFO *ci = (CONTROLINFO *) param;

	switch (ci->id)
		{
		case CTRL_COMPILEDICTSTR:
			{
			if (*((char *) (ci->ptr)))
				{
				CompileDictionary(wnd, (char *) (ci->ptr));
				}

			break;
			}

		case CTRL_CIT86PATHSTR:
			{
			if (*((char *) (ci->ptr)))
				{
				ImportCit86Data(wnd, (char *) (ci->ptr));
				}

			break;
			}
		}
	}

// From -O
static Bool fscKeyHandler1(CITWINDOW *wnd, int key)
	{
	switch (key)
		{
		case 1:
			{
			if (filexists(citfiles[C_CONFIG_CIT]))
				{
				CitWindowsNote(wnd, getfsmsg(16));
				}
			else
				{
				CitWindowsGetString(getfsmsg(12), 63, ns, wnd,
						CTRL_CIT86PATHSTR, NULL, FALSE);
				}

			break;
			}

		case 2:
			{
			KeepGoing = TRUE;
			return (FALSE);
			}

		case 3:
		case MK_ESC:
			{
			return (FALSE);
			}

		case 0:
			{
			CitWindowsGetString(getfsmsg(9), 63, getfsmsg(10), wnd,
					CTRL_COMPILEDICTSTR, NULL, FALSE);

			break;
			}
		}

	return (TRUE);
	}

// From Control+F6
static Bool fscKeyHandler2(CITWINDOW *wnd, int key)
	{
	switch (key)
		{
		case 1:
		case MK_ESC:
			{
			return (FALSE);
			}

		case 0:
			{
			CitWindowsGetString(getfsmsg(9), 63, getfsmsg(10), wnd,
					CTRL_COMPILEDICTSTR, NULL, FALSE);

			break;
			}
		}

	return (TRUE);
	}

Bool initFullScreenConfig(CITWINDOW *Parent)
	{
	if (read_fs_messages())
		{
		int Top, Left;

		if (Parent)
			{
			Top = Parent->extents.top + 2;
			Left = Parent->extents.left + 2;
			}
		else
			{
			Top = Left = 0;
			}

		CITWINDOW *w = MakeMenuWindow(Parent, Parent ?
				(const char **) ExtMessages[MSG_FSCONFIG].Data->next->next->aux :
				(const char **) ExtMessages[MSG_FSCONFIG].Data->next->aux,
				Top, Left, Parent ? fscKeyHandler2 : fscKeyHandler1,
				fscControlHandler, fscDump, getfsmsg(11));

		if (w)
			{
			return (TRUE);
			}
		else
			{
			dump_fs_messages();
			}
		}

	return (FALSE);
	}

Bool FullScreenConfig(void)
	{
	setscreen();

	if ((long) physScreen == 0xB0000000l)
		{
		// monochrome default colors...
		cfg.attr = 7;
		cfg.wattr = 0x70;
		cfg.cattr = 15;
		cfg.uttr = 1;
		}
	else
		{
		// color default colors
		cfg.attr = 15;
		cfg.wattr = 0x4e;
		cfg.cattr = 0x0e;
		cfg.uttr = 0x0d;
		}

	logo(TRUE);

	if (initFullScreenConfig(NULL))
		{
		// wait for them to finish
		while (allWindows)
			{
			// let the CitWindows handlers handle stuff
			kb_hit();
			}

		logo(FALSE);
		}
	else
		{
		cOutOfMemory(4);
		}

	return (KeepGoing);
	}
#endif
