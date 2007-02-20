// --------------------------------------------------------------------------
// Citadel: Net6969.CPP
//
// Stuff for Net 6969

#ifdef WINCIT

#include "ctdl.h"
#pragma hdrstop

#include "net6969.h"
#include "termwndw.h"
#include "extmsg.h"

// From TelnetD.cpp
Bool CreateListenSocket(SOCKET *LSock, int Port);


static SOCKET Net6969ListenSocket = -1;
Net6969C Net6969;

Bool Net6969C::NetworkActive(l_index LogIndex)
    {
    Lock();

    Bool RetVal = FALSE;

    for (ActiveSessionsS *cur = ActiveSessions; cur && !RetVal; cur = (ActiveSessionsS *) getNextLL(cur))
        {
        if (cur->Session && cur->Session->GetLog() == LogIndex)
            {
            RetVal = TRUE;
            }
        }

    Unlock();

    return (RetVal);
    }

static Bool GetStringFromSocket(SOCKET Socket, char *Buffer, size_t BufferLength)
    {
    size_t BufOffset = 0;

    while (BufOffset < BufferLength - 1)
        {
        int RRet = recv(Socket, Buffer + BufOffset, 1, 0);

        if (RRet > 0)
            {
            Buffer[BufOffset + RRet] = 0;

            if (Buffer[BufOffset + RRet - 1] == LF)
                {
                Buffer[BufOffset + RRet - 1] = 0;
                return (TRUE);
                }

            BufOffset += RRet;
            }
        else
            {
            return (FALSE);
            }
        }

    return (TRUE);
    }

Net6969CommandsE Net6969C::GetCommand(const char *String) const
    {
    Net6969CommandsE Cmd;

    for (Cmd = (Net6969CommandsE) 0; Cmd < N6969_NUM; Cmd = (Net6969CommandsE)(Cmd +  1))
        {
        if (SameString(String, CommandText(Cmd)))
            {
            break;
            }
        }

    return (Cmd);
    }


void Net6969SessionC::DoNetReceive(void)
    {
    SetReceive(TRUE);

    if (Net6969.InitializeDiscardable())
        {
        char Wow[512];
        while (!IsKilled() && GetString(Wow, sizeof(Wow)))
            {
            Net6969CommandsE Cmd = Net6969.GetCommand(Wow);

            switch (Cmd)
                {
                case N6969_NETID:
                    {
                    // get NetID, add to list
                    if (GetString(Wow, sizeof(Wow)))
                        {
                        if (!IsNetIDCorrupted(Wow))
                            {
                            AddRequestedNetID(Wow);
                            }
                        }

                    break;
                    }

                case N6969_MESSAGE:
                    {
                    SaveN6969Mess(NULL);   // seems as good a time as any.
                    STARTMESS = FALSE;
                    Message *Msg = new Message;

                    if (Msg)
                        {
                        if (MsgIO->LoadAll(Msg) == RMS_OK)
                            {
errorDisp("Got message %s (room %s)", Msg->GetSourceID(), Msg->GetCreationRoom());
                            const r_slot Room = IdExists(Msg->GetCreationRoom(), TRUE);

                            if (Room == CERROR)
                                {
errorDisp("Room %s not found!", Msg->GetCreationRoom());
                                }
                            else if (Node->CanAccessRoom(Room))
                                {
errorDisp("Incorporating message from %s", Msg->GetAuthor());

                                // Map the NetID to the local name
                                label RoomName;
                                RoomTab[Room].GetName(RoomName, sizeof(RoomName));
                                Msg->SetCreationRoom(RoomName);

                                MessageDat.IncorporateMessage(Msg, Room, Node, cfg.temppath, Log, NodesCit);
                                }
                            else
                                {
errorDisp("Node cannot access room %d", Room);
                                }
                            }
                        else
                            {
                            errorDisp("bad MsgIO->LoadAll()");
                            }

                        delete Msg;
                        }
                    break;
                    }

				case N6969_CHATALL:
					{
					GetString(Wow, sizeof(Wow));	// Ignore the size flag
					GetString(Wow, sizeof(Wow));	// This one is the chat text!

					TermWindowCollection.SystemEvent(SE_CHATALL, 0, NULL, FALSE, Wow);

					break;
					}

                default:
                    {
                    // add: better error?
                    errorDisp(Wow);

//  I'm not sure what to do here, maybe just leave it commented out for now until someone knows what to do
//  The following appears to skip over some valid incoming stuff.
//					if (GetString(Wow, sizeof(Wow)))
//						{
//						SkipBytes(atol(Wow));
//						}

                    break;
                    }
                }
            }
        }
    else
        {
        // add: some sort of error
        }

    // In case we're the first to notice a disconnection, tell the sending thread to go away
    Kill();

    // Let the sending thread know that we've gone away
    SetReceive(FALSE);
    }

