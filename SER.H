#ifndef WINCIT
extern "C"
	{
	// SER.ASM
	extern void cdecl InitRS(int Port, int Baud, int Stops, int Parity, int Length, int cCts);
	extern void cdecl DeInitRS(void);
	extern void cdecl FlushRS(void);
	extern int cdecl StatRS(void);
	extern int cdecl GetRS(void);
	extern void cdecl PutRS(char ch);
	extern void cdecl DtrRS(int dtr);
	extern int cdecl CarrStatRS(void);
	extern int cdecl RingStatRS(void);
	extern int cdecl StatCON(void);
	extern int cdecl GetCON(void);
	};
#endif
