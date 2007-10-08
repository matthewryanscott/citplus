// --------------------------------------------------------------------------
// Citadel: AplFile.CPP
//
// INPUT.APL/OUTPUT.APL/MESSAGE.APL/others code

#include "ctdl.h"
#pragma hdrstop
#include "applic.h"
#include "apstruct.h"
#include "statline.h"
#include "msg.h"
#include "room.h"
#include "log.h"
#include "account.h"
#include "hall.h"
#include "filecmd.h"
#include "term.h"

// --------------------------------------------------------------------------
// Contents
//
// writeAplFile 	Writes OUTPUT.APL
// readAplFile		reads input.apl from disk
//
// writeCallinfo	writes CALLINFO.BBS (most undefined)
// writeChain		writes CHAIN.TXT
// writeDoorfileSr	writes DOORFILE.SR
// writeDoorSys 	writes DOOR.SYS 	[Jim Harrer can't program]
// writeDorinfo 	writes DORINFO1.DEF
// writePcboard 	writes PCBOARD.SYS
// writeTribbs		writes TRIBBS.SYS
// writeUserinfo	writes USERINFO.DAT
//	"???" = undefined item, "!!!" = unimplemented item

static const char * const callinfoBbs	= "CALLINFO.BBS";
static const char * const chainTxt		= "CHAIN.TXT";
static const char * const doorfileSr	= "DOORFILE.SR";
static const char * const doorSys		= "DOOR.SYS";
static const char * const dorinfo1Def	= "DORINFO1.DEF";
static const char * const pcboardSys	= "PCBOARD.SYS";
static const char * const tribbsSys 	= "TRIBBS.SYS";
static const char * const userinfoDat	= "USERINFO.DAT";

static void eraseAplFiles(void)
	{
	char FileName[128];

	sprintf(FileName, sbs, cfg.aplpath, outputApl);
	unlink(FileName);

	sprintf(FileName, sbs, cfg.aplpath, inputApl);
	unlink(FileName);

	sprintf(FileName, sbs, cfg.aplpath, messageApl);
	unlink(FileName);

	sprintf(FileName, sbs, cfg.aplpath, readmeApl);
	unlink(FileName);

	sprintf(FileName, sbs, cfg.aplpath, consoleApl);
	unlink(FileName);

	sprintf(FileName, sbs, cfg.aplpath, callinfoBbs);
	unlink(FileName);

	sprintf(FileName, sbs, cfg.aplpath, chainTxt);
	unlink(FileName);

	sprintf(FileName, sbs, cfg.aplpath, doorfileSr);
	unlink(FileName);

	sprintf(FileName, sbs, cfg.aplpath, doorSys);
	unlink(FileName);

	sprintf(FileName, sbs, cfg.aplpath, dorinfo1Def);
	unlink(FileName);

	sprintf(FileName, sbs, cfg.aplpath, pcboardSys);
	unlink(FileName);

	sprintf(FileName, sbs, cfg.aplpath, tribbsSys);
	unlink(FileName);

	sprintf(FileName, sbs, cfg.aplpath, userinfoDat);
	unlink(FileName);
	}


// --------------------------------------------------------------------------
// writeAplFile(): Writes OUTPUT.APL.