void _USERENTRY Net6969Receive(void *S)
    {
    Net6969SessionC *Us = (Net6969SessionC *) S;
    Us->DoNetReceive();
    }

// This does not call FlushOutBuffer(), so it would be a good idea to call it yourself after calling this
void Net6969SessionC::RequestNetID(r_slot Room, const char *NewNetID)
    {
    if (Node && Node->CanAccessRoom(Room) && !Node->IsRoomExcluded(Room))
        {
        Lock();
        strList *cur = (strList *) addLL((void **) &NetIDsToRequest, sizeof(strList) + strlen(NewNetID));

        if (cur)
            {
            strcpy(cur->string, NewNetID);
            }

        Unlock();
        }
    }

Bool Net6969SessionC::GetNetIDToRequest(char *NetID, size_t Length)
    {
    Lock();

    Bool RetVal = FALSE;

    if (NetIDsToRequest)
        {
        RetVal = TRUE;
        CopyString2Buffer(NetID, NetIDsToRequest->string, Length);
        deleteLLNode((void **) &NetIDsToRequest, 1);
        }

    Unlock();

    return (RetVal);
    }

void Net6969SessionC::DoNetSend(void)
    {
    if (Node->Load(Log))
        {
        // add: some sort of timeout

        // Start the receiver...
        if ((HANDLE) _beginthread(Net6969Receive, 8 * 1024, this) == (HANDLE) -1)
            {
            // No start receiver? uh-oh!
            Kill();

            // add: send error
            }
        else
            {
            // Go through each room, requesting NetIDs from remote system as appropriate
            for (r_slot roomNo = 0; roomNo < cfg.maxrooms; roomNo++)
                {
                label NetID;

                if (RoomTab[roomNo].IsShared() && *RoomTab[roomNo].GetNetID(NetID, sizeof(NetID)))
                    {
                    RequestNetID(roomNo, NetID);
                    }
                }

            FlushOutBuffer();
            }

        // add: instead of polled, some sort of WaitFor flag?
        while (!IsKilled())
            {
            Bool SomethingSent = FALSE;
            label NetID;

            while (GetNetIDToSend(NetID, sizeof(NetID)))
                {
                r_slot roomNo;
                if ((roomNo = IdExists(NetID, TRUE)) != CERROR)
                    {
                    if (Node->CanAccessRoom(roomNo))
                        {
                        RoomTab[roomNo].GetNetID(NetID, sizeof(NetID));

                        const m_index lowLim = Node->GetRoomNewPointer(roomNo) + 1;
                        const m_index highLim = MessageDat.NewestMessage();

                        for (m_index ID = lowLim; ID <= highLim; ID++)
                            {
                            Message *Msg = ShouldNetworkMessage(ID, roomNo, Node);

                            if (Msg)
                                {
                                Msg->SetCreationRoom(NetID);

                                if (MsgIO->Store(Msg))
                                    {
                                    Node->SetRoomNewPointer(roomNo, ID);
                                    }

                                delete Msg;
                                }
                            }

                        SomethingSent = TRUE;
                        }
                    }
                }

            while (GetNetIDToRequest(NetID, sizeof(NetID)))
                {
                char Wow[128];

                // add: NETID from ctdldata.def(24,1,1)
                sprintf(Wow, "%s\n%s\n", "NETID", NetID);
//                sprintf(Wow, getmsg(readData(24, 1, 1), 4), getmsg(readData(24, 1, 1), 1), NetID);
                SendString(Wow);
                SomethingSent = TRUE;
                }

			while (PendingPackets)
				{
				Lock();

				Net6969PacketListS *ThisPacket = PendingPackets;
				PendingPackets = PendingPackets->Next;
				
				Unlock();

				if (ThisPacket)
					{
					if (ThisPacket->Packet)
						{
						if (ThisPacket->Packet->IsValid())
							{
							SendString(ThisPacket->Packet->GetCommand(), TRUE);
		
							label PacketLengthString;
							sprintf(PacketLengthString, "%d", ThisPacket->Packet->GetDataLength());
							SendString(PacketLengthString, TRUE);
		
							SendData(ThisPacket->Packet->GetData(), ThisPacket->Packet->GetDataLength());

							SomethingSent = TRUE;
							}

						delete ThisPacket->Packet;
						}

					delete ThisPacket;
					}
				}

            if (SomethingSent)
                {
                FlushOutBuffer();
                }

            Sleep(1000);
            }
        }
    // else: some sort of error to caller?

    // add: wait for pending network operations to cease somehow

    // Wait for the receiver to notice our death
    while (IsReceive())
        {
        Sleep(1000);
        }

    // log the node out!!!!!!! First, save userlog entry for node
    // add: update lastcalltime
    Node->Save(GetLog(), LOBBY);

    // now, remove from LoginList
    LoginList.Remove(GetLog());

    // and close the Socket
    closesocket(GetSocket());

    // Then, no longer an active network session (if successful, RemoveSession deletes Us, as well)
    if (!Net6969.RemoveSession(this))
        {
        delete this;
        }

    }

