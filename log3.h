// --------------------------------------------------------------------------
// Citadel: Log3.H
//
// This class defines the contents of LOG3.DAT for each user. It specifies
// which rooms a user is in. See LOG.H for an explanation of the LogBitBag
// class, which is used to store this information. There are three reasons we
// derive this class: to give it a default file name for storage on disk, so
// we can use nicely-named functions for access, and to implement a "smart"
// clear() function, which is found in LOGOVL.CPP.
//
// The bit assignment in this bit bag is more complex than most, so it
// deserves its own comments. These bits are used to implement hidden and
// by-invitation-only (BIO) rooms, as well as to kick users out of public-
// access rooms. It works like this:
//
// Regular rooms:	When the room is created, this bit is set for all current
//					users of the system. When a new user logs in for the
//					first time, this bit is set for all regular rooms.
//					Therefore, all users can, usually, get to all rooms on
//					the sytem. However, the .Invite commands can be used to
//					reset this bit for any user and any room. If this is so,
//					then there is no way for the user to get to this room.
//					(Without someone using the .Invite commands to set the
//					bit again.)
//
// Hidden rooms:	When the room is created, only the creator of the room
//					gets this bit set. Nobody else knows about the room, and
//					the bit is not set for any new callers, either. However,
//					users can get to the room by using the .Goto command and
//					giving the room's full name. This will set the bit for
//					the user, making it so he can get to the room just like
//					it was not a hidden room. The .Invite commands can also
//					be used to add to or remove them rom hidden rooms.
//
// BIO rooms:		These are very similar to hidden rooms, except that users
//					cannot get to them by simply knowing their name: the only
//					way to get access to a BIO room is to be invited with the
//					.Invite commands. (Thus the name "by-invitation-only.")
//
// There are two exceptions to these rules. First, if the user is the main
// sysop as defined by #SYSOP in CONFIG.CIT, he has access to all of the
// rooms on the system always. Second, if a user is sent mail in a room, then
// the user is allowed into the room to read the mail, which also gives him
// full access to the room in the future.

class LogEntry3 : public LogBitBag
	{
public:
#ifdef WINCIT
	void ResumeSyncSend(void);
	Bool Load(l_index Index);
#endif

	LogEntry3(r_slot NumRooms) : LogBitBag(NumRooms)
		{
		GAINEXCLUSIVEACCESS();
		ResetFileName();
		ADDSYNC();
		RELEASEEXCLUSIVEACCESS();
		}

	~LogEntry3(void)
		{
		REMOVESYNC();
		}

	void ResetFileName(void)
		{
		assert(this);
		GAINEXCLUSIVEACCESS();
		SetFileName(log3Dat);
		RELEASEEXCLUSIVEACCESS();
		}

	Bool IsInRoom(r_slot RoomSlot) const
		{
		return (BitTest(RoomSlot));
		}

	void SetInRoom(r_slot RoomSlot, Bool InRoom)
		{
		SYNCDATA(L3_SETINROOM, RoomSlot, InRoom);
		
		BitSet(RoomSlot, InRoom);
		}

	void Clear(void);
	};