void TERMWINDOWMEMBER writeAplFile(void)
	{
	FILE *fd;
	char buff[80];
	int i;

	for (i = 0; AplTab[i].item != APL_END; i++)
		{
		switch (AplTab[i].item)
			{
			case APL_SYSOPNAME:
				{
				AplTab[i].variable = cfg.sysop;
				break;
				}

			case APL_MDATA:
				{
				AplTab[i].variable = &cfg.mdata;
				break;
				}

			case APL_PRINTING:
				{
				AplTab[i].variable = &OC.Printing;
				break;
				}

			case APL_NODE:
				{
				AplTab[i].variable = cfg.nodeTitle;
				break;
				}

			case APL_REGION:
				{
				AplTab[i].variable = cfg.nodeRegion;
				break;
				}

			case APL_COUNTRY:
				{
				AplTab[i].variable = cfg.nodeCountry;
				break;
				}

			case APL_TEMPPATH:
				{
				AplTab[i].variable = LocalTempPath;
				break;
				}

			case APL_APPLPATH:
				{
				AplTab[i].variable = cfg.aplpath;
				break;
				}

			case APL_HELPPATH:
				{
				AplTab[i].variable = cfg.helppath;
				break;
				}

			case APL_HOMEPATH:
				{
				AplTab[i].variable = cfg.homepath;
				break;
				}

			case APL_ROOMPATH:
				{
				AplTab[i].variable = cfg.roompath;
				break;
				}

			case APL_MSGPATH:
				{
				AplTab[i].variable = cfg.msgpath;
				break;
				}

			case APL_PRINTPATH:
				{
				AplTab[i].variable = cfg.printer;
				break;
				}

			case APL_TRANSPATH:
				{
				AplTab[i].variable = cfg.transpath;
				break;
				}

			case APL_ATTR:
				{
				AplTab[i].variable = &cfg.attr;
				break;
				}

			case APL_WATTR:
				{
				AplTab[i].variable = &cfg.wattr;
				break;
				}

			case APL_CATTR:
				{
				AplTab[i].variable = &cfg.cattr;
				break;
				}

			case APL_UTTR:
				{
				AplTab[i].variable = &cfg.uttr;
				break;
				}

			case APL_BATTR:
				{
				AplTab[i].variable = &cfg.battr;
				break;
				}

			case APL_CONLOCK:
				{
				AplTab[i].variable = cfg.f6pass;
				break;
				}
			}
		}

	eraseAplFiles();

	sprintf(buff, sbs, cfg.aplpath, outputApl);
	if ((fd = fopen(buff, FO_W)) == NULL)
		{
		mPrintf(getmsg(8), outputApl);
		return;
		}

	for (i = 0; AplTab[i].item != APL_END; i++)
		{
		switch (AplTab[i].type)
			{
			case TYP_STR:
				{
				sprintf(buff, getmsg(249), AplTab[i].item,
						deansi((char *) AplTab[i].variable), bn);
				break;
				}

			case TYP_BOOL:
			case TYP_CHAR:
				{
				sprintf(buff, getmsg(247), AplTab[i].item,
						*((uchar *)AplTab[i].variable), bn);
				break;
				}

			case TYP_INT:
				{
				sprintf(buff, getmsg(246), AplTab[i].item,
						*((int *)AplTab[i].variable), bn);
				break;
				}

			case TYP_LONG:
				{
				sprintf(buff, getmsg(245), AplTab[i].item,
						*((long *)AplTab[i].variable), bn);
				break;
				}

			case TYP_OTHER:
				{
				char TermCodeBuffer[128];

				switch (AplTab[i].item)
					{
					case APL_ANSION:
						{
						sprintf(buff, getmsg(247), AplTab[i].item,
								TermCap->GetTermType() == TT_ANSI, bn);
						break;
						}

					case APL_TRM_NORM:
						{
						sprintf(buff, getmsg(249), AplTab[i].item,
								TermCap->GetNormal(TermCodeBuffer), bn);
						break;
						}

					case APL_TRM_BOLD:
						{
						sprintf(buff, getmsg(249), AplTab[i].item,
								TermCap->GetBold(TermCodeBuffer), bn);
						break;
						}

					case APL_TRM_INVR:
						{
						sprintf(buff, getmsg(249), AplTab[i].item,
								TermCap->GetInverse(TermCodeBuffer), bn);
						break;
						}

					case APL_TRM_BLINK:
						{
						sprintf(buff, getmsg(249), AplTab[i].item,
								TermCap->GetBlink(TermCodeBuffer), bn);
						break;
						}

					case APL_TRM_UNDER:
						{
						sprintf(buff, getmsg(249), AplTab[i].item,
								TermCap->GetUnderline(TermCodeBuffer), bn);
						break;
						}

					case APL_NAME:
						{
						label Buffer;
						CurrentUser->GetName(Buffer, sizeof(Buffer));

						sprintf(buff, getmsg(249), AplTab[i].item,
								Buffer, bn);
						break;
						}

					case APL_INITIALS:
						{
						label Buffer;
						CurrentUser->GetInitials(Buffer, sizeof(Buffer));

						sprintf(buff, getmsg(249), AplTab[i].item,
								Buffer, bn);
						break;
						}

					case APL_PASSWORD:
						{
						label Buffer;
						CurrentUser->GetPassword(Buffer, sizeof(Buffer));

						sprintf(buff, getmsg(249), AplTab[i].item,
								Buffer, bn);
						break;
						}

					case APL_SURNAME:
						{
						label Buffer;
						CurrentUser->GetSurname(Buffer, sizeof(Buffer));

						sprintf(buff, getmsg(249), AplTab[i].item,
								Buffer, bn);
						break;
						}

					case APL_TITLE:
						{
						label Buffer;
						CurrentUser->GetTitle(Buffer, sizeof(Buffer));

						sprintf(buff, getmsg(249), AplTab[i].item,
								Buffer, bn);
						break;
						}

					case APL_COLUMNS:
						{
						sprintf(buff, getmsg(246), AplTab[i].item,
								CurrentUser->GetWidth(), bn);
						break;
						}

					case APL_NULLS:
						{
						sprintf(buff, getmsg(246), AplTab[i].item,
								CurrentUser->GetNulls(), bn);
						break;
						}

					case APL_LINES:
						{
						sprintf(buff, getmsg(246), AplTab[i].item,
								CurrentUser->GetLinesPerScreen(), bn);
						break;
						}

					case APL_DLMULT:
						{
						if (cfg.accounting)
							{
							sprintf(buff, getmsg(246), AplTab[i].item,
									CurrentUserAccount->GetDownloadMultiplier(),
									bn);
							}

						break;
						}

					case APL_ULMULT:
						{
						if (cfg.accounting)
							{
							sprintf(buff, getmsg(246), AplTab[i].item,
									CurrentUserAccount->GetUploadMultiplier(),
									bn);
							}

						break;
						}

					case APL_EFF_BAUD:
						{
						int MS = CommPort->GetModemSpeed();

						if (MS > 1)
							{
							MS--;
							}

						sprintf(buff, getmsg(246), AplTab[i].item, MS, bn);
						break;
						}

					case APL_BAUD:
						{
						int PS = CommPort->GetSpeed();

						if (PS > 1)
							{
							PS--;
							}

						sprintf(buff, getmsg(246), AplTab[i].item, PS, bn);
						break;
						}

					case APL_PORTRATE:
						{
						sprintf(buff, getmsg(245), AplTab[i].item,
								bauds[CommPort->GetSpeed()], bn);
						break;
						}

					case APL_CONNRATE:
						{
						sprintf(buff, getmsg(245), AplTab[i].item,
								connectbauds[CommPort->GetModemSpeed()], bn);
						break;
						}

					case APL_TRANSMIT:
						{
						sprintf(buff, getmsg(245), AplTab[i].item,
								CommPort->GetTransmitted(), bn);
						break;
						}

					case APL_RECEIVE:
						{
						sprintf(buff, getmsg(245), AplTab[i].item,
								CommPort->GetReceived(), bn);
						break;
						}

					case APL_ECHO:
						{
						sprintf(buff, getmsg(246), AplTab[i].item,
								!StatusLine.IsFullScreen(), bn);
						break;
						}

					case APL_ULISTED:
						{
						sprintf(buff, getmsg(246), AplTab[i].item,
								CurrentUser->IsUnlisted(), bn);
						break;
						}

					case APL_UPPERCASE:
						{
						sprintf(buff, getmsg(246), AplTab[i].item,
								CurrentUser->IsUpperOnly(), bn);
						break;
						}

					case APL_LINEFEED:
						{
						sprintf(buff, getmsg(246), AplTab[i].item,
								CurrentUser->IsLinefeeds(), bn);
						break;
						}

					case APL_EXPERT:
						{
						sprintf(buff, getmsg(246), AplTab[i].item,
								CurrentUser->IsExpert(), bn);
						break;
						}

					case APL_TWIT:
						{
						sprintf(buff, getmsg(246), AplTab[i].item,
								CurrentUser->IsProblem(), bn);
						break;
						}

					case APL_AIDE:
						{
						sprintf(buff, getmsg(246), AplTab[i].item,
								CurrentUser->IsAide(), bn);
						break;
						}

					case APL_SYSOP:
						{
						sprintf(buff, getmsg(246), AplTab[i].item,
								CurrentUser->IsSysop(), bn);
						break;
						}

					case APL_CREDITS:
						{
						if (cfg.accounting && CurrentUser->IsAccounting())
							{
							sprintf(buff, getmsg(245), AplTab[i].item,
									CurrentUser->GetCredits() / 60, bn);
							}
						else
							{
							sprintf(buff, getmsg(245), AplTab[i].item,
									LONG_MAX, bn);
							}

						break;
						}

					case APL_MDATA:
						{
						if (onConsole)
							{
							sprintf(buff, getmsg(244), AplTab[i].item, bn);
							}
						else
							{
							sprintf(buff, getmsg(246), AplTab[i].item,
									cfg.mdata, bn);
							}

						break;
						}

					case APL_HALL:
						{
						label Buffer;
						HallData[thisHall].GetName(Buffer, sizeof(Buffer));

						sprintf(buff, getmsg(249), AplTab[i].item,
								Buffer, bn);
						break;
						}

					case APL_ROOM:
						{
						label Buffer;
						RoomTab[thisRoom].GetName(Buffer, sizeof(Buffer));

						sprintf(buff, getmsg(249), AplTab[i].item,
								Buffer, bn);
						break;
						}

					case APL_ACCOUNTING:
						{
						sprintf(buff, getmsg(246), AplTab[i].item,
								cfg.accounting &&
								!CurrentUser->IsAccounting(), bn);
						break;
						}

					case APL_PERMANENT:
						{
						sprintf(buff, getmsg(246), AplTab[i].item,
								CurrentUser->IsPermanent(), bn);
						break;
						}

					case APL_VERIFIED:
						{
						sprintf(buff, getmsg(246), AplTab[i].item,
								CurrentUser->IsVerified(), bn);
						break;
						}

					case APL_NETUSER:
						{
						sprintf(buff, getmsg(246), AplTab[i].item,
								CurrentUser->IsNetUser(), bn);
						break;
						}

					case APL_NOMAIL:
						{
						sprintf(buff, getmsg(246), AplTab[i].item,
								!CurrentUser->IsMail(), bn);
						break;
						}

					case APL_CHAT:
						{
						if (CurrentUser->IsChat())
							{
							sprintf(buff, getmsg(246), AplTab[i].item,
									cfg.noChat, bn);
							}
						else
							{
							sprintf(buff, getmsg(246), AplTab[i].item, 1, bn);
							}

						break;
						}

					case APL_BELLS:
						{
						sprintf(buff, getmsg(246), AplTab[i].item,
								cfg.noBells, bn);
						break;
						}

					default:
						{
						buff[0] = 0;
						break;
						}
					}
				break;
				}

			default:
				{
				buff[0] = 0;
				break;
				}
			}

		if (strlen(buff) > 1)
			{
			fputs(buff, fd);
			}
		}

	fprintf(fd, getmsg(212), APL_END, bn);

	fclose(fd);

	writeCallinfo();
	writeChain();
	writeDoorfileSr();
	writeDoorSys();
	writeDorinfo();
	writePcboard();
	writeTribbs();
	writeUserinfo();
	}


// --------------------------------------------------------------------------
// readAplFile(): Reads input.apl from disk.