void _USERENTRY DoNet6969(void *S)
    {
    Net6969SessionC *Us = (Net6969SessionC *) S;
    Us->DoNetSend();
    }


Bool Net6969C::Network(SOCKET Socket, l_index LogIndex)
    {
    Lock();

    Bool RetVal = FALSE;

    Net6969SessionC *New = AddSession(Socket, LogIndex);

    if (New)
        {
        send(Socket, "69", 2, 0);   // this is a bit of an issue: would have been send already even if _beginthread
                                    // fails. so the calling node gets a 69, then a 96. oops. oh well.
                                    // add: send error in that case!
        RetVal = ((HANDLE) _beginthread(DoNet6969, 8 * 1024, (void *) New) != (HANDLE) -1);
        }

    Unlock();

    return (RetVal);
    }

Bool Net6969C::Network(NodesCitC *NodeInfo, l_index LogIndex)
    {
    Bool RetVal = FALSE;

    Lock();

    if (!NetworkActive(LogIndex))
        {
        PHOSTENT phe;
        SOCKADDR_IN dest_sin;  /* DESTination Socket INternet */
        SOCKET sock;

        char ipaddress[128];
        CopyStringToBuffer(ipaddress, NodeInfo->GetIpAddress());
        int ipport = NodeInfo->GetIpPort();

        /////////////////////////////////////////////////////////////////////////////

        sock = socket(AF_INET, SOCK_STREAM, 0);
        if (sock == INVALID_SOCKET)
            {
            errorDisp("N6969: socket() failed");
            }
        else
            {
            dest_sin.sin_family = AF_INET;

            phe = gethostbyname(ipaddress);
            if (phe == NULL)
                {
                errorDisp("N6969: gethostbyname() failed.");
                }
            else
                {
                memcpy((char FAR *)&(dest_sin.sin_addr), phe->h_addr, phe->h_length);

                dest_sin.sin_port = htons(ipport);   /* Convert to network ordering */

                // add: REDIAL
                if (connect(sock, (PSOCKADDR) &dest_sin, sizeof(dest_sin)) < 0)
                    {
                    errorDisp("N6969: connect() failed");
                    }
                else
                    {
                    errorDisp("N6969: connect() worked!");

                    //////////////////////////////////////////////////////////////////////////////////////

                    // add: DIAL_TIMEOUT
                    char Wow[128];
                    if (GetStringFromSocket(sock, Wow, 3))
                        {
                        if (SameString(Wow, "69"))
                            {
                            // add: it would be good here to parse_it the login macro
                            send(sock, NodeInfo->GetLoginMacro(), strlen(NodeInfo->GetLoginMacro()), 0);
                            send(sock, bn, 1, 0);

                            if (GetStringFromSocket(sock, Wow, 3))
                                {
                                if (SameString(Wow, "69"))
                                    {
                                    Net6969SessionC *New = AddSession(sock, LogIndex);

                                    if (New)
                                        {
                                        RetVal = ((HANDLE) _beginthread(DoNet6969, 8 * 1024, (void *) New) != (HANDLE) -1);
                                        }
                                    }
                                else
                                    {
                                    GetStringFromSocket(sock, Wow, sizeof(Wow));
                                    trap(T_NETWORK, pcts, Wow);
                                    }
                                }
                            }
                        }
                    }
                }

            if (!RetVal)
                {
                closesocket(sock);
                }
            }
        }

    Unlock();

    return (RetVal);
    }

