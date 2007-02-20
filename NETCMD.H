// --------------------------------------------------------------------------
// Citadel: NetCmd.H
//
// Stuff for network commands


class NetCmdFileC: MessageStoreC
	{
	FILE	*CmdFile;

	virtual Bool SaveByte(char Byte);
	virtual Bool SaveString(const char *String);
	
	virtual Bool GetByte(char *Byte);

public:
	NetCmdFileC(FILE *File)
		{
		NetPacket = TRUE;
		CmdFile = File;
		}

	ReadMessageStatus LoadAll(Message *Msg)
		{
		return(MessageStoreC::LoadAll(Msg));
		}

	Bool Store(Message *Msg)
		{
		return (MessageStoreC::Store(Msg));
		}
	};


