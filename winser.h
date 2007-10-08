// --------------------------------------------------------------------------
// Citadel: WinSer.H
//
// The Windows serial stuff

#ifdef WINCIT
void _USERENTRY SerialR(void *A);

class SerialPortC : public CommunicationPortC
    {
    HANDLE hDev;
    DWORD ModemStatus;

    Bool Closed;

    uchar RBuf[256];
    int RBufIdx, InRBuf;

    HANDLE RMutex;
    HANDLE RThread;

    friend void _USERENTRY SerialR(void *A);
    friend void _USERENTRY SerialStatusT(void *A);

    virtual uchar getChar(void);

    virtual void putChar(uchar Out)
        {
        if (!Closed)
            {
            DWORD Read;
            WriteFile(hDev, &Out, 1, &Read, NULL);
            }
        }

    HANDLE OwnerMutex;

public:
    ModemSpeedE ModemSpeed;

    SerialPortC(HANDLE H, int PNum) : CommunicationPortC(CT_SERIAL)
        {
        hDev = H;

        sprintf(PortName, "COM%d", PNum);
        PortNumber = PNum;

        Closed = FALSE;

        InRBuf = 0;
        RBufIdx = 0;

        label mName;
        sprintf(mName, "WincitMutex:%p", this);
        RMutex = CreateMutex(NULL, FALSE, mName);
        sprintf(mName, "WincitMutex1:%p", this);
        OwnerMutex = CreateMutex(NULL, FALSE, mName);


        // This is just a guess as to what will give the best performance.
        // Someone should do some testing some day.
        COMMTIMEOUTS Timeout;
        Timeout.ReadIntervalTimeout = 0;
        Timeout.ReadTotalTimeoutMultiplier = 0;
        Timeout.ReadTotalTimeoutConstant = 100;
        Timeout.WriteTotalTimeoutMultiplier = 0;
        Timeout.WriteTotalTimeoutConstant = 0;
        SetCommTimeouts(hDev, &Timeout);

        long Ht = _beginthread(SerialR, 1024, (void *) this);
        if (Ht == -1)
            {
            RThread = INVALID_HANDLE_VALUE;
            }
        else
            {
            RThread = (HANDLE) Ht;
            }

        ModemStatus = 0;
        _beginthread(SerialStatusT, 1024, (void *) this);
        }

    virtual ~SerialPortC(void)
        {
        // I'm not quite sure about this stuff, designed to close the
        // RThread...

        Closed = TRUE;
        WaitForSingleObject(RMutex, INFINITE);
        ReleaseMutex(RMutex);
        CloseHandle(RMutex);

        WaitForSingleObject(OwnerMutex, INFINITE);
        ReleaseMutex(OwnerMutex);
        CloseHandle(OwnerMutex);

        CloseHandle(hDev);
        }

    virtual InputReadyE CheckInputReady(void)
        {
        if (!InRBuf)
            {
            if (InputWaiting)
                {
                ResetEvent(InputWaiting);
                }
            }
        return (InRBuf ? IR_READY : IR_NONE);
        }

    virtual void FlushOutput(void)
        {
        PurgeComm(hDev, PURGE_TXCLEAR);
        }

    virtual Bool HaveConnection(void) const
        {
        return (!Closed && (ModemStatus & MS_RLSD_ON));
        }

    virtual void Init(void) {}
    virtual void Deinit(void) {}
    virtual void SetSpeed(PortSpeedE NewSpeed);

    Bool IsRingIndicator(void) const
        {
        return (!Closed && (ModemStatus & MS_RING_ON));
        }

    virtual void BreakConnection(void);
    void RaiseDtr(void) const;
    void DropDtr(void);
    void CycleDTR(void);

    Bool Own(void)
        {
        return (WaitForSingleObject(OwnerMutex, 2500) != WAIT_TIMEOUT);
        }

    void Disown(void)
        {
        ReleaseMutex(OwnerMutex);
        }

    virtual void Pause(void)
        {
        WaitForSingleObject(RMutex, INFINITE);
        CloseHandle(hDev);
        hDev = INVALID_HANDLE_VALUE;
        }

    virtual void Resume(void)
        {
        Bool Good = FALSE;

        label ComName;
        sprintf(ComName, "COM%d", PortNumber);

        HANDLE h = CreateFile(ComName, GENERIC_READ | GENERIC_WRITE,
                0, NULL, OPEN_EXISTING, 0, NULL);

        if (h != INVALID_HANDLE_VALUE)
            {
            DCB dcb;

            if (GetCommState(h, &dcb))
                {
                dcb.BaudRate = bauds[cfg.initbaud];
                dcb.fBinary = TRUE;
                dcb.fParity = FALSE;
                dcb.fOutxCtsFlow = cfg.checkCTS;
                dcb.fOutxDsrFlow = FALSE;
                dcb.fDtrControl = DTR_CONTROL_ENABLE;
                dcb.fDsrSensitivity = FALSE;
//              dcb.fTXContinueOnXoff:1; // XOFF continues Tx

                dcb.fOutX = FALSE;
                dcb.fInX = FALSE;
//              dcb.fErrorChar: 1;   // enable error replacement
                dcb.fNull = FALSE;
                dcb.fRtsControl = RTS_CONTROL_ENABLE;
                dcb.fAbortOnError = FALSE;
//              dcb.fDummy2:17;      // reserved
//              dcb.wReserved;        // not currently used
//              dcb.XonLim;           // transmit XON threshold

//              dcb.XoffLim;          // transmit XOFF threshold
                dcb.ByteSize = 8;
                dcb.Parity = NOPARITY;
                dcb.StopBits = ONESTOPBIT;
//              dcb.XonChar;          // Tx and Rx XON character
//              dcb.XoffChar;         // Tx and Rx XOFF character
//              dcb.ErrorChar;        // error replacement character
//              dcb.EofChar;          // end of input character
//              dcb.EvtChar;          // received event character

                if (SetCommState(h, &dcb))
                    {
                    Good = TRUE;
                    }
                }

            if (!Good)
                {
                CloseHandle(h);
                }
            }

        if (Good)
            {
            hDev = h;
            }
        else
            {
            hDev = INVALID_HANDLE_VALUE;
            Closed = TRUE;
            }

        ReleaseMutex(RMutex);
        }
    };
#endif
