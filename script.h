// -------------------------------------------------------------------------
// Citadel: Script.H
//
// Structs, unions, and enums used by the script interpretter.


#include "cwindows.h"
#include "timer.h"

// data types
enum VariableTypeE
	{
	TYPE_NONE,		TYPE_INT,		TYPE_UINT,		TYPE_LONG,
	TYPE_ULONG, 	TYPE_BOOL,

	TYPE_STRING_MIN = 6,		// 2 bytes (1 char + end)
	TYPE_STRING_MAX = 260,		// 256 bytes (255 chars + end)

	TYPE_STRUCT,	TYPE_STRUCTTEMPL,


	BY_REF = 0x8000,			// By reference
	};


// commands
enum CommandTypeE
	{
	CMD_UNK,

	// math
	CMD_ADD,		CMD_SUB,		CMD_MUL,		CMD_DIV,		CMD_MOD,
	CMD_ASGN,

	// looping and branching
	CMD_EXIT,		CMD_WHILE,		CMD_WHILENOT,	CMD_WEND,		CMD_DO,
	CMD_LOOP,		CMD_LOOPNOT,	CMD_GOTO,		CMD_LABEL,		CMD_RET,
	CMD_CALL,		CMD_RUN,		CMD_CHAIN,

	// conditional
	CMD_IF, 		CMD_IFNOT,		CMD_ELSE,		CMD_ENDIF,		CMD_CASE,
	CMD_SWITCH,		CMD_DEFAULT,	CMD_ENDSWITCH,

	// comparison
	CMD_EQ, 		CMD_GT, 		CMD_LT, 		CMD_NE,
	CMD_GE, 		CMD_LE,

	// strings
	CMD_STRB,		CMD_STRU,		CMD_STRL,		CMD_STRC,
	CMD_STRS,		CMD_STRE,		CMD_STRPR,		CMD_STRPL,

	// Boolean
	CMD_AND,		CMD_OR, 		CMD_XOR,
	};

#define MAXARGS 5

union cmdData
	{
	strList *sl;				// if cmd takes more than one parameter
	char *str;					// if cmd takes one parameter
	};

struct scriptCommandRecord
	{
	scriptCommandRecord *next;
	int line;					// line number in .CSF file
	CommandTypeE type;			// type of command
	void *ptr;					// For speedy stuff.
	ulong time; 				// For profiler.
	ulong runCounter;			// For profiler.
	cmdData c;					// command's parameters
	};

struct scriptVariables;

union varTypes
	{
	short i;					// value of INT
	ushort ui;					// value of UINT
	long l; 					// value of LONG
	ulong ul;					// value of ULONG
	Bool b; 					// value of Bool
	char *string;				// ptr to value of STR
	scriptVariables *s;			// ptr to struct stuff
	};

struct scriptVariables
	{
	scriptVariables *next;
	label name; 				// name of variable
	VariableTypeE type;			// type of variable
	varTypes v; 				// value of variable
	};

enum LoopTypeE
	{
	LOOP_NONE,

	LOOP_WHILE, 	LOOP_WHILENOT,		LOOP_DO
	};

struct scriptLoops
	{
	scriptLoops *next;
	LoopTypeE LoopType; 			// type of loop
	scriptCommandRecord *where; 	// where to loop to
	};

struct FunctionContext
	{
	FunctionContext *next;

	scriptLoops *loopInfo;
	scriptVariables *AutoVars;

	// argument data
	scriptVariables *argv[MAXARGS];

	// the arguments that are being referenced
	scriptVariables *byref[MAXARGS];
	};

struct scriptFunctionList
	{
	scriptFunctionList *next;

	label name; 					// name of function
	VariableTypeE rett; 			// type function returns

	// argument types
	VariableTypeE argt[MAXARGS];

	// argument names
	label argn[MAXARGS];

	// argument struct templates
	scriptVariables *args[MAXARGS];

	scriptVariables *vars;			// local variables (static)
	scriptVariables *AutoVars;		// local Automatic variables; copied to
										// FunctionContext when called

	scriptCommandRecord *cmds;		// commands of function

	FunctionContext *Context;		// Local for each call to function
	};

