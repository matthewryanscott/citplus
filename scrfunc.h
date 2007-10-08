// --------------------------------------------------------------------------
// Citadel: ScrFunc.H
//
// Wow.


#include "script.h"

#define SETINT(x)	pCurScript->lastResult.v.i = x; \
					pCurScript->lastResult.type = TYPE_INT

#define SETUINT(x)	pCurScript->lastResult.v.ui = x; \
					pCurScript->lastResult.type = TYPE_UINT

#define SETLONG(x)	pCurScript->lastResult.v.l = x; \
					pCurScript->lastResult.type = TYPE_LONG

#define SETULONG(x)	pCurScript->lastResult.v.ul = x; \
					pCurScript->lastResult.type = TYPE_ULONG

#define SETBOOL(x)	pCurScript->lastResult.v.b = x; \
					pCurScript->lastResult.type = TYPE_BOOL

#define SETSTR(x)	CopyStringToBuffer(pCurScript->wowza, x);			\
					pCurScript->lastResult.v.string = pCurScript->wowza;\
					pCurScript->lastResult.type = TYPE_STRING_MAX

#define SETRTSTR()	pCurScript->lastResult.v.string = pCurScript->wowza;\
					pCurScript->lastResult.type = TYPE_STRING_MAX
