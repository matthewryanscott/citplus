/* -------------------------------------------------------------------- */
/*	EXTEDIT.H					Citadel									*/
/* -------------------------------------------------------------------- */
/*				How to use the external editor subsystem.				*/
/* -------------------------------------------------------------------- */

struct editors
	{
	editors *next;
	label name;					/* The name of the editor.				*/
	label cmd;					/* The command line for the editor.		*/
	Bool local : 1;				/* If you must be on local to run it.	*/
	Bool confirm : 1;			/* If you are asked for confirmation.	*/
	};

void ForgetAllExternalEditors(void);
Bool AddExternalEditor(const char *name, const char *cmd, Bool local, Bool cnfrm, Bool autoEditor);