void TERMWINDOWMEMBER readAplFile(void)
	{
	Message *Msg = new Message;

	if (Msg)
		{
		FILE *fd;
		char FullFileName[128];

		Msg->ClearHeader();
		Msg->SetAuthor(cfg.nodeTitle);
		Msg->SetRoomNumber(thisRoom);

		sprintf(FullFileName, sbs, cfg.aplpath, inputApl);

		if ((fd = fopen(FullFileName, FO_R)) != NULL)
			{
			int item;

			do
				{
				item = fgetc(fd);

				if (feof(fd))
					{
					break;
					}

				char InputLine[200];
				fgets(InputLine, 198, fd);
				InputLine[strlen(InputLine) - 1] = 0;

				char InputAsChar = (char) atoi(InputLine);
				int InputAsInt = atoi(InputLine);
				long InputAsLong = atol(InputLine);

				Bool found = FALSE;

				int i;
				for (i = 0; AplTab[i].item != APL_END; i++)
					{
					if (AplTab[i].item == item && AplTab[i].keep)
						{
						found = TRUE;

						switch (AplTab[i].type)
							{
							case TYP_STR:
								{
								strncpy((char *) AplTab[i].variable, InputLine,
										AplTab[i].length);

								((char *) AplTab[i].variable)[AplTab[i].length - 1] = 0;
								break;
								}

							case TYP_BOOL:
							case TYP_CHAR:
								{
								*((char *) AplTab[i].variable) = InputAsChar;
								break;
								}

							case TYP_INT:
								{
								*((int *) AplTab[i].variable) = InputAsInt;
								break;
								}

							case TYP_LONG:
								{
								*((long *) AplTab[i].variable) = InputAsLong;
								break;
								}

							case TYP_GRP:
								{
								label Buffer;
								CurrentUser->GetName(Buffer, sizeof(Buffer));
								groupfunc(InputLine, Buffer, item == GRP_ADD);
								break;
								}

							case TYP_OTHER:
								{
								switch (AplTab[i].item)
									{
									case APL_SURNAME:
										{
										CurrentUser->SetSurname(InputLine);
										break;
										}

									case APL_TITLE:
										{
										CurrentUser->SetTitle(InputLine);
										break;
										}

									case APL_NULLS:
										{
										CurrentUser->SetNulls(InputAsInt);
										break;
										}

									case APL_COLUMNS:
										{
										CurrentUser->SetWidth(InputAsInt);
										break;
										}

									case APL_LINES:
										{
										CurrentUser->SetLinesPerScreen(InputAsInt);
										break;
										}

									case APL_ULISTED:
										{
										CurrentUser->SetUnlisted(InputAsInt);
										break;
										}

									case APL_UPPERCASE:
										{
										CurrentUser->SetUpperOnly(InputAsInt);
										break;
										}

									case APL_LINEFEED:
										{
										CurrentUser->SetLinefeeds(InputAsInt);
										break;
										}

									case APL_DLMULT:
										{
										if (cfg.accounting)
											{
											CurrentUserAccount->SetDownloadMultiplier(InputAsInt);
											}

										break;
										}

									case APL_ULMULT:
										{
										if (cfg.accounting)
											{
											CurrentUserAccount->SetUploadMultiplier(InputAsInt);
											}

										break;
										}

									case APL_EXPERT:
										{
										CurrentUser->SetExpert(InputAsInt);
										break;
										}

									case APL_TWIT:
										{
										CurrentUser->SetProblem(InputAsInt);
										break;
										}

									case APL_AIDE:
										{
										CurrentUser->SetAide(InputAsInt);
										break;
										}

									case APL_SYSOP:
										{
										CurrentUser->SetSysop(InputAsInt);
										break;
										}

									case APL_CREDITS:
										{
										CurrentUser->SetCredits(InputAsLong * 60);
										break;
										}

									case APL_HALL:
										{
										if (!HallData[thisHall].IsSameName(
												InputLine))
											{
											h_slot HallSlot;

											HallSlot = hallexists(InputLine);

											if (HallSlot != CERROR)
												{
												mPrintfCR(getmsg(155),
														cfg.Uhall_nym,
														InputLine);
												thisHall = HallSlot;
												}
											else
												{
												cPrintf(getmsg(157),
														cfg.Lhall_nym,
														InputLine);
												doccr();
												}
											}

										break;
										}

									case APL_ROOM:
										{
										if (!RoomTab[thisRoom].IsSameName(
												InputLine))
											{
											gotoRoom(InputLine, FALSE, FALSE,
													FALSE);
											}

										break;
										}

									case APL_PERMANENT:
										{
										CurrentUser->SetPermanent(InputAsInt);
										break;
										}

									case APL_VERIFIED:
										{
										CurrentUser->SetVerified(InputAsInt);
										break;
										}

									case APL_NETUSER:
										{
										CurrentUser->SetNetUser(InputAsInt);
										break;
										}

									case APL_NOMAIL:
										{
										CurrentUser->SetMail(!InputAsInt);
										break;
										}

									case APL_CHAT:
										{
										cfg.noChat = (Bool) InputAsInt;
										break;
										}

									case APL_BELLS:
										{
										cfg.noBells = (Bool) InputAsInt;
										break;
										}

									default:
										{
										mPrintfCR(getmsg(160), item, InputLine);
										break;
										}
									}

								break;
								}
							}
						}
					}

				if (!found)
					{
					found = TRUE;

					switch (item)
						{
						case MSG_NAME:
							{
							Msg->SetAuthor(InputLine);
							break;
							}

						case MSG_TO:
							{
							Msg->SetToUser(InputLine);
							break;
							}

						case MSG_GROUP:
							{
							Msg->SetGroup(InputLine);
							break;
							}

						case MSG_ROOM:
							{
							r_slot roomno;

							if ((roomno = RoomExists(InputLine)) == CERROR)
								{
								cPrintf(getmsg(157), cfg.Lroom_nym,
										InputLine);
								doccr();
								}
							else
								{
								Msg->SetRoomNumber(roomno);
								}

							break;
							}

						default:
							{
							doCR();
							found = FALSE;
							break;
							}
						}
					}

				if (!found && AplTab[i].item != APL_END)
					{
					mPrintfCR(getmsg(160), item, InputLine);
					}
				} while (item != APL_END && !feof(fd));

			fclose(fd);
			}

		StatusLine.Update(WC_TWp);

		sprintf(FullFileName, sbs, cfg.aplpath, messageApl);
		if ((fd = fopen(FullFileName, FO_RB)) != NULL)
			{
			GetFileMessage(fd, Msg->GetTextPointer(), MAXTEXT);
			fclose(fd);

			putAndNoteMessage(Msg, TRUE);
			}

		sprintf(FullFileName, sbs, cfg.aplpath, readmeApl);
		if (filexists(FullFileName))
			{
			dumpf(FullFileName, FALSE, FALSE);
			doCR();
			}

		sprintf(FullFileName, sbs, cfg.aplpath, consoleApl);
		if (filexists(FullFileName))
			{
			dumpfc(FullFileName, FALSE);
			doccr();
			}

		eraseAplFiles();

		delete Msg;
		}
	else
		{
		OutOfMemory(71);
		}
	}


// shorthand for writing to file
static void wrtD(int x, FILE *fd)
	{
	fprintf(fd, getmsg(556), (x), bn);
	}

static void wrtLD(long x, FILE *fd)
	{
	fprintf(fd, getmsg(555), (x), bn);
	}

static void wrtU(uint x, FILE *fd)
	{
	fprintf(fd, getmsg(510), x, bn);
	}

static void wrtLU(ulong x, FILE *fd)
	{
	fprintf(fd, getmsg(225), x, bn);
	}

static void wrtC(char x, FILE *fd)
	{
	fprintf(fd, getmsg(212), x, bn);
	}

static void wrtS(const char *x, FILE *fd)
	{
	fprintf(fd, getmsg(472), deansi(x), bn);
	}

static void wrt(FILE *fd)
	{
	fprintf(fd, bn);
	}

static void wrtDate(time_t x, FILE *fd)
	{
	char buff[80];

	strftime(buff, sizeof(buff) - 1, getmsg(243), x);
	fprintf(fd, getmsg(472), buff, bn);
	}

static void wrtTime(time_t x, FILE *fd)
	{
	char buff[80];

	strftime(buff, sizeof(buff) - 1, getmsg(372), x);
	fprintf(fd, getmsg(472), buff, bn);
	}

void TERMWINDOWMEMBER wrtLvl(int x, FILE *fd)
	{
	int secLevel;

	if (CurrentUser->IsSysop()) 		{	secLevel = (x / 10 * 10);	}
	else if (CurrentUser->IsAide()) 	{	secLevel = (x / 10 * 8);	}
	else if (CurrentUser->IsProblem())	{	secLevel = (5); 			}
	else								{	secLevel = (x / 10 * 3);	}

	fprintf(fd, getmsg(556), secLevel, bn);
	}

static void wrtSBS(const char *x, FILE *fd)
	{
	fprintf(fd, sbs, x, bn);
	}

void TERMWINDOWMEMBER wrtUserSpd(Bool x, FILE *fd)
	{
	fprintf(fd, getmsg(555), UserSpeed(x), bn);
	}

void TERMWINDOWMEMBER wrtAddr(FILE *fd)
	{
	label Buffer;

	if (!*(CurrentUser->GetMailAddr3(Buffer, sizeof(Buffer))))
		{
		if (!*(CurrentUser->GetMailAddr2(Buffer, sizeof(Buffer))))
			{
			CurrentUser->GetMailAddr1(Buffer, sizeof(Buffer));
			}
		}

	fprintf(fd, getmsg(472), Buffer, bn);
	}

void TERMWINDOWMEMBER wrtCredits(int x, FILE *fd)
	{
	fprintf(fd, getmsg(556), Credits(x), bn);
	}


// --------------------------------------------------------------------------
// writeCallinfo(): Writes CALLINFO.BBS (mostly undefined).

