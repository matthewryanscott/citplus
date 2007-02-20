// --------------------------------------------------------------------------
// Citadel: TelnetD.CPP
//
// Used by the Windows version for talking telnet

#ifdef WINCIT
#include <winsock.h>

#include "ctdl.h"
#pragma hdrstop

#include "telnetd.h"
#include "termwndw.h"
#include "extmsg.h"

void _USERENTRY ListenForTelnetConnection(void *);



// --------------------------------------------------------------------------
// This must only be called from TelnetPortC::getChar() or TelnetPortC::CheckInputReady().

Bool TelnetPortC::checkForCommand(void)
    {
    if (InRBuf)
        {
        // See if next character is a command start
        if (!InCommand)
            {
            if (RBuf[RBufIdx] == 255 && !let255through)
                {
                InCommand = 1;
                RBufIdx++;
                InRBuf--;
                }
            }

        // If we are in a command (either before this call or because of this
        // call), we get to buffer the command then try to act on it.
        while (InCommand && InRBuf)
            {
            if (InCommand > sizeof(CBuf))
                {
                // we have overflowed our command buffer. this is not good. just abort the command processing.
                InCommand = 0;
                }
            else
                {
                CBuf[InCommand - 1] = RBuf[RBufIdx++];
                InRBuf--;
                InCommand++;

                if (tryCommand())
                    {
                    return (TRUE);
                    }
                }
            }
        }

    return (FALSE);
    }


// --------------------------------------------------------------------------
// This must only be called from TelnetPortC::checkForCommand().

Bool TelnetPortC::tryCommand(void)
    {
    Bool RetVal = FALSE;

    switch (CBuf[0])
        {
        case 250:   // SB
            {
            if (CBuf[InCommand - 2] == 240)
                {
                switch (CBuf[1])
                    {
                    case 32:    // Terminal speed
                        {
                        if (CBuf[2] == 0)   // TERMINAL-SPEED IS
                            {
                            TSpeed = atol((char *) (CBuf + 3));
                            const char *p = strchr((char *) (CBuf + 3), ',');
                            if (p)
                                {
                                RSpeed = atol(p + 1);
                                }
                            }

                        break;
                        }
                    }

                InCommand = 0;
                RetVal = TRUE;
                }

            break;
            }

        case 244:   // IP
        case 245:   // AO
            {
            uchar Out[2];
            Out[0] = 255;   // IAC
            Out[1] = 242;   // data mark

            // We don't use SendData() here, because we're sending this out-of-band. It gets to go to the head of the
            // class. And I choose to ignore send() errors here; a dead socket will be found elsewhere.
            send(TS, (const char *)Out, 2, MSG_OOB);

            InCommand = 0;
            RetVal = TRUE;
            break;
            }

        case 241:   // nop
        case 242:   // data mark
        case 243:   // BRK
        case 246:   // AYT
        case 247:   // EC
        case 248:   // EL
        case 249:   // GA
            {
            InCommand = 0;
            RetVal = TRUE;
            break;
            }

        case 251:   // WILL
            {
            if (InCommand >= 3)
                {
                switch (CBuf[1])
                    {
                                // Supported stuff:
                    case 32:    // Terminal speed (rfc1079)
                    case 10:    // Force logout (rfc727)
                    case 0:     // Transmit binary (rfc856)
                    case 3:     // Supress GA (rfc858)
                        {
                        if (!Closed)
                            {
                            uchar Out[30];

                            if (!Enabled[CBuf[1]])
                                {
                                Out[0] = 255;   // IAC
                                Out[1] = 253;   // DO
                                Out[2] = CBuf[1];   // whatever it is WILLing

                                SendData((char *) Out, 3);

                                Enabled[CBuf[1]] = TRUE;
                                }

                            // For sending stuff after the DO response
                            switch (CBuf[1])
                                {
                                case 32:    // Terminal speed
                                    {
                                    Out[0] = 255;   // IAC
                                    Out[1] = 250;   // SB
                                    Out[2] = 32;    // Terminal speed
                                    Out[3] = 1;     // Send
                                    Out[4] = 255;   // IAC
                                    Out[5] = 240;   // SE

                                    SendData((char *) Out, 6);
                                    break;
                                    }
                                }
                            }

                        break;
                        }

                    default:    // If we don't support it, DON'T back
                        {
                        if (!Closed)
                            {
                            uchar Out[3];
                            Out[0] = 255;   // IAC
                            Out[1] = 254;   // DON'T
                            Out[2] = CBuf[1];   // whatever it is WILLing

                            SendData((char *) Out, 3);
                            }

                        break;
                        }
                    }

                InCommand = 0;
                RetVal = TRUE;
                }

            break;
            }

        case 252:   // WON'T
            {
            if (InCommand >= 3)
                {
                Enabled[CBuf[1]] = FALSE;
                InCommand = 0;
                RetVal = TRUE;
                }

            break;
            }

        case 253:   // DO
            {
            if (InCommand >= 3)
                {
                switch (CBuf[1])
                    {
                                // Supported DOs
                    case 10:    // Force logout (rfc727)
                    case 0:     // Transmit binary (rfc856)
                    case 1:     // Echo (rfc857)
                    case 3:     // Supress GA (rfc858)
                        {
                        if (!Closed)
                            {
                            uchar Out[30];

                            if (!Enabled[CBuf[1]])
                                {
                                Out[0] = 255;   // IAC
                                Out[1] = 251;   // WILL
                                Out[2] = CBuf[1];   // whatever it is DOing

                                SendData((char *) Out, 3);

                                Enabled[CBuf[1]] = TRUE;
                                }

                            // For sending stuff after the WILL response
                            switch (CBuf[1])
                                {
                                default:
                                    {
                                    break;
                                    }
                                }
                            }

                        break;
                        }

                    default:    // If we don't support it, WON'T back
                        {
                        if (!Closed)
                            {
                            uchar Out[3];
                            Out[0] = 255;   // IAC
                            Out[1] = 252;   // WON'T
                            Out[2] = CBuf[1];   // whatever it is DOing

                            SendData((char *) Out, 3);
                            }

                        break;
                        }
                    }

                InCommand = 0;
                RetVal = TRUE;
                }

            break;
            }

        case 254:   // DON'T
            {
            if (InCommand >= 3)
                {
                Enabled[CBuf[1]] = FALSE;
                InCommand = 0;
                RetVal = TRUE;
                }

            break;
            }

        case 255:
            let255through = TRUE;

        default:    // 255 (IAC) and the rest
            {
            InCommand = 0;

            InRBuf++;
            RBufIdx--;
            break;
            }
        }

    return (RetVal);
    }

