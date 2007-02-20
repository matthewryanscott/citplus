// --------------------------------------------------------------------------
// TelnetD.H
//
// How to talk to the telnet talker

void _USERENTRY TelnetR(void *A);
void _USERENTRY TelnetT(void *A);

class TelnetPortC: public CommunicationPortC
    {
// andy debug (x2)
protected:
friend void TermWindowC::doWow(Bool);
    SOCKET TS;
    Bool SocketValid;

    Bool Closed;

    uchar RBuf[16384];
    int RBufIdx, InRBuf;

    uchar CBuf[50];
    int InCommand;

    Bool Enabled[256];  // Which commands are enabled.

    long TSpeed, RSpeed;

    Bool LastInWasCR, LastOutWasCR;

    Bool let255through;

    Bool getCharHasMutex;
    HANDLE RMutex, TMutex;
    HANDLE RThread, TThread;
    Bool RThreadRunning, TThreadRunning;

    friend void _USERENTRY TelnetR(void *A);
    friend void _USERENTRY TelnetT(void *A);

    void SendData(char *ToSend, int Len);
    char TBuf[256];
    int TBufSize, InTBuf;

    virtual uchar getChar(void);
    virtual void putChar(uchar Out);

    Bool tryCommand(void);
    Bool checkForCommand(void);

public:
    virtual void FlushOutput(void);
    virtual InputReadyE CheckInputReady(void);
	virtual void Ping(void);

    TelnetPortC(SOCKET S) : CommunicationPortC(CT_TELNET)
        {
        TS = S;
        SocketValid = TRUE;
        InRBuf = InTBuf = 0;
        RBufIdx = 0;
        LastInWasCR = LastOutWasCR = Closed = getCharHasMutex = let255through = FALSE;
        TThreadRunning = RThreadRunning = TRUE;
        TSpeed = RSpeed = 0;
        TBufSize = sizeof(TBuf);
        InCommand = FALSE;

        // standard NVT to start
        memset(Enabled, 0, sizeof(Enabled));

		int Option = TRUE;
        setsockopt(S, SOL_SOCKET, SO_OOBINLINE, (char *) &Option, sizeof(Option));

        label mName;
        sprintf(mName, "WincitMutex:1%p", this);
        RMutex = CreateMutex(NULL, FALSE, mName);
        sprintf(mName, "WincitMutex:2%p", this);
        TMutex = CreateMutex(NULL, FALSE, mName);

        long H = _beginthread(TelnetR, 1024, (void *) this);
        if (H == -1)
            {
            RThreadRunning = FALSE;
            RThread = INVALID_HANDLE_VALUE;
            }
        else
            {
            RThread = (HANDLE) H;
            SetThreadPriority(RThread, THREAD_PRIORITY_NORMAL);
//          SetThreadPriority(RThread, THREAD_PRIORITY_ABOVE_NORMAL);
            }

        H = _beginthread(TelnetT, 1024, (void *) this);
        if (H == -1)
            {
            // If we couldn't start up the buffer flushing thread, don't
            // buffer output.
            TBufSize = 0;
            TThreadRunning = FALSE;
            TThread = INVALID_HANDLE_VALUE;
            }
        else
            {
            TThread = (HANDLE) H;
            SetThreadPriority(TThread, THREAD_PRIORITY_NORMAL);
///         SetThreadPriority(TThread, THREAD_PRIORITY_ABOVE_NORMAL);
            }

        // And let client know what we want
        label Out;
        Out[0] = 255;
        Out[1] = 253;   // DO

        Out[2] = 32;    // Terminal speed
        SendData(Out, 3);
        Enabled[32] = TRUE;

        Out[2] = 10;    // Force logout
        SendData(Out, 3);
        Enabled[10] = TRUE;

        Out[2] = 0;     // Binary
        SendData(Out, 3);
        Enabled[0] = TRUE;

        Out[2] = 3;     // Supress GA
        SendData(Out, 3);
        Enabled[3] = TRUE;

        Out[1] = 251;   // WILL
        Out[2] = 1;     // Local echo off
        SendData(Out, 3);
        Enabled[1] = TRUE;
        }

    virtual ~TelnetPortC(void);

    virtual Bool HaveConnection(void) const
        {
        return (!Closed);
        }

    virtual void Init(void) {}
    virtual void Deinit(void) {}

    virtual void SetSpeed(PortSpeedE NewSpeed)
        {
        Speed = NewSpeed;
        }

    virtual void BreakConnection(void)
        {
        Closed = TRUE;
        }

    virtual long GetTransmitSpeed(void)
        {
        return (TSpeed);
        }

    virtual long GetReceiveSpeed(void)
        {
        return (RSpeed);
        }
    };


void StartTelnetD(void);
void StopTelnetD(void);