void Net6969C::KillAllSessions(void)
    {
    Lock();

    for (ActiveSessionsS *cur = ActiveSessions; cur; cur = (ActiveSessionsS *) getNextLL(cur))
        {
        if (cur->Session)
            {
            cur->Session->Kill();
            }
        }

    Unlock();
    }

void Net6969LoginError(SOCKET Socket, char *Reason)
    {
    send(Socket, "96", 2, 0);
    send(Socket, Reason, strlen(Reason), 0);
    send(Socket, bn, 1, 0);
    }

void _USERENTRY ListenForNet6969Connection(void *)
    {
    if (!CreateListenSocket(&Net6969ListenSocket, cfg.Net6969Port))
        {
        return;
        }

    if (listen(Net6969ListenSocket, 10))
        {
        errorDisp("[lfn6c] Listen error: %hd", WSAGetLastError());
        return;
        }

    u_long NonBlock = FALSE;
    ioctlsocket(Net6969ListenSocket, FIONBIO, &NonBlock);

    for (;;)
        {
        SOCKADDR_IN Client;
        int AddrLen = sizeof(Client);

        SOCKET NewSocket = accept(Net6969ListenSocket, (SOCKADDR *) (&Client), &AddrLen);

        if (NewSocket == -1)
            {
            errorDisp("[lfn6c] Accept error: %hd", WSAGetLastError());
            }
        else
            {
            Bool DontClose = FALSE;

            ioctlsocket(NewSocket, FIONBIO, &NonBlock);

            send(NewSocket, "69", 2, 0);

            char Authentication[LABELSIZE * 2 + 3];
            if (GetStringFromSocket(NewSocket, Authentication, sizeof(Authentication)))
                {
                char Init[LABELSIZE * 2 + 3];
                char PW[LABELSIZE * 2 + 3];
                l_slot *LogOrder = new l_slot[cfg.MAXLOGTAB];

                if (LogOrder)
                    {
                    for (l_slot i = 0; i < cfg.MAXLOGTAB; i++)
                        {
                        LogOrder[i] = i;
                        }

                    normalizepw(Authentication, Init, PW);
                    const l_slot FoundSlot = pwslot(Init, PW, LogOrder);

                    if (FoundSlot != CERROR)
                        {
                        if (LogTab[FoundSlot].IsNode())
                            {
                            if (LoginList.AddUnique(FoundSlot))
                                {
                                if (Net6969.Network(NewSocket, FoundSlot))
                                    {
                                    DontClose = TRUE;
                                    }
                                else
                                    {
                                    Net6969LoginError(NewSocket, "Net6969.Network() failed");
                                    }
                                }
                            else
                                {
                                if (LoginList.IsLoggedIn(FoundSlot))
                                    {
                                    Net6969LoginError(NewSocket, "Already logged in");
                                    }
                                else
                                    {
                                    Net6969LoginError(NewSocket, "System Busy; Try again later");
                                    }
                                }
                            }
                        else
                            {
                            Net6969LoginError(NewSocket, "Not node account");
                            }
                        }
                    else
                        {
                        Net6969LoginError(NewSocket, "Authentication failed");
                        }
                    }
                else
                    {
                    Net6969LoginError(NewSocket, "Could not create LogOrder");
                    }
                }

            if (!DontClose)
                {
                closesocket(NewSocket);
                }
            }
        }
    }