void _USERENTRY TelnetR(void *A)
    {
    TelnetPortC *TP = (TelnetPortC *) A;

    while (TP->RThreadRunning)
        {
        WaitForSingleObject(TP->RMutex, INFINITE);

        if (!TP->InRBuf)
            {
            int RRet = recv(TP->TS, (char *) TP->RBuf, sizeof(TP->RBuf), 0);

            if (RRet > 0)
                {
                if (TP->InputWaiting)
                    {
                    SetEvent(TP->InputWaiting);
                    }

                TP->InRBuf = RRet;
                TP->RBufIdx = 0;
                }
            else if (!RRet)
                {
                TP->Closed = TRUE;
                }
            else
                {
                errorDisp("[telnet] recv(): %d, %d", RRet, WSAGetLastError());
                TP->Closed = TRUE;
                }
            }

        if (TP->Closed)
            {
            TP->RThreadRunning = FALSE;
            }

        ReleaseMutex(TP->RMutex);
        }
    }

// This makes sure we flush the Telnet output buffer at least once a second.
void _USERENTRY TelnetT(void *A)
    {
    TelnetPortC *TP = (TelnetPortC *) A;

    while (TP->TThreadRunning)
        {
        Sleep(1000);

        WaitForSingleObject(TP->TMutex, INFINITE);

        TP->FlushOutput();

        if (TP->Closed)
            {
            TP->TThreadRunning = FALSE;
            }

        ReleaseMutex(TP->TMutex);
        }
    }

void TelnetPortC::FlushOutput(void)
    {
    SendData(NULL, 0);
    }

void TelnetPortC::SendData(char *Data, int Len)
    {
    WaitForSingleObject(TMutex, INFINITE);

    // if our buffer cannot fit what we are about to add, or this is a call
    // from TelnetT, flush the buffer.
    if (!Len || (Len + InTBuf > TBufSize))
        {
        int Sent = 0;

        while (Sent < InTBuf)
            {
            int S = send(TS, TBuf + Sent, InTBuf - Sent, 0);
            if (S == SOCKET_ERROR)
                {
                InTBuf = 0;
                ReleaseMutex(TMutex);
                return;
                }

            Sent += S;
            }

        InTBuf = 0;
        }

    // If this won't fit into the buffer, send it directly
    if (Len > TBufSize)
        {
        int Sent = 0;

        while (Sent < Len)
            {
            int S = send(TS, Data + Sent, Len - Sent, 0);
            if (S == SOCKET_ERROR)
                {
                ReleaseMutex(TMutex);
                return;
                }

            Sent += S;
            }
        }
    // Else add it to the buffer
    else if (Len)
        {
        memcpy(TBuf + InTBuf, Data, Len);
        InTBuf += Len;
        }

    ReleaseMutex(TMutex);
    }