void TERMWINDOWMEMBER writeCallinfo(void)
	{
	char buff[80];
	FILE *fd;
	int bpnum;
	const LogEntry * const cu = CurrentUser;		// wow, no changes

	sprintf(buff, sbs, cfg.aplpath, callinfoBbs);
	if ((fd = fopen(buff, FO_W)) == NULL)
		{
		mPrintf(getmsg(8), callinfoBbs);
		return;
		}

	label Buffer;
	wrtS(cu->GetName(Buffer, sizeof(Buffer)), fd);	// user name
	switch (CommPort->GetSpeed())
		{
		case PS_300:	bpnum = 1;	break;	// 300
		case PS_1200:	bpnum = 2;	break;	// 1200
		case PS_2400:	bpnum = 0;	break;	// 2400
		case PS_9600:	bpnum = 3;	break;	// 9600
		default:	bpnum = 5;	break;	// console or other
		}

	wrtD((CommPort->HaveConnection()) ? bpnum : 5, fd);// 5 on console
	wrtAddr(fd);									// City
	wrtLvl(100, fd);								// security level, 100=sysop
	wrtCredits(60, fd); 							// time left in minutes
	wrtS(TermCap->IsColor() ? getmsg(304) : getmsg(319), fd);
	wrtS(cu->GetPassword(Buffer, sizeof(Buffer)), fd);// password
	wrtD(ThisLog, fd);								// Userlog number
	wrtLD((time(NULL) - LogTime()) / 60L, fd);		// Minutes used this call
	wrt(fd);										// ??? "01:23"
	wrt(fd);										// ??? "01:23 01/02/90"
	wrt(fd);										// ??? "ABCDEFGH"
	wrt(fd);										// ??? "0"
	wrt(fd);										// ??? "99"
	wrt(fd);										// ??? "0"
	wrt(fd);										// ??? "9999"
	wrtS(cu->GetPhoneNumber(Buffer, sizeof(Buffer)), fd);// phone number
	wrt(fd);										// ??? "01/01/90 02:34\n"
	wrtS(cu->IsExpert() ? getmsg(320) : getmsg(356), fd); // "EXPERT" or "NOVICE"

	const protocols *theProt = GetProtocolByKey(cu->GetDefaultProtocol());
	wrtS(theProt ? theProt->name : ns, fd); 		// Protocol name (as a word)

	wrt(fd);										// ??? "01/01/90\n"
	wrtLD(cu->GetLogins(), fd); 					// Total times on
	wrtD(cu->GetLinesPerScreen(), fd);				// user rows
	wrtLD(cu->GetRoomNewPointer(thisRoom), fd); // Last msg read
	wrtU(cu->GetUL_Num(), fd);						// total uploads
	wrtU(cu->GetDL_Num(), fd);						// total downloads
	wrtS(getmsg(375), fd);							// How fucking stupid!!!
	wrtS(onConsole ? getmsg(376) : getmsg(378), fd);// where is user
	fprintf(fd, getmsg(323), cfg.mdata, bn, ns);// port
	wrtD(bpnum, fd);						// Cit-86 uses wrtD(connectspeed)
	wrt(fd);								// ??? "FALSE"
	wrtS(getmsg(395), fd);					// ??? Another pretty stupid thing
	wrt(fd);								// "01/02/94 01:20\n"
	wrtD(0, fd);							// task number (or 0 for no task #)
	wrtD(1, fd);							// !!! Door number
	fclose(fd);
	}


// --------------------------------------------------------------------------
// writeChain(): Writes CHAIN.TXT.
//
// ####################################################################
// ####################################################################
// ####################################################################

/*
Article 20673 of alt.bbs:
From: Paranoid@f102.n514.z17.mtlnet.org (Paranoid)
Path: news.u.washington.edu!netnews.nwnet.net!usenet.coe.montana.edu!caen!usenet.cis.ufl.edu!eng.ufl.edu!spool.mu.edu!uwm.edu!cs.utexas.edu!uun
Newsgroups: alt.bbs
Subject: Chain.txt
Message-ID: <749240331.AA06352@mtlnet.org>
Date: Tue, 28 Sep 1993 10:27:00 -0500
X-FTN-To: Evan Johnson
Lines: 51

HI! well I'v got the following, hope it will help you out!!

					  CHAIN.TXT Definition File
							  by MrBill

----------------------------CHAIN.TXT--------------------------------
1								  User number
MRBILL							  User alias
Bill							  User real name
								  User callsign (HAM radio)
21								  User age
M								  User sex
  16097.00						  User gold
05/19/89						  User last logon date
80								  User colums
25								  User width
255 							  User security level (0-255)
1								  1 if Co-SysOp, 0 if not
1								  1 if SysOp, 0 if not
1								  1 if ANSI, 0 if not
0								  1 if at remote, 0 if local console
   2225.78						  User number of seconds left till logoff
F:\WWIV\GFILES\ 				  System GFILES directory (gen. txt files)
F:\WWIV\DATA\					  System DATA directory
890519.LOG						  System log of the day
2400							  User baud rate
2								  System com port
MrBill's Abode (the original)     System name
The incredible inedible MrBill	  System SysOp
83680							  Time user logged on/# of secs. from midn.
554 							  User number of seconds on system so far
5050							  User number of uploaded k
22								  User number of uploads
42								  User amount of downloaded k
1								  User number of downloads
8N1 							  User parity
2400							  Com port baud rate
7400							  WWIVnet node number
------------------------------EOF------------------------------------
You can Reach Mr. Bill at:

WWIVnet Address 1 @ 7300 [MrBill's Abode 703-369-6140]
Compuserve Address 71001,1101

GEnie Address, oh hell, i forgot!
*/

// ####################################################################
// ####################################################################
// ####################################################################

void TERMWINDOWMEMBER writeChain(void)
	{
	FILE *fd;
	char buff[80];
	const LogEntry * const cu = CurrentUser;	// wow, no changes
	struct tm *tmnow;	// These are for a seconds
	long timenow;		//	after midnight calculation

	sprintf(buff, sbs, cfg.aplpath, chainTxt);
	if ((fd = fopen(buff, FO_W)) == NULL)
		{
		mPrintf(getmsg(8), chainTxt);
		return;
		}

	wrtD(ThisLog, fd);				// User number
	label Buffer;
	wrtS(cu->GetName(Buffer, sizeof(Buffer)), fd);		// User alias
	wrtS(cu->GetRealName(Buffer, sizeof(Buffer)), fd);	// User real name
	wrtS(ns, fd);						// !!! User call sign (ham radio)
	if (cu->GetBirthDate())
		{
		struct time tme;
		struct date datethen, datenow;

		unixtodos(cu->GetBirthDate(), &datethen, &tme);
		unixtodos(time(NULL), &datenow, &tme);

		int Years = datenow.da_year - datethen.da_year;

		if (	// not yet birth month
				(datenow.da_mon < datethen.da_mon) ||

				(
					// is birthmonth
					(datenow.da_mon == datethen.da_mon) &&

					// but not birthday
					(datenow.da_day < datethen.da_day)
				)
			)
			{
			// Then we are not quite as old as we though.
			Years--;
			}

		wrtD(Years, fd);
		}
	else
		{
		wrtD(21, fd);					// Don't have BD? Call them 21.
		}
	wrtC(cu->GetSex() == SEX_MALE ? 'M' : 'F', fd); // User sex
	wrtCredits(1, fd);					// User gold, "  16097.00\n"
	wrtDate(cu->GetCallTime(), fd); 	// User last logon date, 05/19/89
	wrtD(cu->GetWidth(), fd);			// User colums [sic]
	wrtD(cu->GetLinesPerScreen(), fd);	// User rows
	wrtLvl(250, fd);					// User security level (0-255)
	wrtD(cu->IsAide(), fd); 			// 1 if Aide, 0 if not
	wrtD(cu->IsSysop(), fd);			// 1 if SysOp, 0 if not
	wrtD(TermCap->IsANSI(), fd);	// 1 if ANSI, 0 if not
	wrtD(!onConsole, fd);				// 1 if at remote, 0 if local console
	wrtCredits(1, fd);					// #secs till logoff, "   2225.78"
		// And yes, I too find it funny that they have fractional seconds.
	wrtSBS(cfg.dirpath, fd);			// System GFILES directory
									//	general text files, "F:\WWIV\GFILES\"
	wrtSBS(cfg.homepath, fd);			// System DATA directory
	wrtS(getmsg(418), fd);				// System log of the day (÷cfg.trapfile)
	wrtUserSpd(TRUE, fd);				// User baud rate
	wrtD(cfg.mdata, fd);				// System com port
	wrtS(cfg.nodeTitle, fd);			// System name
	wrtS(cfg.sysop, fd);				// System SysOp
	///////////////////// Newer version of WWIV //////////////////////////////
	timenow = LogTime();
	tmnow = localtime(&timenow);				//	express as secs after midn.
	timenow = tmnow->tm_hour * 3600L + tmnow->tm_min * 60L + tmnow->tm_sec;
	wrtLD(timenow, fd); 							// when logged in
	wrtLD(time(NULL) - LogTime(), fd);				// # secs on system so far
	wrtLU(cu->GetUL_Bytes() / 1024L, fd);			// Number of uploaded K
	wrtU(cu->GetUL_Num(), fd);						// Number of uploads
	wrtLU(cu->GetDL_Bytes() / 1024L, fd);			// Number of downloaded K
	wrtU(cu->GetDL_Num(), fd);						// Number of downloads
	wrtS(getmsg(479), fd);							// User parity
	wrtLD(bauds[CommPort->GetSpeed()], fd); // Com port baud rate
	wrtD(0, fd);									// WWIVnet node number
	fclose(fd);
	}


