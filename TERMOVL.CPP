// --------------------------------------------------------------------------
// Citadel: TermOvl.CPP
//
// Overlayed terminal emulation stuff.

#include "ctdl.h"
#pragma hdrstop

#include "log.h"
#include "term.h"
#include "cfgfiles.h"
#include "miscovl.h"
#include "extmsg.h"


// --------------------------------------------------------------------------
// Contents
//
// autoansi()			Handles auto ansi detect.


// --------------------------------------------------------------------------
// autoansi(): This sets term at according to cfg.autoansi or does actual
//	auto ansi detect.

void TERMWINDOWMEMBER autoansi(void)
	{
	char x;
	char inch;
	char detected = 0;
	long t;

	if (cfg.autoansi == 0)
		{
		setdefaultTerm(TT_DUMB);
		}
	else if (cfg.autoansi == 2)
		{
		setdefaultTerm(TT_ANSI);
		}
	else
		{
		if (modStat)
			{
			mPrintf(pctss, getmsg(85));

			CommPort->FlushInput();

			for (x = 0; x < 3 && !CommPort->IsInputReady(); x++)
				{
				CommPort->OutString("[6n");
				pause(50);
				}

			if (CommPort->IsInputReady())
				{
				inch = (char)CommPort->Input();

				if (inch == ESC)
					{
					detected = TRUE;
					mPrintfCR(getmsg(87));
					doCR();
					setdefaultTerm(TT_ANSI);
					}
				}

			if (!detected)
				{
				mPrintfCR(getmsg(88));
				CRmPrintf(getmsg(89));

				t = time(NULL);

				CommPort->FlushInput();

				inch = 0;

				do
					{
					if (CommPort->IsInputReady())
						{
						inch = (char)CommPort->Input();
						}

					if (KBReady())
						{
						inch = (char) ciChar();
						}

					inch = (char) toupper(inch);
					} while (CommPort->HaveConnection() && (inch != ' ') &&
							(inch != 'A') && (inch != 'S') &&
							(inch != ESC) && ((uint) (time(NULL) - t) < 5));

				doCR();

				if (toupper(inch) == 'A')
					{
					setdefaultTerm(TT_ANSI);
					}
				else
					{
					setdefaultTerm(TT_DUMB);
					}
				}
			}
		else
			{
			setdefaultTerm(TT_ANSI);
			}
		}

	CommPort->FlushInput();
	}


// --------------------------------------------------------------------------
// setdefaultTerm(): Setup the terminal

void TERMWINDOWMEMBER setdefaultTerm(TermTypeE NewType)
	{
	switch (NewType)
		{
		case TT_DUMB:
			{
			TermCap->Load("Dumb");
			break;
			}

		case TT_ANSI:
			{
			TermCap->Load("ANSI Color");
			break;
			}
		}
	}


// --------------------------------------------------------------------------
// askAttributes(): Lets the user config own colors.

void TERMWINDOWMEMBER askAttributes(LogEntry1 *EditLog)
	{
	doCR();

	EditLog->SetAttribute(ATTR_NORMAL, (int) GetNumberWithBlurb(getmsg(94),
			0L, 255L, EditLog->GetAttribute(ATTR_NORMAL), B_COLORS));

	EditLog->SetAttribute(ATTR_BLINK, (int) GetNumberWithBlurb(getmsg(95),
			0L, 255L, EditLog->GetAttribute(ATTR_BLINK), B_COLORS));

	EditLog->SetAttribute(ATTR_REVERSE, (int) GetNumberWithBlurb(getmsg(96),
			0L, 255L, EditLog->GetAttribute(ATTR_REVERSE), B_COLORS));

	EditLog->SetAttribute(ATTR_BOLD, (int) GetNumberWithBlurb(getmsg(98),
			0L, 255L, EditLog->GetAttribute(ATTR_BOLD), B_COLORS));

	EditLog->SetAttribute(ATTR_UNDERLINE, (int) GetNumberWithBlurb(getmsg(99),
			0L, 255L, EditLog->GetAttribute(ATTR_UNDERLINE), B_COLORS));
	}


// This function is a bit ugly. Oh well.
Bool IsColorOrFormattingOnly(const char *Code)
	{
	return (isdigit(*Code) ||
			(toupper(*Code) >= 'A' && toupper(*Code) <= 'H') ||
			toupper(*Code) == 'R');
	}


