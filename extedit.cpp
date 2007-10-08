/* -------------------------------------------------------------------- */
/*	EXTEDIT.CPP 				Citadel 								*/
/*					The external editor subsystem.						*/
/* -------------------------------------------------------------------- */
/*	Citadel has an internal message editor which is quick and powerful, */
/*	but can never be a match for a good full-screen editor. Because of	*/
/*	this, there is support of external editors. These can also be used	*/
/*	to implement message filters. Editors are defined in EXTERNAL.CIT	*/
/*	and handled by this module. 										*/
/* -------------------------------------------------------------------- */
#include "ctdl.h"
#pragma hdrstop

#include "extedit.h"
#include "events.h"

/* -------------------------------------------------------------------- */
/*								Contents								*/
/* -------------------------------------------------------------------- */
/*								Public									*/
/*	Functions:															*/
/*		RunExternalEditor() 											*/
/*			Attempts to run an external editor, given the keystroke to	*/
/*			call it.													*/
/*																		*/
/*		RunAutomaticExternalEditors()									*/
/*			Attempts to run all valid automatic external editors.		*/
/*																		*/
/*		ForgetAllExternalEditors()										*/
/*			Removes record of all editors from memory.					*/
/*																		*/
/*		AddExternalEditor() 											*/
/*			Adds an external editor to memory.							*/
/*																		*/
/* -------------------------------------------------------------------- */
/*								Private 								*/
/*	Functions:															*/
/*		doExtEdit() 	Runs a specified editor on specified text.		*/
/*																		*/
/*	Data types: 														*/
/*		editors 		Structure used to keep track of editors.		*/
/*																		*/
/*	Data:																*/
/*		extEditorList	Base of list of editors.						*/
/*		autoEditList	Base of list of automatic editors.				*/
/* -------------------------------------------------------------------- */


/* -------------------------------------------------------------------- */
/* -------------------------------------------------------------------- */

editors *extEditorList; 		/* our external editors 				*/
editors *autoEditList;			/* our automatic editors				*/


/* -------------------------------------------------------------------- */
/* -------------------------------------------------------------------- */


/* -------------------------------------------------------------------- */
/*	RunExternalEditor() Attempts to run an external editor, given the	*/
/*						keystroke to call it.							*/
/*																		*/
/*	Input:		Key 		Key for editor. 							*/
/*				len 		Maximum length of the text. 				*/
/*																		*/
/*	Modified:	text		Text to edit. Returns as edited text.		*/
/*																		*/
/*	Output: 	return		TRUE if the editor exists.					*/
/*							FALSE if the editor does not exist. 		*/
/* -------------------------------------------------------------------- */
Bool TERMWINDOWMEMBER RunExternalEditor(char Key, size_t len, char *text)
	{
	editors *theEditor;

	for (theEditor = extEditorList; theEditor;
			theEditor = (editors *) getNextLL(theEditor))
		{
		if (tolower(Key) == tolower(*theEditor->name))
			{
			break;
			}
		}

	if (theEditor)
		{
		doExtEdit(theEditor, FALSE, len, text);
		return (TRUE);
		}
	else
		{
		return (FALSE);
		}
	}


/* -------------------------------------------------------------------- */
/*	RunAutomaticExternalEditors()	Attempts to run all valid automatic */
/*									external editors.					*/
/*																		*/
/*	Input:		len 		Maximum length of the text. 				*/
/*																		*/
/*	Modified:	text		Text to edit. Returns as edited text.		*/
/* -------------------------------------------------------------------- */
void TERMWINDOWMEMBER RunAutomaticExternalEditors(size_t len, char *text)
	{
	editors *theEditor;

	for (theEditor = autoEditList; theEditor;
			theEditor = (editors *) getNextLL(theEditor))
		{
		doExtEdit(theEditor, TRUE, len, text);
		}
	}


/* -------------------------------------------------------------------- */
/*	ForgetAllExternalEditors()		Removes record of all editors from	*/
/*									memory. 							*/
/* -------------------------------------------------------------------- */
void ForgetAllExternalEditors(void)
	{
	disposeLL((void **) &extEditorList);
	disposeLL((void **) &autoEditList);
	}


/* -------------------------------------------------------------------- */
/*	AddExternalEditor() 	Adds an external editor to memory.			*/
/*																		*/
/*	Input:		name		Name of the editor. 						*/
/*				cmd 		Command line for the editor.				*/
/*				local		TRUE if can only be run on console. 		*/
/*				cnfrm		TRUE if user has to confirm the run.		*/
/*				autoEditor	TRUE if it should be an automatic editor.	*/
/*																		*/
/*	Output: 	return		TRUE if everything worked.					*/
/*							FALSE if it didn't. (Out of memory.)        */
/* -------------------------------------------------------------------- */
Bool AddExternalEditor(const char *name, const char *cmd, Bool local,
		Bool cnfrm, Bool autoEditor)
	{
	editors *theEditor;

	theEditor = (editors *) addLL((void **)
			(autoEditor ? &autoEditList : &extEditorList),
				sizeof(*theEditor));

	if (theEditor)
		{
		strncpy(theEditor->name, name, LABELSIZE);
		theEditor->name[LABELSIZE] = 0;

		theEditor->local = local;

		strncpy(theEditor->cmd, cmd, LABELSIZE);
		theEditor->cmd[LABELSIZE] = 0;

		theEditor->confirm = cnfrm;

		return (TRUE);
		}
	else
		{
		return (FALSE);
		}
	}


/* -------------------------------------------------------------------- */
/* -------------------------------------------------------------------- */


/* -------------------------------------------------------------------- */
/*	doExtEdit() 		Runs a specified editor on specified text.		*/
/*																		*/
/*	Input:		theEditor	A pointer to the editor structure to use.	*/
/*				autoEdit	A boolean: if TRUE, run as an automatic 	*/
/*							editor. If FALSE, run as a standard editor. */
/*				len 		How long text can be, including Nul.		*/
/*																		*/
/*	Modified:	text		A pointer to the text to edit.				*/
/* -------------------------------------------------------------------- */
void TERMWINDOWMEMBER doExtEdit(const editors *theEditor, Bool autoEdit,
		size_t len, char *text)
	{
	static const char *edittextTmp = "EDITTEXT.TMP";

	if (!autoEdit)
		{
		mPrintfCR(pcts, theEditor->name);
		}

	if (theEditor->local && !onConsole)
		{
		if (!autoEdit)
			{
			CRmPrintfCR(getmsg(473));
			}
		}
	else
		{
		FILE *fd;

		if (theEditor->confirm)
			{
			char string[128];

			if (autoEdit)
				{
				sprintf(string, getmsg(639), theEditor->name);
				}
			else
				{
				strcpy(string, getmsg(57));
				}

			if (!getYesNo(string, 1))
				{
				return;
				}
			}

		char EditFileName[128];

		sprintf(EditFileName, sbs, cfg.aplpath, edittextTmp);

		if ((fd = fopen(EditFileName, FO_WB)) != NULL)
			{
			dFormat(text, fd);
			fclose(fd);

			RunApplication(theEditor->cmd, edittextTmp, TRUE, TRUE);

			if ((fd = fopen(EditFileName, FO_RB)) != NULL)
				{
				GetFileMessage(fd, text, len - 1);
				fclose(fd);
				}
			else
				{
				CRmPrintfCR(getmsg(78), EditFileName);
				}

			unlink(EditFileName);
			}
		else
			{
			CRmPrintfCR(getmsg(78), EditFileName);
			}
		}
	}