struct scriptCallList
	{
	scriptCallList *next;

	scriptFunctionList *oldFunc;	// function to return to
	scriptCommandRecord *oldCmd;	// command to return to
	FunctionContext *oldContext;	// context to return to
	};

enum
	{
	S_GVAR, 	S_LVAR, 	S_ADD,		S_SUB,		S_MUL,		S_DIV,
	S_MOD,		S_ASGN, 	S_STR,		S_WHILE,	S_WHILENOT, S_WEND,
	S_DO,		S_LOOP, 	S_LOOPNOT,	S_IF,		S_IFNOT,	S_ELSE,
	S_ENDIF,	S_GOTO, 	S_LABEL,	S_FUNC, 	S_RET,		S_ENDFUNC,
	S_CALL, 	S_RUN,		S_CHAIN,	S_EQ,		S_GT,		S_LT,
	S_NE,		S_GE,		S_LE,		S_EXIT, 	S_AND,		S_OR,
	S_XOR,		S_VERSION,	S_AVAR,		S_STRUCT,	S_ENDSTRUCT,S_SWITCH,
	S_CASE,		S_DEFAULT,	S_ENDSWITCH,

	S_NUM
	};


extern discardable *ScriptDData;			// all script discardable data
#define getscrmsg(xxx) ((char **) ScriptDData->next->next->aux)[xxx]

// Debugging stuff
struct Breakpoints
	{
	Breakpoints *next;
	int LineNum;
	};

#define MAXSCRIPTFILES	5
struct ScriptInfoS
	{
#ifdef WINCIT
	TermWindowC *TW;
#endif

	CITWINDOW *DebuggingWindow, *WWindow, *SourceWindow;
	Bool Debug, Pause;
	Bool oldSystemDebug, SingleStep;
	Breakpoints *BPoints;

	scriptVariables lastResult; 		// _RESULT of last cmd/func
	char wowza[256];					// string _RESULT

	scriptVariables *gvBase;			// our global variables
	scriptFunctionList *sfBase; 		// our functions
	scriptCallList *callList;			// where to #RETurn

	scriptVariables *structTemplates;	// defined #STRUCTs

	char Name[81];						// name of current script

	scriptCommandRecord *curCmd;		// command in execution
	scriptFunctionList *curFunc;		// function in execution
	FunctionContext *curContext;		// context in execution

	FILE *UsrFile[MAXSCRIPTFILES];		// script user files
	Message *UsrMsg;					// script message.

	LogEntry *UsrUsr;					// script user.
	Bool UseCU; 						// If using CurrentUser.

	RoomC *UsrRoom;						// script room.
	Bool UseCR;							// If using CurrentRoom.

	strList *dupCheck;					// AddDupCheck, etc.
	directoryinfo *Files;				// script directory.

	Bool Error;

	scriptVariables byval;				// For returning BYVAL params
	char tmpstr[20];					// For numeric->string evals

#ifndef WINCIT
	Timer CmdTimer;
#endif
	scriptCommandRecord *LastCmd;

	Bool ToRet; 						// Back to Citadel

	Bool ReadAplOnExit; 				// Should we?

	void (*DeinitScript)(ScriptInfoS *si);
	};

extern long THREAD CurScript;
#define pCurScript ((ScriptInfoS *) CurScript)

#ifdef WINCIT
#define RequiresScriptTW()		\
	if (!pCurScript->TW)		\
		{						\
		SETBOOL(FALSE);			\
		return;					\
		}

#define ScriptTW() ((ScriptInfoS *) CurScript)->TW->
#else
#define RequiresScriptTW()
#define ScriptTW()
#endif

void DebuggingMsg(const char *Msg);
Bool RemoveWatch(const char *Expr);
Bool AddWatch(const char *Expr);
#ifdef MINGW
static void ToggleWatch(const char *Expr);
#else
void ToggleWatch(const char *Expr);
#endif

short evaluateInt(const char *what);
ushort evaluateUint(const char *what);
long evaluateLong(const char *what);
ulong evaluateUlong(const char *what);
Bool evaluateBool(const char *what);
const char *evaluateString(const char *what);


scriptVariables *FindStructTemplate(const char *Name);