void Net6969C::StartListener(void)
    {
    if (cfg.Net6969Port)
        {
        WSADATA WSAData;
        if (WSAStartup(MAKEWORD(1, 1), &WSAData) == -1)
            {
            errorDisp("[n6969] Startup error: %hd", WSAGetLastError());
            return;
            }

        HANDLE hListener = (HANDLE) _beginthread(ListenForNet6969Connection, 4096, NULL);

        if (hListener != (HANDLE) -1)
            {
    //      SetThreadPriority(hListener, THREAD_PRIORITY_NORMAL);
            SetThreadPriority(hListener, THREAD_PRIORITY_LOWEST);
            }
        }
    }

Net6969SessionC *Net6969C::AddSession(SOCKET NewSocket, l_index NewLog)
    {
    Lock();

    ActiveSessionsS *New = (ActiveSessionsS *) addLL((void **) &ActiveSessions, sizeof(ActiveSessionsS));

    if (New)
        {
        New->Session = new Net6969SessionC(NewSocket, NewLog);

        if (!(New->Session && New->Session->IsGood()))
            {
            RemoveSession(New);
            New = NULL;
            }
        }

    Unlock();

    return (New ? New->Session : NULL);
    }

Bool Net6969C::RemoveSession(Net6969SessionC *S)
    {
    Lock();

    ActiveSessionsS *cur;
    int Counter;

    for (cur = ActiveSessions, Counter = 1; cur; cur = (ActiveSessionsS *) getNextLL(cur), Counter++)
        {
        if (cur->Session == S)
            {
            delete cur->Session;
            deleteLLNode((void **) &ActiveSessions, Counter);
            break;
            }
        }

    Unlock();

    return (cur != NULL);
    }

Bool Net6969C::RemoveSession(ActiveSessionsS *S)
    {
    Lock();

    ActiveSessionsS *cur;
    int Counter;

    for (cur = ActiveSessions, Counter = 1; cur; cur = (ActiveSessionsS *) getNextLL(cur), Counter++)
        {
        if (cur == S)
            {
            delete cur->Session;
            deleteLLNode((void **) &ActiveSessions, Counter);
            break;
            }
        }

    Unlock();

    return (cur != NULL);
    }

Bool Net6969C::InitializeDiscardable(void)
    {
    Lock();

    if (!DData)
        {
        DData = readData(24);
        }

    Unlock();

    return (!!DData);
    }

Net6969C::~Net6969C()
    {
    KillAllSessions();

    // KillAllSession() skips missing sessions -- this gets those
    Lock();

    Bool Found;

    do
        {
        Found = FALSE;

        ActiveSessionsS *cur;
        int Counter;

        for (cur = ActiveSessions, Counter = 1; cur; cur = (ActiveSessionsS *) getNextLL(cur), Counter++)
            {
            if (!cur->Session)
                {
                Found = TRUE;
                deleteLLNode((void **) &ActiveSessions, Counter);
                break;
                }
            }

        } while (Found == TRUE);

    Unlock();

    // KillAllSessions() just sets a flag. The sessions are then expected to remove themselves after cleaning up.
    // This checks once a second until they're done.
    while (ActiveSessions)
        {
        Sleep(1000);
        }

    if (OurMutex != INVALID_HANDLE_VALUE)
        {
        CloseHandle(OurMutex);
        }

    discardData(DData);
    }

