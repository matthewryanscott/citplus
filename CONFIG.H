extern Bool CitadelIsConfiguring;	// CONFIG.CPP

// CONFIG.CPP
void buildhalls(void);
void cdecl illegal(const char *fmt, ...);
void WaitUntilDoneConfiguring(void);
void CreateConfigurationFile(int FileNameIndex, int FileDataIndex, Bool PrintStuff);
#ifdef WINCIT
void mPrintfOrUpdateCfgDlg(TermWindowC *TW, const char *fmt, ...);
void CRmPrintfOrUpdateCfgDlg(TermWindowC *TW, const char *fmt, ...);
void CRmPrintfCROrUpdateCfgDlg(TermWindowC *TW, const char *fmt, ...);
void mPrintfCROrUpdateCfgDlg(TermWindowC *TW, const char *fmt, ...);
void cPrintfOrUpdateCfgDlg(TermWindowC *TW, const char *fmt, ...);
#else
void mPrintfOrUpdateCfgDlg(int, const char *fmt, ...);
void CRmPrintfOrUpdateCfgDlg(int, const char *fmt, ...);
void CRmPrintfCROrUpdateCfgDlg(int, const char *fmt, ...);
void mPrintfCROrUpdateCfgDlg(int, const char *fmt, ...);
#endif


// EXTERNAL.CPP
void initExtDrivers(void);


// INIT.CPP
void critical(Bool turnOn);
int fileHandlesLeft(void);
void startUp(int argc, const char *argv[]);


// MAKETAB.CPP
void logInit(void);


// MSGCFG.CPP
void msgInit(void);


// OPENFILE.CPP
void openFile(const char *filename, FILE **fd);


// READCFG.CPP
#ifdef WINCIT
int readconfig(TermWindowC *TW, int ignore);
#else
int readconfig(void *, int ignore);
#endif


// READEXT.CPP
void ReadExternalCit(WC_TW);
void ReadProtocolCit(WC_TW);
void ReadMdmresltCit(WC_TW);
#ifdef WINCIT
void ReadMCICit(Bool TellIfNotFound, WC_TW);
#else
void ReadMCICit(Bool TellIfNotFound);
#endif
void ReadCommandsCit(WC_TW);
