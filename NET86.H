// --------------------------------------------------------------------------
// Citadel: Net86.H
//
// Stuff for Citadel-86 style networking


class Net86PacketC: MessageStoreC
	{
	FILE *NetFile;
	label OtherNode;

	virtual Bool SaveByte(char Byte);
	virtual Bool SaveString(const char *String);

	virtual Bool GetByte(char *Byte);

public:
	Net86PacketC(FILE *File, const char *NewOtherNode)
		{
		NetPacket = TRUE;
		NetFile = File;
		CopyStringToBuffer(OtherNode, NewOtherNode);
		}

	ReadMessageStatus LoadAll(Message *Msg);
	Bool Store(Message *Msg);
	};