InputReadyE TelnetPortC::CheckInputReady(void)
    {
    // This is because rfc854 says that CR must be followed by NUL or LF
    // and we don't want bogus NULs coming in

//  if (InRBuf && LastInWasCR)
//      {
//      if (RBuf[RBufIdx] == 0)
//          {
//          RBufIdx++;
//          InRBuf--;
//          }
//
//      LastInWasCR = FALSE;
//      }

    if (InRBuf)
        {
        if (!getCharHasMutex)
            {
            WaitForSingleObject(RMutex, INFINITE);
            getCharHasMutex = TRUE;
            }

        while (checkForCommand());

        if (!InRBuf)
            {
            if (InputWaiting)
                {
                ResetEvent(InputWaiting);
                }

            getCharHasMutex = FALSE;
            ReleaseMutex(RMutex);
            }
        }

//  if (InRBuf &&
//          (LastInWasCR && ((RBuf[RBufIdx] == 0) ||
//          (RBuf[RBufIdx] == 10))))
//      {
//      return (IR_IGNORE);
//      }
//
//  else

        if (InRBuf && ((RBuf[RBufIdx] != 255) || let255through))
        {
        return (IR_READY);
        }

    return (IR_NONE);
    }

uchar TelnetPortC::getChar(void)
    {
    if (InRBuf)
        {
        if (!getCharHasMutex)
            {
            WaitForSingleObject(RMutex, INFINITE);
            getCharHasMutex = TRUE;
            }

        while (checkForCommand());

        uchar toRet = 0;

        if (InRBuf)
            {
            InRBuf--;
            toRet = RBuf[RBufIdx++];

//          if (toRet == '\r')
//              {
//              LastInWasCR = TRUE;
//              }

            if (toRet == 255)
                {
                let255through = FALSE;
                }
            }

        if (!InRBuf)
            {
            if (InputWaiting)
                {
                ResetEvent(InputWaiting);
                }

            getCharHasMutex = FALSE;
            ReleaseMutex(RMutex);
            }

        return (toRet);
        }

    return (0);
    }

void TelnetPortC::putChar(uchar Out)
    {
    if (!Closed)
        {
        // CR needs to be followed by LF or NUL according to rfc854
//      if (LastOutWasCR && Out != '\n')
//          {
//          char Poop = 0;
//          SendData(&Poop, 1);
//          }

//      LastOutWasCR = (Out == '\r');

        if (Out == 255)
            {
            uchar Out2[2];

            Out2[0] = Out2[1] = 255;

            SendData((char *) Out2, 2);
            }
        else
            {
            SendData((char *) &Out, 1);
            }
        }
    }


BOOL WINAPI _WNetGetHostAddress(LPCSTR lpszHost, int Port, LPSOCKADDR lpAddr)
    {
    LPHOSTENT lpHost;
    SOCKADDR_IN sin;

    lpHost = gethostbyname(lpszHost);
    if (lpHost != NULL)
        {
        sin.sin_family = PF_INET;
        CopyMemory(&sin.sin_addr, lpHost->h_addr_list[0], lpHost->h_length);

        sin.sin_port = (short) (((Port & 255) << 8) | (Port >> 8));

        ZeroMemory(sin.sin_zero, sizeof(sin.sin_zero));
        CopyMemory(lpAddr, &sin, sizeof(SOCKADDR));
        return (TRUE);
        }

    return (FALSE);
    }

static SOCKET TelnetListenSocket = -1;

void StartTelnetD(void)
    {
    WSADATA WSAData;
    if (WSAStartup(MAKEWORD(1, 1), &WSAData) == -1)
        {
        errorDisp("[td] Startup error: %hd", WSAGetLastError());
        return;
        }

    HANDLE hListener = (HANDLE) _beginthread(ListenForTelnetConnection, 4096, NULL);

    if (hListener != (HANDLE) -1)
        {
//      SetThreadPriority(hListener, THREAD_PRIORITY_NORMAL);
        SetThreadPriority(hListener, THREAD_PRIORITY_LOWEST);
        }
    }

void StopTelnetD(void)
    {
    if (TelnetListenSocket != -1)
        {
        closesocket(TelnetListenSocket);
        TelnetListenSocket = -1;
        }

    WSACleanup();
    }

