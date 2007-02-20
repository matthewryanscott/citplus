// MISC2, MISC3, and UIMISC stuff

// MISC2.CPP
const char *nodet_or_address(void);
void buildaddress(void);
PortSpeedE digitbaud(long bigbaud);
ModemSpeedE digitConnectbaud(long bigbaud);
void showStatScreen(void);
void cdecl nullFunc(void);
void cyclesignature(void);


// MISC3.CPP
void logo(int turnOn);
void parseArgs(int argc, const char *argv[]);
const char *mkDblBck(const char *str, char *newstr);
Bool TurnDebugOn(void);
Bool TurnDebugOff(void);
long RomanInput(const char *Roman);
const char *RomanOutput(long Number);
void memoryDump(void);
Bool GetBooleanFromCfgFile(const char *Setting, Bool *Error);


// Status, only when VERSION != RELEASE
void CheckDiscardablePtr(const discardable *D, const void *Check, const char *Name);