void Net6969SessionC::AddRequestedNetID(const char *NewNetID)
    {
    Lock();

    // first, see if it's already in list...
    netIdList *cur;
    for (cur = RequestedNetIDs; cur; cur = (netIdList *) getNextLL(cur))
        {
        if (SameString(cur->NetID, NewNetID))
            {
            break;
            }
        }

    // if not, add it!
    if (!cur)
        {
        cur = (netIdList *) addLL((void **) &RequestedNetIDs, sizeof(netIdList) + strlen(NewNetID));
        cur->CheckSend = TRUE;
        strcpy(cur->NetID, NewNetID);

        // and see if we should create the room
        const r_slot roomNo = IdExists(NewNetID, FALSE);

        if (roomNo == CERROR)
            {
            if (NodesCit->IsAutoRoom() && !IsNetIDCorrupted(NewNetID) && !IsNetIDInNetIDCit(NewNetID))
                {
                label Name;
                Node->GetName(Name, sizeof(Name));

                if (AutoRoom(NewNetID, Name, NodesCit->GetAutoGroup(), NodesCit->GetAutoHall()))
                    {
                    if (NodesCit->IsVerbose(NCV_ROOMCREATED))
                        {
                        if(!read_net_messages())
                            {
                            errorDisp(getmsg(172));
                            return;
                            }

                        net6969_error(STARTMESS, Name, getnetmsg(107), NewNetID);
                        STARTMESS = TRUE;

                        dump_net_messages();
                        }
                    }
                else
                    {
                    if (NodesCit->IsVerbose(NCV_ROOMNOTCREATED))
                        {
                        if(!read_net_messages())
                            {
                            errorDisp(getmsg(172));
                            return;
                            }

                        net6969_error(STARTMESS, Name, getnetmsg(108), NewNetID);
                        STARTMESS = TRUE;

                        dump_net_messages();
                        }
                    }
                }
            }
        }

    Unlock();
    }

void Net6969SessionC::NewMessage(const char *NetID)
    {
    Lock();

    // look for it in the list; mark as need to send if found
    for (netIdList *cur = RequestedNetIDs; cur; cur = (netIdList *) getNextLL(cur))
        {
        if (SameString(cur->NetID, NetID))
            {
            cur->CheckSend = TRUE;
            break;
            }
        }

    Unlock();
    }

Bool Net6969SessionC::GetNetIDToSend(char *NetID, size_t Length)
    {
    Lock();

    netIdList *cur;
    for (cur = RequestedNetIDs; cur; cur = (netIdList *) getNextLL(cur))
        {
        if (cur->CheckSend)
            {
            CopyString2Buffer(NetID, cur->NetID, Length);
            cur->CheckSend = FALSE;
            break;
            }
        }

    Unlock();

    return (cur != NULL);
    }

Net6969SessionC::~Net6969SessionC()
    {
    Lock();

    FlushOutBuffer();
    delete [] InBuffer;
    delete [] OutBuffer;
    delete MsgIO;

    disposeLL((void **) &RequestedNetIDs);

    if (OurMutex != INVALID_HANDLE_VALUE)
        {
        CloseHandle(OurMutex);
        }

    delete Node;
    freeNode (&NodesCit);
    }

Bool Net6969SessionC::FlushOutBuffer(void)
    {
    Lock();

    if (OutBuffer)
        {
        int Sent = 0;

        while (Sent < OutBufferUsed)
            {
            int S = send(Socket, OutBuffer + Sent, OutBufferUsed - Sent, 0);
            if (S == SOCKET_ERROR)
                {
                OutBufferUsed = 0;
                return (FALSE);
                }

            Sent += S;
            }

        OutBufferUsed = 0;
        }

    Unlock();

    return (TRUE);
    }

Bool Net6969SessionC::SendByte(char Byte)
    {
    if (OutBuffer)
        {
        if (OutBufferUsed == Net6969MessageIOOutBufferSize)
            {
            // All out of buffer - send it, then start building a new one
            if (!FlushOutBuffer())
                {
                return (FALSE);
                }
            }

        OutBuffer[OutBufferUsed++] = Byte;
        }
    else
        {
        // send it unbuffered (ew!)
        if (send(Socket, &Byte, 1, 0) == SOCKET_ERROR)
            {
            return (FALSE);
            }
        }

    return (TRUE);
    }

Bool Net6969SessionC::SendString(const char *String, Bool WithNul)
    {
	return (SendData(String, strlen(String) + (WithNul ? 1 : 0)));
    }