Bool CreateListenSocket(SOCKET *LSock, int Port)
    {
    if (*LSock != -1)
        {
        closesocket(*LSock);
        }

    *LSock = socket(AF_INET, SOCK_STREAM, 0);

    if (*LSock == -1)
        {
        errorDisp("CreateListenSocket(): error: %hd", WSAGetLastError());
        return (FALSE);
        }

    SOCKADDR_IN sa;
//    char szMyName[256];
//    gethostname(szMyName, sizeof(szMyName));
//
//    if (!_WNetGetHostAddress(szMyName, Port, &sa))
//        {
//        errorDisp("[cls] wngha error");
//        closesocket(*LSock);
//        *LSock = -1;
//        return (FALSE);
//        }
    memset(&sa, 0, sizeof(sa));
    sa.sin_family = PF_INET;
    sa.sin_port = (short) (((Port & 255) << 8) | (Port >> 8));
    sa.sin_addr.S_un.S_addr = 0;

    if (bind(*LSock, (PSOCKADDR) &sa, sizeof(sa)))
        {
        errorDisp("[cls] bind error: %hd", WSAGetLastError());
        closesocket(*LSock);
        *LSock = -1;
        return (FALSE);
        }

    return (TRUE);
    }

void _USERENTRY ListenForTelnetConnection(void *)
    {

    if(!read_tr_messages())
        {
        errorDisp("%s- %s\n", getmsg(172), getmsg(59));
        return;
        }

    if (!CreateListenSocket(&TelnetListenSocket, cfg.TelnetPort))
        {
        dump_tr_messages();
        return;
        }

    if (listen(TelnetListenSocket, cfg.MaxTelnet))
        {
        errorDisp("[td] Listen error: %hd", WSAGetLastError());
        dump_tr_messages();
        return;
        }

    u_long NonBlock = FALSE;
    ioctlsocket(TelnetListenSocket, FIONBIO, &NonBlock);

    for (;;)
        {
        SOCKADDR_IN Client;
        int AddrLen = sizeof(Client);

        SOCKET NewSocket = accept(TelnetListenSocket, (SOCKADDR *) (&Client), &AddrLen);

        if (NewSocket == -1)
            {
            errorDisp("[td] Accept error: %hd", WSAGetLastError());
            }
        else
            {
            ioctlsocket(NewSocket, FIONBIO, &NonBlock);

            Bool KeepWindow = ViewingTerm;

            TurnOnTermWindow();

            if (ViewingTerm)
                {
                long NewSeq = ConnSeq.Next();
                label NewPort;

                sprintf(NewPort, "%d.%d.%d.%d", Client.sin_addr.S_un.S_un_b.s_b1, Client.sin_addr.S_un.S_un_b.s_b2,
                        Client.sin_addr.S_un.S_un_b.s_b3, Client.sin_addr.S_un.S_un_b.s_b4);

                trap(T_CARRIER, ns, gettrmsg(1), NewSeq, gettrmsg(4), gettrmsg(2), gettrmsg(8), NewPort);

                TelnetPortC *TP = new TelnetPortC(NewSocket);

                if (TP)
                    {
                    CopyStringToBuffer(TP->PortName, NewPort);

                    TermWindowC *TW = new TermWindowC(TP, NewSeq);

                    if (TW && TW->IsGood())
                        {
                        if (!KeepWindow)    // Not quite a good var name...
                            {
                            TermWindowCollection.SetFocus(TW);
                            }

                        TW->CarrierJustFound();

                        HANDLE t = (HANDLE) _beginthread(NewTermWindowThread, 1024 * 20, TW);

                        if (t != (HANDLE) -1)
                            {
                            TW->hThread = t;
                            }

                        KeepWindow = TRUE;
                        }
                    else
                        {
                        delete TW;
                        delete TP;
                        }
                    }

                if (!KeepWindow)
                    {
                    TurnOffTermWindow();
                    }
                }
            }
        }
    dump_tr_messages();
    }

TelnetPortC::~TelnetPortC(void)
    {
    Closed = TRUE;


    // Ask things to exit and give them half a second
    WSACancelBlockingCall();
    Sleep(500);

    // Then just close life on them
    closesocket(TS);

    // These will eventually see that Closed is TRUE and exit
    while (TThreadRunning || RThreadRunning)
        {
        Sleep(250);
        }

    // After they set ?ThreadRunning to FALSE, they need to release
    // their mutex... give them that chance.
    WaitForSingleObject(TMutex, INFINITE);
    WaitForSingleObject(RMutex, INFINITE);

    // Now our two helper threads are dead and gone, and we can finish
    // up cleaning up after ourselves.
    ReleaseMutex(RMutex);
    CloseHandle(RMutex);
    ReleaseMutex(TMutex);
    CloseHandle(TMutex);
    }


// Sends a TELNET NOP command to the user. Basically, to check to make sure that the network still lives
void TelnetPortC::Ping(void)
    {
    char PingBytes[2];

    PingBytes[0] = 255;   // IAC
    PingBytes[1] = 241;   // NOP

    SendData(PingBytes, sizeof(PingBytes));
    }

#endif