Bool TermCapC::Load(const char *Type)
	{
	Clear();

	// If we don't have a name, nothing to load
	if (!*Type)
		{
		return (TRUE);
		}

	Bool Loaded = FALSE;

	char fullPathToTermCapCit[256];
	sprintf(fullPathToTermCapCit, sbs, cfg.homepath,
			citfiles[C_TERMCAP_CIT]);

	FILE *fBuf;
	if ((fBuf = fopen(fullPathToTermCapCit, FO_R)) == NULL)	// ASCII mode
		{
#ifndef WINCIT
		cPrintf(getmsg(15), fullPathToTermCapCit);
		doccr();
#endif
		}
	else
		{
		if (LockMessages(MSG_TERMCAP))
			{
			Bool Started = FALSE;

			int lineNo = 0;

			const char **termkeywords = (const char **)
					ExtMessages[MSG_TERMCAP].Data->next->aux;

			const char **termtypes = (const char **)
					ExtMessages[MSG_TERMCAP].Data->next->next->aux;

			char line[257];
			while ((fgets(line, sizeof(line) - 1, fBuf) != NULL) && !Loaded)
				{
				lineNo++;

				if (line[0] != '#')
					{
					continue;
					}

				char *words[256];
				const int count = parse_it(words, line);

				int i;
				for (i = 0; i < TCK_NUM; i++)
					{
					if (SameString(words[0] + 1, termkeywords[i]))
						{
						break;
						}
					}

				if (count > 1)
					{
					switch (i)
						{
						case TCK_TERMINAL:
							{
							if (Started)
								{
								Loaded = TRUE;
								}
							else if (SameString(words[1], Type))
								{
								Started = TRUE;
								CopyStringToBuffer(Name, words[1]);
								}

							break;
							}

						case TCK_IBMUPPER:
							{
							if (Started)
								{
								Bool Error;
								Bool Value = GetBooleanFromCfgFile(words[1], &Error);

								if (Error)
									{
#ifndef WINCIT
									cPrintf(getmsg(MSG_TERMCAP, 0), citfiles[C_TERMCAP_CIT], lineNo, termkeywords[i],
											words[1]);
									doccr();
#endif
									}
								else
									{
									IBMExtended = Value;
									}
								}

							break;
							}

						case TCK_COLOR:
							{
							if (Started)
								{
								Bool Error;
								Bool Value = GetBooleanFromCfgFile(words[1],
										&Error);

								if (Error)
									{
#ifndef WINCIT
									cPrintf(getmsg(MSG_TERMCAP, 0),
											citfiles[C_TERMCAP_CIT], lineNo,
											termkeywords[i], words[1]);
									doccr();
#endif
									}
								else
									{
									Color = Value;
									}
								}

							break;
							}

						case TCK_BLANKFOR1CURS:
							{
							if (Started)
								{
								Bool Error;
								Bool Value = GetBooleanFromCfgFile(words[1],
										&Error);

								if (Error)
									{
#ifndef WINCIT
									cPrintf(getmsg(MSG_TERMCAP, 0),
											citfiles[C_TERMCAP_CIT], lineNo,
											termkeywords[i], words[1]);
									doccr();
#endif
									}
								else
									{
									BlankForCursorOne = Value;
									}
								}

							break;
							}

						case TCK_BLANKFORTLCUR:
							{
							if (Started)
								{
								Bool Error;
								Bool Value = GetBooleanFromCfgFile(words[1],
										&Error);

								if (Error)
									{
#ifndef WINCIT
									cPrintf(getmsg(MSG_TERMCAP, 0),
											citfiles[C_TERMCAP_CIT], lineNo,
											termkeywords[i], words[1]);
									doccr();
#endif
									}
								else
									{
									BlankForTLCursor = Value;
									}
								}

							break;
							}

						case TCK_TERMTYPE:
							{
							if (Started)
								{
								int tt;
								for (tt = 0; tt < TT_NUM; tt++)
									{
									if (SameString(words[1], termtypes[tt]))
										{
										break;
										}
									}

								if (tt == TT_NUM)
									{
#ifndef WINCIT
									cPrintf(getmsg(MSG_TERMCAP, 0),
											citfiles[C_TERMCAP_CIT], lineNo,
											termkeywords[i], words[1]);
									doccr();
#endif

									tt = TT_UNKNOWN;
									}

								TermType = (TermTypeE) tt;
								}

							break;
							}

						case TCK_CLS:
							{
							if (Started)
								{
								CopyStringToBuffer(ClsCode, words[1]);
								}

							break;
							}

						case TCK_FORECOLOR:
							{
							if (Started)
								{
								CopyStringToBuffer(SetForeCode, words[1]);
								}

							break;
							}

						case TCK_BACKCOLOR:
							{
							if (Started)
								{
								CopyStringToBuffer(SetBackCode, words[1]);
								}

							break;
							}

						case TCK_FULLCOLOR:
							{
							if (Started)
								{
								CopyStringToBuffer(SetBothCode, words[1]);
								}

							break;
							}

						case TCK_HIGHFORECOLOR:
							{
							if (Started)
								{
								CopyStringToBuffer(SetHighForeCode, words[1]);
								}

							break;
							}

						case TCK_HIGHBOTHCOLOR:
							{
							if (Started)
								{
								CopyStringToBuffer(SetHighBothCode, words[1]);
								}

							break;
							}

						case TCK_UNDERLINE:
							{
							if (Started)
								{
								CopyStringToBuffer(UnderlineCode, words[1]);
								}

							break;
							}

						case TCK_BLINK:
							{
							if (Started)
								{
								CopyStringToBuffer(FlashCode, words[1]);
								}

							break;
							}

						case TCK_NORMAL:
							{
							if (Started)
								{
								CopyStringToBuffer(NormalCode, words[1]);
								}

							break;
							}

						case TCK_REVERSE:
							{
							if (Started)
								{
								CopyStringToBuffer(ReverseCode, words[1]);
								}

							break;
							}

						case TCK_BOLD:
							{
							if (Started)
								{
								CopyStringToBuffer(BoldCode, words[1]);
								}

							break;
							}

						case TCK_DELEOL:
							{
							if (Started)
								{
								CopyStringToBuffer(DelEolCode, words[1]);
								}

							break;
							}

						case TCK_SAVECURS:
							{
							if (Started)
								{
								CopyStringToBuffer(SaveCursCode, words[1]);
								}

							break;
							}

						case TCK_RESTORECURS:
							{
							if (Started)
								{
								CopyStringToBuffer(RestCursCode, words[1]);
								}

							break;
							}

						case TCK_CURSUP:
							{
							if (Started)
								{
								CopyStringToBuffer(CursorUpCode, words[1]);
								}

							break;
							}

						case TCK_CURSDOWN:
							{
							if (Started)
								{
								CopyStringToBuffer(CursorDownCode, words[1]);
								}

							break;
							}

						case TCK_CURSLEFT:
							{
							if (Started)
								{
								CopyStringToBuffer(CursorLeftCode, words[1]);
								}

							break;
							}

						case TCK_CURSRIGHT:
							{
							if (Started)
								{
								CopyStringToBuffer(CursorRightCode, words[1]);
								}

							break;
							}

						case TCK_ABSCURSPOS:
							{
							if (Started)
								{
								CopyStringToBuffer(CursorAbsoluteCode, words[1]);
								}

							break;
							}

						case TCK_ROWSTART:
							{
							if (Started)
								{
								CursTopNum = atoi(words[1]);
								}

							break;
							}

						case TCK_COLSTART:
							{
							if (Started)
								{
								CursLeftNum = atoi(words[1]);
								}

							break;
							}

						default:
							{
#ifndef WINCIT
							cPrintf(getmsg(21), citfiles[C_TERMCAP_CIT],
									words[0]);
							doccr();
#endif
							}
						}
					}
				else
					{
#ifndef WINCIT
					cPrintf(getmsg(MSG_TERMCAP, 1), citfiles[C_TERMCAP_CIT],
							lineNo, termkeywords[i]);
					doccr();
#endif
					}
				}

			Loaded = Started;
	  
			UnlockMessages(MSG_TERMCAP);
			}
		else
			{
			OutOfMemory(113);
			}

		fclose(fBuf);
		}

	return (Loaded);
	}