Bool Net6969SessionC::SendData(const void *Data, size_t SendLength)
    {
    if (OutBuffer)
        {
        if (OutBufferUsed + SendLength > Net6969MessageIOOutBufferSize)
            {
            if (!FlushOutBuffer())
                {
                return (FALSE);
                }
            }

        // Yes, this is different than the last if(): FlushBuffer() sets OutBufferUsed to 0
        if (OutBufferUsed + SendLength > Net6969MessageIOOutBufferSize)
            {
            // not enough room to send it buffered -- send it unbuffered!
            size_t Sent = 0;

            while (Sent < SendLength)
                {
                size_t S = send(Socket, ((char *) Data) + Sent, SendLength - Sent, 0);
                if (S == SOCKET_ERROR)
                    {
                    return (FALSE);
                    }

                Sent += S;
                }
            }
        else
            {
            memcpy(OutBuffer + OutBufferUsed, Data, SendLength);
            OutBufferUsed += SendLength;
            }
        }
    else
        {
        // send it unbuffered (ew!)
        size_t Sent = 0;

        while (Sent < SendLength)
            {
            size_t S = send(Socket, ((char *) Data) + Sent, SendLength - Sent, 0);
            if (S == SOCKET_ERROR)
                {
                return (FALSE);
                }

            Sent += S;
            }
        }

    return (TRUE);
    }

Bool Net6969SessionC::GetByte(char *Byte)
    {
    if (InBuffer)
        {
        while (!InBufferUsed)
            {
            int RRet = recv(Socket, InBuffer, Net6969MessageIOInBufferSize, 0);

            if (RRet > 0)
                {
                InBufferUsed = RRet;
                InBufferPos = 0;
                }
            else
                {
                return (FALSE);
                }
            }

        *Byte = InBuffer[InBufferPos++];

        if (InBufferPos == InBufferUsed)
            {
            InBufferUsed = 0;
            InBufferPos = 0;
            }
        }
    else
        {
        int RRet = recv(Socket, Byte, 1, 0);

        if (RRet <= 0)
            {
            return (FALSE);
            }
        }

    return (TRUE);
    }

// Stops at LF or Nul from socket; replaces LF with Nul
Bool Net6969SessionC::GetString(char *Buffer, size_t BufferSize)
    {
    size_t Got = 0;

    while (Got < BufferSize - 1)
        {
        if (GetByte(Buffer + Got))
            {
            if (Buffer[Got] == LF)
                {
                Buffer[Got] = 0;
                }

            if (Buffer[Got] == 0)
                {
                return (TRUE);
                }

            Buffer[++Got] = 0;
            }
        else
            {
            return (FALSE);
            }
        }

    return (TRUE);
    }

void Net6969SessionC::SkipBytes(long ToSkip)
	{
	while (ToSkip--)
		{
		char c;

		if (!GetByte(&c))
			{
			return;
			}
		}
	}

Bool Net6969MessageIOC::Store(Message *Msg)
    {
    assert(N);

    // add: use ctdldata.def for MESSAGE
//    InitializeDiscardable();
//    N->SendString(getmsg(DData, 2));
    N->SendString("MESSAGE\n");
    return (MessageStoreC::Store(Msg));
    }

// We can assume N is good for these three, because Net6969MessageIOC::Store() has already checked it
Bool Net6969MessageIOC::SaveByte(char Byte)
    {
    return (N->SendByte(Byte));
    }

Bool Net6969MessageIOC::SaveString(const char *String)
    {
    return (N->SendString(String, TRUE));
    }

Bool Net6969MessageIOC::GetByte(char *Byte)
    {
    return (N->GetByte(Byte));
    }

void Net6969C::Propogate(const Message *Msg)
    {
    Lock();

    label NetID;
    RoomTab[Msg->GetRoomNumber()].GetNetID(NetID, sizeof(NetID));

    for (ActiveSessionsS *cur = ActiveSessions; cur; cur = (ActiveSessionsS *) getNextLL(cur))
        {
        if (cur->Session)
            {
            cur->Session->NewMessage(NetID);
            }
        }

    Unlock();
    }