// --------------------------------------------------------------------------
// writeDoorfileSr(): Writes DOORFILE.SR.
//
// ####################################################################
// ####################################################################
// ####################################################################
/*
[from 93Oct20 SRDOOR.DOC documentation]

		 You can also write your own program to convert your favorite door
		 file to DOORFILE.SR.  The DOORFILE.SR format is:

			(line 1):	Complete name or handle of user
			(line 2):	ANSI status:  1 = yes, 0 = no, -1 = don't know
			(line 3):	IBM Graphic characters:  1 = yes, 0 = no, -1 = unknown
			(line 4):	Page length of screen, in lines.  Assume 25 if unknown
			(line 5):	Baud Rate:	300, 1200, 2400, 9600, 19200, etc.
			(line 6):	Com Port:  1, 2, 3, or 4.
			(line 7):	Time Limit:  (in minutes); -1 if unknown.
			(line 8):	Real name (the same as line 1 if not known)
*/

// ####################################################################
// ####################################################################
// ####################################################################

void TERMWINDOWMEMBER writeDoorfileSr(void)
	{
	FILE *fd;
	char buff[80];
	const LogEntry * const cu = CurrentUser;	// wow, no changes

	const int Length = cu->GetLinesPerScreen();

	sprintf(buff, sbs, cfg.aplpath, doorfileSr);
	if ((fd = fopen(buff, FO_W)) == NULL)
		{
		mPrintf(getmsg(8), doorfileSr);
		return;
		}

	label Buffer;
	wrtS(cu->GetName(Buffer, sizeof(Buffer)), fd);// Complete name or handle of user
	wrtD(TermCap->IsANSI(), fd);	// ANSI: 1=yes, 0=no, -1=unknown
	wrtD(TermCap->IsIBMExtended(), fd);// IBM Graphics: 1=yes, 0=no, -1=?
	wrtD(Length ? 25 : Length, fd); 	// Page length of screen, 25 if ?
	wrtLD(bauds[CommPort->GetSpeed()], fd);// Baud rate (9600, 19200,etc)
	wrtD(onConsole ? 0 : cfg.mdata, fd);// Com Port: 1, 2, 3, or 4
//	wrtD(cfg.mdata, fd);				// Com Port: 1, 2, 3, or 4
	wrtCredits(60, fd); 				// Time Limit (in minutes), -1=?
	label Buffer2;						// User real name (or line 1 if ?)
	cu->GetRealName(Buffer, sizeof(Buffer));
	wrtS(*Buffer ? Buffer : cu->GetName(Buffer2, sizeof(Buffer2)), fd);
	fclose(fd);
	}


// --------------------------------------------------------------------------
// writeDoorSys(): Writes DOOR.SYS. 	[Jim Harrer can't program]
//
// ####################################################################
// ####################################################################
// ####################################################################
/*
		 03/14/88 - Submitted by the "Limited Release Software Group"

Updated: 03/21/88
Updated: 10/22/88  KG
Updated: 07/07/90  KG, to add additional info for Raymond Clements
Updated: 07/14/91  Note to developers added by Jim Harrer/Mustang Software.

=============================================================================

Sample DOOR.SYS file to be used as a STANDARD for ALL Bulletin Board Systems.

=============================================================================

  This file format has been laid out and will be presented to ALL BBS authors
in an attempt to establish a national standard for door program applications.

BBS AUTHORS: I urge you to consider this STANDARD with respect for your 3rd
			 party support authors. Come on guys, make life EASY for us.
																(BIG GRIN)

=============================================================================

  I have tried to include EVERYTHING a 3rd party DOOR author would need to
provide the SYSOPS with the look and feel they want from an online program
while at the same time keeping things plain and simple to ease the problems
of interfacing programs with a system.

This type of format is intended to be a "read-only" type of file
(although it could be written again) because most BBS's have some sort
of other file they use for re-entering the system with (PCBoard uses
PCBOARD.SYS) which is harder to read in, and changes frequently.
This file format, once established, wouldn't need to be changed EVERYTIME
a new version of the BBS software gets released and this would also
ease the demand for more info being made available in the BBS Re-Entry
file thus making life easier for both parties. If there is something
found in the following format that your BBS software may not be currently
supporting, a DEFAULT value has been suggested.

----------
KG, Note:

   GAP DOES treat this file as Read/Write since GAP does not use
or have any use for a Door File. GAP will re-read the DOOR.SYS upon
return from a door. But not all fields are re-read. Only those fields
that are marked with an '*' will be read by GAP when a caller returns
from a door.

   When reading this file, you should not go into an EOF loop. In
other words, you should not read until you encounter End Of File.
Instead, you should read what you KNOW is there. This allows for
additions to the end of the file without forcing all programs to be
re-compiled.

----------
Note from Jim Harrer:

   Beginning with WILDCAT! release 3, we will begin supporting this DOOR.SYS
Standard by creating this file in the \WC30\WCWORK\NODEx (x = Node Number)
directory.	We will NOT read this file back in.  If DOORS wish to change
any of the callers variables, then the door author should refer to our
USERINFO.DAT standard.

   Mustang Software will not support any future changes to DOOR.SYS.  Other
BBS authors must regard this DOOR.SYS as a "Standard" and not change or
add to it in any way.  DOOR authors can be assured we will not change
DOOR.SYS in the future.  Any changes we need to make will be done to our
USERINFO.DAT standard.

   I hope other BBS authors will join us in agreement of not making any
additional changes to DOOR.SYS in the future.

----------
DOOR.SYS
--------
COM1:			  <-- Comm Port - COM0: = LOCAL MODE
2400			  <-- Baud Rate - 300 to 38400
8				  <-- Parity - 7 or 8
1				  <-- Node Number - 1 to 99 				   (Default to 1)
19200			  <-- DTE Rate. Actual BPS rate to use. (kg)
Y				  <-- Screen Display - Y=On  N=Off			   (Default to Y)
Y				  <-- Printer Toggle - Y=On  N=Off			   (Default to Y)
Y				  <-- Page Bell 	 - Y=On  N=Off			   (Default to Y)
Y				  <-- Caller Alarm	 - Y=On  N=Off			   (Default to Y)
Rick Greer		  <-- User Full Name
Lewisville, Tx.   <-- Calling From
214 221-7814	  <-- Home Phone
214 221-7814	  <-- Work/Data Phone
PASSWORD		  <-- Password
110 			 *<-- Security Level
1456			  <-- Total Times On
03/14/88		  <-- Last Date Called
7560			  <-- Seconds Remaining THIS call (for those that particular)
126 			  <-- Minutes Remaining THIS call
GR				  <-- Graphics Mode - GR=Graph, NG=Non-Graph, 7E=7,E Caller
23				  <-- Page Length
Y				  <-- User Mode - Y = Expert, N = Novice
1,2,3,4,5,6,7	  <-- Conferences/Forums Registered In	(ABCDEFG)
7				  <-- Conference Exited To DOOR From	(G)
01/01/99		  <-- User Expiration Date				(mm/dd/yy)
1				  <-- User File's Record Number
Y				  <-- Default Protocol - X, C, Y, G, I, N, Etc.
0				 *<-- Total Uploads
0				 *<-- Total Downloads
0				 *<-- Daily Download "K" Total
999999			  <-- Daily Download Max. "K" Limit
10/22/88		  <-- Caller's Birthdate                              (kg)
G:\GAP\MAIN 	  <-- Path to the MAIN directory (where User File is) (kg)
G:\GAP\GEN		  <-- Path to the GEN directory 					  (kg)
Michael 		  <-- Sysop's Name (name BBS refers to Sysop as)      (kg)
Stud			  <-- Alias name									  (rc)
00:05			  <-- Event time						(hh:mm) 	  (rc)
Y				  <-- If its an error correcting connection (Y/N)	  (rc)
N				  <-- ANSI supported & caller using NG mode (Y/N)	  (rc)
Y				  <-- Use Record Locking					(Y/N)	  (rc)
14				  <-- BBS Default Color (Standard IBM color code, ie, 1-15) (rc)
10				 *<-- Time Credits In Minutes (positive/negative)	  (rc)
07/07/90		  <-- Last New Files Scan Date			(mm/dd/yy)	  (rc)
14:32			  <-- Time of This Call 				(hh:mm) 	  (rc)
07:30			  <-- Time of Last Call 				(hh:mm) 	  (rc)
6				  <-- Maximum daily files available 				  (rc)
3				 *<-- Files d/led so far today						  (rc)
23456			 *<-- Total "K" Bytes Uploaded                        (rc)
76329			 *<-- Total "K" Bytes Downloaded                      (rc)
A File Sucker	  <-- User Comment									  (rc)
10				  <-- Total Doors Opened							  (rc)
10283			  <-- Total Messages Left							  (rc)


  *****  Each line is STRAIGHT ASCII TEXT with a CR/LF at the end.	*****

  Lines marked with an '*' will be re-read in GAP when a caller
  returns from a door.

  Rick Greer, the author of this Universal Specification, is no
  where to be found!
*/
// ####################################################################
// ####################################################################
// ####################################################################