const char *GetTermType(int Num, char *Buffer, int Length)
	{
	char *Return = NULL;

	char fullPathToTermCapCit[128];
	sprintf(fullPathToTermCapCit, sbs, cfg.homepath,
			citfiles[C_TERMCAP_CIT]);

	FILE *fBuf;
	if ((fBuf = fopen(fullPathToTermCapCit, FO_R)) == NULL)	// ASCII mode
		{
#ifndef WINCIT
		mPrintfCR(getmsg(15), fullPathToTermCapCit);
#endif
		}
	else
		{
		if (LockMessages(MSG_TERMCAP))
			{
			const char **termkeywords = (const char **)
					ExtMessages[MSG_TERMCAP].Data->next->aux;

			char line[257];
			while ((fgets(line, sizeof(line) - 1, fBuf) != NULL))
				{
				if (line[0] != '#')
					{
					continue;
					}

				char *words[256];
				const int count = parse_it(words, line);

				if (count > 1 &&
						SameString(termkeywords[TCK_TERMINAL], words[0] + 1))
					{
					if (Num)
						{
						Num--;
						}
					else
						{
						CopyString2Buffer(Buffer, words[1], Length);
						Return = Buffer;
						break;
						}
					}
				}

			UnlockMessages(MSG_TERMCAP);
			}
		else
			{
			OutOfMemory(113);
			}

		fclose(fBuf);
		}

	return (Return);
	}

const char *TermCapC::GetName(void)
	{
	return (Name);
	}