void Net6969C::Propogate(r_slot RoomNum, const RoomC *Room)
    {
    label NetID;

    if (Room->IsInuse() && Room->IsShared() && *Room->GetNetID(NetID, sizeof(NetID)))
        {
        Lock();

        for (ActiveSessionsS *cur = ActiveSessions; cur; cur = (ActiveSessionsS *) getNextLL(cur))
            {
            if (cur->Session)
                {
                cur->Session->RequestNetID(RoomNum, NetID);
                }
            }

        Unlock();
        }
    }


void Net6969C::Debug(TermWindowC *TW)
    {
    Lock();

    for (ActiveSessionsS *cur = ActiveSessions; cur; cur = (ActiveSessionsS *) getNextLL(cur))
        {
        if (cur->Session)
            {
            cur->Session->Debug(TW);
            }
        }

    Unlock();
    }

void Net6969SessionC::Debug(TermWindowC *TW)
    {
    Lock();

    label Buf;

    TW->CRmPrintfCR("3Network connection:0 %s0", LogTab[Log].GetName(Buf, sizeof(Buf)));
    TW->mPrintfCR("3Requested:0");
    TW->prtList(LIST_START);

    for (netIdList *cur = RequestedNetIDs; cur; cur = (netIdList *) getNextLL(cur))
        {
        TW->prtList(cur->NetID);
        }

    TW->prtList(LIST_END);

    Unlock();
    }

void Net6969C::ChatAll(const char *ChatText)
	{
	Net6969PacketC ThePacket("CHATALL", strlen(ChatText) + 1, ChatText);

	Lock();

	for (ActiveSessionsS *cur = ActiveSessions; cur; cur = (ActiveSessionsS *) getNextLL(cur))
		{
		if (cur->Session && cur->Session->IsChat(NCC_ALL))
			{
			cur->Session->AddPacket(&ThePacket);
			}
		}

	Unlock();
	}

Bool Net6969SessionC::AddPacket(Net6969PacketC *ToAdd)
	{
	Bool ToRet = FALSE;

	if (ToAdd && ToAdd->IsValid())
		{
		Lock();

        Net6969PacketListS *cur = (Net6969PacketListS *) addLL((void **) &PendingPackets, sizeof(Net6969PacketListS));

		if (cur)
			{
			cur->Packet = new Net6969PacketC(ToAdd->GetCommand(), ToAdd->GetDataLength(), ToAdd->GetData());

			ToRet = (cur->Packet && cur->Packet->IsValid());
			}

		Unlock();
		}

	return (ToRet);
	}

void Net6969SessionC::net6969_error(Bool errorflag, const char *node, const char *fmt, ...)
	{
	char buff[256];
	va_list ap;

	va_start(ap, fmt);
	vsprintf(buff, fmt, ap);
	va_end(ap);

    if(!read_net_messages())
        {
        errorDisp(getmsg(172));
        return;
        }

	if (!errorflag)
		{
        natPrintf(getnetmsg(144), getnetmsg(55), node, bn);
		}

    natPrintf(getnetmsg(145), buff, bn);

    dump_net_messages();
	}

void cdecl Net6969SessionC::natPrintf(const char *fmt, ... )
	{
	// no message in progress? make one!
    if (N6969Msg == NULL)
		{
        N6969Msg = new Message;
		}

    if (N6969Msg)
		{
		va_list ap;
		char Buffer[128];

		va_start(ap, fmt);
		vsprintf(Buffer, fmt, ap);
		va_end(ap);

        N6969Msg->AppendText(Buffer);
		}
	}

void Net6969SessionC::SaveN6969Mess(const char *group, r_slot RoomNumber)
	{
    if (N6969Msg)
		{
        if (strlen(N6969Msg->GetText()) > 10)
			{
            N6969Msg->SetRoomNumber(RoomNumber);

			if (group)
				{
                N6969Msg->SetGroup(group);
				}

            systemMessage(N6969Msg);
			}

        delete N6969Msg;
        N6969Msg = NULL;
		}
	}
#endif