void TERMWINDOWMEMBER writeDoorSys(void)	// original by Rick Greer, 3/14/88
	{
	FILE *fd;
	char buff[80];
	const LogEntry * const cu = CurrentUser;		// wow, no changes

	sprintf(buff, sbs, cfg.aplpath, doorSys);
	if ((fd = fopen(buff, FO_W)) == NULL)
		{
		mPrintf(getmsg(8), doorSys);
		return;
		}

	fprintf(fd, getmsg(323),
			onConsole ? 0 : cfg.mdata,
			getmsg(241), bn);				// COMM port, 0=local

	wrtUserSpd(TRUE, fd);					// User baud rate
	wrtD(8, fd);							// Parity [sic] - 7 or 8
	wrtD(1, fd);							// Node Number - 1 to 99, default=1
	wrtLD(bauds[CommPort->GetSpeed()], fd);// DTE rate or "N" if not locked
	wrtC(ScreenSaver.IsOn() ? 'N' : 'Y', fd);// Screen Display
	wrtC(OC.Printing ? 'Y' : 'N', fd);  // Printer Toggle
	wrtC(cfg.noBells == 2 ? 'N' : 'Y', fd); // Page bell
	wrtC('Y', fd);                          // ??? Caller Alarm
	label Buffer;
	wrtS(cu->GetName(Buffer, sizeof(Buffer)), fd);				// User name
	wrtAddr(fd);							// Calling from
	wrtS(cu->GetPhoneNumber(Buffer, sizeof(Buffer)), fd);// Home Phone
	wrtS(cu->GetPhoneNumber(Buffer, sizeof(Buffer)), fd);// Work/Data Phone
	wrtS(cu->GetPassword(Buffer, sizeof(Buffer)), fd);// Password (should be CAPS)
	wrtLvl(100, fd);						// Security Level
	wrtLD(cu->GetLogins(), fd); 			// Total times on
	wrtDate(cu->GetCallTime(), fd); 		// Last Date Called
	wrtCredits(1, fd);						// Seconds remaining
	wrtCredits(60, fd); 					// Minutes remaining
	wrtS(TermCap->IsANSI() ? getmsg(480) : getmsg(481), fd);// or "7E" or "RIP"
	wrtD(cu->GetLinesPerScreen(), fd);		// Page Length
	wrtC(cu->IsExpert() ? 'Y' : 'N', fd);   // User mode: Y=Expert
	wrt(fd);								// Conferences/Forums Registered In
	wrtD(thisRoom, fd); 				// Conference Exited To DOOR From
	wrtS(getmsg(488), fd);					// !!! User Expiration Date
	wrtD(ThisLog, fd);					// User File's Record Number
	wrtC((char) toupper(cu->GetDefaultProtocol()), fd);// Default protocol
	wrtU(cu->GetUL_Num(), fd);				// Number of uploads
	wrtU(cu->GetDL_Num(), fd);				// Number of downloads
	wrtLU(0L, fd);							// Daily Download "K" Total
	wrtLU(999999L, fd); 					// Daily Download Max "K" Limit
	/////////////////////// Revised 10/22/88 by Kenny Gardner ////////////////
	/////////////////////// (note: TAG 2.6d stops at this pt) ////////////////
	wrtDate(cu->GetBirthDate(), fd);		// Caller's Birthdate
	wrtS(cfg.homepath, fd); 				// Path to the MAIN directory
	wrtS(cfg.dirpath, fd);					// Path to the GEN directory
	wrtS(cfg.sysop, fd);					// Sysop's Name
	////////// Revised 7/7/90 by Kenny Gardner for Raymond Clements //////////
	wrtS(cu->GetName(Buffer, sizeof(Buffer)), fd);// Alias name (seriously!)
	wrt(fd);								// Event time
	wrtC(CommPort->UsingCorrection ? 'Y' : 'N', fd);// Error correcting connection
	wrtC('N', fd);                          // If ANSI supported and using NG mode
	wrtC('N', fd);                          // Use Record Locking
	wrtD(cfg.attr & 0x15, fd);				// BBS default color (IBM code)
	wrtCredits(60, fd); 					// Time credits in minutes
	wrtDate(cu->GetCallTime(), fd); 		// !!! Last New Files Scan Date
	wrtTime(LogTime(), fd); 				// This calltime
	wrtTime(cu->GetCallTime(), fd); 		// Time of Last Call
	wrtD(99, fd);							// !!! Max daily files available
	wrtD(0, fd);							// !!! Files d/led so far today
	wrtLU(cu->GetUL_Bytes() / 1024L, fd);	// Total "K" Bytes Uploaded
	wrtLU(cu->GetDL_Bytes() / 1024L, fd);	// Total "K" Downloaded
	wrtS(cu->GetSignature(Buffer, sizeof(Buffer)), fd);// User comment (wow ! ! !)
	wrtD(0, fd);							// Total Doors Opened
	wrtLD(cu->GetPosted(), fd); 			// Total Messages Left
	fclose(fd);
	}


// --------------------------------------------------------------------------
// writeDorinfo(): Writes DORINFO1.DEF.

void TERMWINDOWMEMBER writeDorinfo(void)
	{
	FILE *fd;
	char buff[80];
	const LogEntry * const cu = CurrentUser;	// wow, no changes
	char *buffptr;

	sprintf(buff, sbs, cfg.aplpath, dorinfo1Def);
	if ((fd = fopen(buff, FO_W)) == NULL)
		{
		mPrintf(getmsg(8), dorinfo1Def);
		return;
		}

	wrtS(cfg.nodeTitle, fd);						// nodename
	strcpy(buff, cfg.sysop);
	stripansi(buff);

	buffptr = strchr(buff, ' ');
	if (buffptr)
		{
		*buffptr++ = 0;
		}	// tie string with nul
	else
		{
		buffptr = buff + strlen(buff);
		}	// *buffptr = 0
	wrtS(buff, fd); 								// Sysop f.name
	wrtS(buffptr, fd);								// Sysop l.name
	fprintf(fd, getmsg(323),
			onConsole ? 0 : cfg.mdata, bn, ns); 	// COMM port, 0=local
	fprintf(fd, getmsg(491), bauds[CommPort->GetSpeed()], bn); // Baud rate
	wrtD(0, fd);									// Not networked

	label Buffer;
	strcpy(buff, cu->GetName(Buffer, sizeof(Buffer)));
	stripansi(buff);
	buffptr = strchr(buff, ' ');
	if (buffptr)
		{
		*buffptr++ = 0;
		}	// tie string with nul
	else
		{
		buffptr = buff + strlen(buff);
		}	// *buffptr = 0
	wrtS(buff, fd); 							// Your f.name
	wrtS(buffptr, fd);							// Your l.name
	wrtAddr(fd);								// City
	wrtD(TermCap->IsANSI(), fd);				// 0=TTY, 1=ANSI, 2=AVATAR
	wrtLvl(100, fd);							// Security level
	wrtCredits(60, fd); 						// Minutes remaining
#ifdef WINCIT
	wrtD(0, fd); 								// -1 = using fossil
#else
	wrtD(*cdDesc ? -1 : 0, fd); 				// -1 = using fossil
#endif
	fclose(fd);
	}


// --------------------------------------------------------------------------
// writePcboard(): Writes PCBOARD.SYS.
//
// ####################################################################
// ####################################################################
// ####################################################################
/*
FeatherNet PRO! v1.01 Documentation 				   Page 44.15
ÉÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍ»
º CONTROL FILES - PCBOARD.SYS									º
ÈÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍ¼

This  is  the  exit to DOS information FeatherNet  PRO!  uses  to
maintain  a  caller's stats and the system's  configuration  upon
exit  to  DOS  when  running a DOOR, during  file  transfers,  or
viewing some archives

START POS	SAVED
& LENGTH	 AS 	  DESCRIPTION OF DATA
ÄÄÄÄÄÄÄÄÄ  ÄÄÄÄÄÄ	 ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
1, 2	   ASCII	 "-1" always used by FeatherNet PRO!
3, 2	   ASCII	 " 0" always used By FeatherNet PRO!
5, 2	   ASCII	 "-1" if page allowed or 0 if not.
7, 2	   ASCII	 User Number in Users file
9, 1	   ASCII	 "Y" if Expert or "N"if Not
10, 2	   ASCII	 "-1"  if  Error Correcting modem, "0" if not
12, 1	   ASCII	 "Y" if Graphics Mode or "N" if Not
13, 1	   ASCII	 "A" is always placed here by FeatherNet PRO!
14, 5	   ASCII	 The DTE speed or PC  to Modem baud rate
19, 5	   ASCII	 The connect baud rate:"300-38400" or "Local"
24, 2	   MKI$ 	 User's Record # in "USERS" file
26, 15	   ASCII	 User's FIRST Name padded with spaces
41, 12	   ASCII	 User's Password
53, 2	   MKI$ 	 Time user logged on in Mins: (60 x Hr)+Mins
55, 2	   MKI$ 	 User's Time on today in minutes
57, 5	   ASCII	 Time user logged on in HH:MM format.
					 Ex: "12:30"
62, 2	   MKI$ 	 Time user allowed today in minutes
64, 2	   ASCII	 Daily D/L Limit from pwrd file
66, 1	   Chr$ 	 Conference the user has last joined
67, 5	   Bitmap	 Areas user has been in
72, 5	   Bitmap	 Areas user has scanned
77, 2	   MKI$i	 An mki$(0) used by FeatherNet PRO!
79, 2	   MKI$ 	 Currently a value of 0 is here (MKI$(0))
81, 4	   ASCII	 4 Spaces are placed here
85, 25	   ASCII	 User's Full name placed here.
110, 2	   MKI$ 	 Number of minutes user has left today
112, 1	   chr$ 	 Node user is on (actual character)
113, 5	   ASCII	 Scheduled EVENT time
118, 2	   ASCII	 A "-1" if EVENT is active or a " 0"
120, 2	   ASCII	 " 0" is Placed here by FeatherNet PRO!
122, 4	   MKS$ 	 Time of day in secs format when user is on
126, 1	   ASCII	 The Com port this node uses (0 - 8)
127, 2	   ASCII	 Flag to let FNET PRO! know type of file xfer
129, 1	   CHAR 	 Ansi Detected Flag - Char[0] or Char[1]
130, 13    ASCII	 Unused by FeatherNet PRO! - SPACE filled
143, 2	   MKI$ 	 Last Area User was in (0 - 32766 possible)
145 	   BITMAP	 Not Currently Used by FeatherNet PRO!
*/
// ####################################################################
// ####################################################################
// ####################################################################

// first, a stupid function:
static void wrtMKI(int x, FILE *fd)
	{
	fprintf(fd, getmsg(238), (char) (x % 256), (char) (x / 256));
	}

void TERMWINDOWMEMBER writePcboard(void)
	{
	FILE *fd;
	char buff[80];
	const LogEntry * const cu = CurrentUser;		// wow, no changes
	struct tm *tmnow;	// These are for a minutes
	time_t timenow; 	//	after midnight calculation

	sprintf(buff, sbs, cfg.aplpath, pcboardSys);
	if ((fd = fopen(buff, FO_W)) == NULL)
		{
		mPrintf(getmsg(8), pcboardSys);
		return;
		}

	fprintf(fd, pct2d, -1); // good enough for FeatherNet Pro
	fprintf(fd, pct2d, 0);	// good enough for FeatherNet Pro
	fprintf(fd, pct2d, cfg.noBells == 2 ? 0 : -1);
	fprintf(fd, pct2d, -1); // always in user file, fucked doc above
	fprintf(fd, pctc, cu->IsExpert() ? 'Y' : 'N');      // User mode: Y=Expert
	fprintf(fd, pct2d, CommPort->UsingCorrection ? -1 : 0);	// Error correcting
	fprintf(fd, pctc, TermCap->IsANSI() ? 'Y' : 'N');// Y if ANSI, N if not
	fprintf(fd, pctc, 'A'); // good enough for FeatherNet Pro
	fprintf(fd, getmsg(201), (uint) bauds[CommPort->GetSpeed()]); // DTE, can't handle 115.2
	if (CommPort->HaveConnection()) fprintf(fd, getmsg(201), (int)UserSpeed(FALSE));
	else fprintf(fd, pcts, getmsg(492));			// ugh, stupid padding!
	wrtMKI(ThisLog, fd);

	label Buffer;
	strcpy(buff, deansi(cu->GetName(Buffer, sizeof(Buffer))));

	char *buffptr = strchr(buff, ' ');
	if (buffptr)
		{
		*buffptr = 0;
		}	// tie string with nul

	fprintf(fd, getmsg(199), buff); 				// padding is so stupid
	fprintf(fd, getmsg(196), cu->GetPassword(Buffer, sizeof(Buffer)));// padding is horrible

	timenow = LogTime();
	tmnow = localtime(&timenow);				//	express as mins after midn.
	timenow = tmnow->tm_hour * 60L + tmnow->tm_min;
	wrtMKI((int) timenow, fd);

	wrtMKI((int) ((time(NULL) - LogTime()) / 60L), fd); // # mins on system so far
	strftime(buff, sizeof(buff)-1, getmsg(372), (LogTime()));
	fprintf(fd, pcts, buff);					// Time user logged on
	wrtMKI((int) ((Credits(1) + time(NULL) - LogTime()) / 60L), fd);	// Time allowed
	fprintf(fd, getmsg(195), 99);	// ??? How many times can they d/l LOG.DAT?!?
	fprintf(fd, pctc, thisRoom);	// Conference last joined
	fprintf(fd, getmsg(194), 0xFF, 0xFF, 0xFF, 0xFF, 0xFF); // been in
	fprintf(fd, getmsg(194), 0xFF, 0xFF, 0xFF, 0xFF, 0xFF); // scanned
	wrtMKI(0, fd);				// good enough for FeatherNet Pro
	wrtMKI(0, fd);				// FeatherNet Pro thinks it's good enough
	fprintf(fd, getmsg(193), ns);	// perhaps this is good too?
	fprintf(fd, getmsg(192), deansi(cu->GetName(Buffer, sizeof(Buffer))));	// kinda near "full" name
	wrtMKI(Credits(60), fd);	// Time left (contrast with Time allowed)
	fprintf(fd, pctc, 0);		// wow, an actual character, go figure
	fprintf(fd, pcts, getmsg(191)); // we don't need no stinking event times
	fprintf(fd, pct2d, 0);		// and hopefully never will, they are evil
	fprintf(fd, pct2d, 0);		// good enough for FeatherNet Pro, -1 for DM5.8

	fprintf(fd, getmsg(190), 0,0,0,0);				// I refuse to research MKS$
	fprintf(fd, pctd, onConsole ? 0 : cfg.mdata);	// COM port, 0-8
	fprintf(fd, pctc, cu->GetDefaultProtocol());	// just a hunch
	fprintf(fd, pctc, cu->GetDefaultProtocol());	// ^2
	////////////////////////////// FeatherNet extensions /////////////////////
	fprintf(fd, pctc, TermCap->IsANSI());			// ANSI detected?
	fprintf(fd, getmsg(158), ns);					// good nuf for FNP, but it's still padding
	wrtMKI(thisRoom, fd);
	// Door Master 5.8 stops after 128 characters, so this should be enough
	fclose(fd);
	}


// --------------------------------------------------------------------------
//	writeTribbs(): Writes TRIBBS.SYS.
//
// ####################################################################
// ####################################################################
// ####################################################################
/*
	 TRIBBS.SYS
	 ----------

	 Is TriBBS's proprietary door data file.  It is an ASCII text file and
	 uses the following format:

	 1							   <- The user's record number
	 Mark Goodwin				   <- The user's name
	 Something					   <- The user's password
	 200						   <- The user's security level
	 Y							   <- Y for Expert, N for Novice
	 Y							   <- Y for ANSI, N for monochrome
	 60 						   <- Minutes left for this call
	 207-941-0805				   <- The user's phone number
	 Bangor, Me 				   <- The user's city and state
	 10/19/56					   <- The user's birth date
	 1							   <- The node number
	 1							   <- The serial port
	 2400						   <- Baud rate or 0 for local
	 38400						   <- Locked rate or 0 for not locked
	 Y							   <- Y for RTS/CTS, N for no RTS/CTS
	 N							   <- Y for error correcting or N
	 Unnamed BBS				   <- The board's name
	 Joe Sysop					   <- The sysop's name
	 Programmer 				   <- The user's alias.
	 Y							   <- Y for RIPScrip, N for no RIPScrip
*/

// ####################################################################
// ####################################################################
// ####################################################################

void TERMWINDOWMEMBER writeTribbs(void)
	{
	FILE *fd;
	char buff[80];
	const LogEntry * const cu = CurrentUser;		// wow, no changes

	sprintf(buff, sbs, cfg.aplpath, tribbsSys);
	if ((fd = fopen(buff, FO_W)) == NULL)
		{
		mPrintf(getmsg(8), tribbsSys);
		return;
		}

	wrtD(ThisLog, fd);						// User's record number
	label Buffer;
	wrtS(cu->GetName(Buffer, sizeof(Buffer)), fd);// User's name
	wrtS(cu->GetPassword(Buffer, sizeof(Buffer)), fd);// User's password
	wrtLvl(250, fd);							// User's security level (0-255)
	wrtC(cu->IsExpert() ? 'Y' : 'N', fd);       // User mode: Y=Expert
	wrtC(TermCap->IsANSI() ? 'Y' : 'N', fd);// Y if ANSI, N if not
	wrtCredits(60, fd); 						// Minutes left this call
	wrtS(cu->GetPhoneNumber(Buffer, sizeof(Buffer)), fd);// User's Phone number
	wrtAddr(fd);								// User's City and state
	wrtDate(cu->GetBirthDate(), fd);			// User's Birthdate
	wrtD(1, fd);								// Node number
	wrtD(cfg.mdata, fd);						// Serial port
	wrtUserSpd(FALSE, fd);						// User baud rate, 0 if local
	wrtLD(bauds[CommPort->GetSpeed()], fd);// Port rate or 0 if not locked
	wrtC(cfg.checkCTS ? 'Y' : 'N', fd);         // Y for RTS/CTS
	wrtC(CommPort->UsingCorrection ? 'Y' : 'N', fd);  // Error correcting connection
	wrtS(cfg.nodeTitle, fd);					// Board's name
	wrtS(cfg.sysop, fd);						// Sysop's Name
	wrtS(cu->GetName(Buffer, sizeof(Buffer)), fd);// User's alias
	wrtC('N', fd);                              // !!! RIPscrip support
	fclose(fd);
	}

// --------------------------------------------------------------------------
//	writeUserinfo	writes USERINFO.DAT.
//
// ####################################################################
// ####################################################################
// ####################################################################
/*
[wc30rec.new, 92May]
With the release of v3.0 Wildcat! no longer creates CALLINFO.BBS.  The
decision to drop CALLINFO.BBS came about due to our desire to support
the generic DOOR.SYS format used by RBBS/GAP & PCBOARD.

However, DOOR.SYS doesn't contain all the information necessary for
Wildcat! v3.0.	Therefore, we also create a file called USERINFO.DAT
which gets created in the \WCWORK\NODEx (x = Node Number) directory.
This is a standard text file.

Here is the format for our new USERINFO.DAT file:

   Jim Harrer		{ Line	1 - Full Users Name }
   LOCAL			{ Line	2 - Connect Rate or LOCAL }
   8				{ Line	3 - Databits }
   Y				{ Line	4 - 'Y' or 'N' for Ansi Enabled }
   Y				{ Line	5 - 'Y' or 'N' for MNP Connection }
   14:22 02/22/91	{ Line	6 - Time and Date of Call }
   14:28 02/22/91	{ Line	7 - Time and Date entered door }
   52				{ Line	8 - Minutes remaining }
   N				{ Line	9 - Started with a CommandLine switch }
   0				{ Line 10 - Current Conference }
   0				{ Line 11 - Active Menu # (0=Main,1=Msg,2=File,3=Sysop)}
   NEWUSER			{ Line 12 - Security level name }
   60				{ Line 13 - Banked time.  (DLT - MaxLogOn) }
   1				{ Line 14 - Door number }
   THEBOSS			{ Line 15 - Alias Name, if they have one }
   N				{ Line 16 - 'Y' or 'N' did the caller hang-up in door }
   0				{ Line 17 - # of files downloaded }
   0				{ Line 18 = # of Kilobytes downloaded }
*/
// ####################################################################
// ####################################################################
// ####################################################################

void TERMWINDOWMEMBER writeUserinfo(void)		// WC3.0:wc30rec.new
	{
	FILE *fd;
	char buff[80];
	const LogEntry * const cu = CurrentUser;		// wow, no changes

	sprintf(buff, sbs, cfg.aplpath, userinfoDat);
	if ((fd = fopen(buff, FO_W)) == NULL)
		{
		mPrintf(getmsg(8), userinfoDat);
		return;
		}

	label Buffer;
	wrtS(cu->GetName(Buffer, sizeof(Buffer)), fd);// Full Users name
	if (CommPort->HaveConnection()) 			// Connect rate or LOCAL
		{
		wrtLD(connectbauds[CommPort->GetModemSpeed()], fd);
		}
	else
		{
		wrtS(getmsg(376), fd);
		}

	wrtD(8, fd);								// Databits
	wrtC(TermCap->IsANSI() ? 'Y' : 'N', fd);// 'Y' or 'N' for ANSI enabled
	wrtC('N', fd);                              // !!! 'Y' or 'N' for MNP connection
	strftime(buff, sizeof(buff)-1, getmsg(237), LogTime());
	wrtS(buff, fd); 							// Time and Date of Call
	strftime(buff, sizeof(buff)-1, getmsg(237), time(NULL));
	wrtS(buff, fd); 							// Time and Date of entered door
	wrtCredits(60, fd); 						// Minutes remaining
	wrtC('N', fd);                              // !!! Started with cmdline switch
	wrtD(thisRoom, fd); 					// Current Conference
	wrtD(0, fd);					// Active menu#: 0=main,1=msg,2=file,3=sysop
	wrt(fd);									// Security level name, "NEWUSER"
	wrtD(60, fd);								// !!! Banked time (DLT - MaxLogOn)
	wrtD(1, fd);								// !!! Door number
	wrtS(cu->GetName(Buffer, sizeof(Buffer)), fd);// Alias name (seriously!)
	wrtC('N', fd);                              // Did user hang up in door?!?
	wrtU(cu->GetDL_Num(), fd);					// # of files downloaded
	wrtLU(cu->GetDL_Bytes() / 1024L, fd);		// # of Kilobytes downloaded
	fclose(fd);
	}

// USERINFO.XBS -- gleaned from XBBS source code (XUSERS.C, 90Nov02)
//static void wrtTimeS(time_t x, FILE *fd)
//	{
//	char buff[80];
//
//	strftime(buff, sizeof(buff) - 1, getmsg(242), (x));
//	fprintf(fd, getmsg(472), buff, bn);
//	}
//
//	wrtS(cu->GetName(), fd);					// User's alias
//	wrtUserSpd(FALSE, fd);						// User baud rate or 0 if local
//	wrtDate(cu->GetCallTime(), fd); 			// Last Date Called
//	wrtTimeS(cu->GetCallTime(), fd);			// Time of Last Call
//	wrtDate(LogTime(), fd); 					// Date of This Call
//	wrtTimeS(LogTime(), fd);					// Time of This Call
//	wrtLD(cu->GetLogins(), fd); 				// Total times on
//	wrtD(cu->IsIBMANSI(), fd);					// graphics?
//	wrtCredits(60, fd); 						// Minutes left
//	wrtD(chatReq, fd);						// Pages???
//	wrtCredits(60, fd); 						// Credits
//	wrtD(cu->GetLinesPerScreen(), fd);			// Length

long TERMWINDOWMEMBER UserSpeed(Bool both)
	{
	if (CommPort->HaveConnection())
		{
		return (connectbauds[CommPort->GetModemSpeed()]);
		}
	else
		{
		return (both ? bauds[CommPort->GetSpeed()] : 0L);
		}			// current port rate or 0 if local
	}

long TERMWINDOWMEMBER LogTime(void) 		// when logged in
	{
	return (loggedIn ? logtimestamp : time(NULL));	// This calltime
	}

int TERMWINDOWMEMBER Credits(int divisor)	// don't overwhelm with a LONG
	{
	assert(divisor);
	if (cfg.accounting && CurrentUser->IsAccounting())
		{
		return ((int)
				min((CurrentUser->GetCredits() / (long)divisor),
				(long) (INT_MAX / divisor)));
		}
	else
		{
		return (INT_MAX / divisor);
		}
	}
