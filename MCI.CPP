// --------------------------------------------------------------------------
// Citadel: MCI.CPP
//
// MCI Handler code

#include "ctdl.h"
#pragma hdrstop

#include "log.h"
#include "hall.h"
#include "room.h"
#include "tallybuf.h"
#include "term.h"
#include "helpfile.h"


// --------------------------------------------------------------------------
// Contents
//
// MCI()			MCI


Bool MCIRecursionCheckerC::Start(MCICommandsE WhichOne)
	{
	assert(WhichOne >= (MCICommandsE) 0);
	assert(WhichOne < MCI_NUMCODES);

	if (MCIDisabled[WhichOne] || MCIBusy[WhichOne])
		{
		return (FALSE);
		}
	else
		{
		MCIBusy[WhichOne] = TRUE;
		return (TRUE);
		}
	}

void TERMWINDOWMEMBER MCI_DispOrAsgn(const char *str, label vars[])
	{
	assert(str != NULL);
	assert(vars != NULL);

	if (MCI_asgnnext)
		{
		strncpy(vars[MCI_asgnvar], str, LABELSIZE);
		vars[MCI_asgnvar][LABELSIZE] = 0;

		MCI_asgnnext = FALSE;
		}
	else
		{
		mFormat(str);
		}
	}

void TERMWINDOWMEMBER MCI(const char *c)
	{
	assert(c);

	char *xOurCopy = strdup(c);

	if (xOurCopy)
		{
		char string[100];

		char *OurCopy = xOurCopy + 1;

		if (c[strlen(OurCopy) - 1] == 'X')
			{
			OurCopy[strlen(OurCopy) - 1] = 0;
			}

		if (OC.MCI_goto)
			{
			if ((strncmpi(OurCopy, MCICodes[MCI_LABEL], 2) == SAMESTRING) && (SameString(OurCopy + 2, OC.MCI_label)))
				{
				ansi(14);
				OC.MCI_goto = FALSE;
				OC.MCI_label[0] = 0;
				}
			}
		else
			{
			int i;
			for (i = 0; strncmpi(OurCopy, MCICodes[i], 2) != SAMESTRING; i++);

			switch (i)
				{
				case MCI_REALNAME:
					{
					if (OC.UseMCI && MCIRecursionChecker.Start(MCI_REALNAME))
						{
						if (loggedIn)
							{
							CurrentUser->GetRealName(string, sizeof(string));
							CopyStringToBuffer(string, special_deansi(string, MCICodes[MCI_REALNAME]));
							MCI_DispOrAsgn(string, MCI_str);
							}
						else
							{
							MCI_DispOrAsgn(getmsg(100), MCI_str);
							}

						MCIRecursionChecker.End(MCI_REALNAME);
						}

					break;
					}

				case MCI_LASTNAME:
					{
					if (OC.UseMCI && MCIRecursionChecker.Start(MCI_LASTNAME))
						{
						if (loggedIn)
							{
							CurrentUser->GetName(string, sizeof(string));

							CopyStringToBuffer(string, special_deansi(string, MCICodes[MCI_LASTNAME]));

							char *str = string;
							if (strrchr(str, ' '))
								{
								str = strrchr(string, ' ');
								}

							MCI_DispOrAsgn(str, MCI_str);
							}
						else
							{
							MCI_DispOrAsgn(getmsg(100), MCI_str);
							}

						MCIRecursionChecker.End(MCI_LASTNAME);
						}

					break;
					}

				case MCI_PHONENUM:
					{
					if (OC.UseMCI && MCIRecursionChecker.Start(MCI_PHONENUM))
						{
						if (loggedIn)
							{
							CurrentUser->GetPhoneNumber(string, sizeof(string));
							CopyStringToBuffer(string, special_deansi(string, MCICodes[MCI_PHONENUM]));
							MCI_DispOrAsgn(string, MCI_str);
							}
						else
							{
							MCI_DispOrAsgn(getmsg(100), MCI_str);
							}

						MCIRecursionChecker.End(MCI_PHONENUM);
						}

					break;
					}

				case MCI_ADDR:
					{
					if (OC.UseMCI && MCIRecursionChecker.Start(MCI_ADDR))
						{
						if (loggedIn)
							{
							if (OurCopy[2] == '2')
								{
								CurrentUser->GetMailAddr2(string, sizeof(string));
								CopyStringToBuffer(string, special_deansi(string, MCICodes[MCI_ADDR]));
								}
							else if (OurCopy[2] == '3')
								{
								CurrentUser->GetMailAddr3(string, sizeof(string));
								CopyStringToBuffer(string, special_deansi(string, MCICodes[MCI_ADDR]));
								}
							else
								{
								CurrentUser->GetMailAddr1(string, sizeof(string));
								CopyStringToBuffer(string, special_deansi(string, MCICodes[MCI_ADDR]));
								}

							MCI_DispOrAsgn(string, MCI_str);
							}
						else
							{
							MCI_DispOrAsgn(getmsg(100), MCI_str);
							}

						MCIRecursionChecker.End(MCI_ADDR);
						}

					break;
					}

				case MCI_LASTCALLD:
					{
					if (OC.UseMCI && MCIRecursionChecker.Start(MCI_LASTCALLD))
						{
						char Buffer[64];
						strftime(string, 79, (loggedIn) ? 
								special_deansi(CurrentUser->GetVerboseDateStamp(Buffer, sizeof(Buffer)),
								MCICodes[MCI_LASTCALLD]) : special_deansi(cfg.vdatestamp, MCICodes[MCI_LASTCALLD]), 0l);
						MCI_DispOrAsgn(string, MCI_str);

						MCIRecursionChecker.End(MCI_LASTCALLD);
						}

					break;
					}

				case MCI_ACCTBAL:
					{
					if (OC.UseMCI && MCIRecursionChecker.Start(MCI_ACCTBAL))
						{
						sprintf(string, pctld, CurrentUser->GetCredits() / 60);
						MCI_DispOrAsgn(string, MCI_str);
						MCIRecursionChecker.End(MCI_ACCTBAL);
						}

					break;
					}

				case MCI_ROOMNAME:
					{
					if (OC.UseMCI && MCIRecursionChecker.Start(MCI_ROOMNAME))
						{
						RoomTab[thisRoom].GetName(string, sizeof(string));
						CopyStringToBuffer(string, special_deansi(string, MCICodes[MCI_ROOMNAME]));
						MCI_DispOrAsgn(string, MCI_str);

						MCIRecursionChecker.End(MCI_ROOMNAME);
						}

					break;
					}

				case MCI_HALLNAME:
					{
					if (OC.UseMCI && MCIRecursionChecker.Start(MCI_HALLNAME))
						{
						HallData[thisHall].GetName(string, sizeof(string));
						CopyStringToBuffer(string, special_deansi(string, MCICodes[MCI_HALLNAME]));
						MCI_DispOrAsgn(string, MCI_str);

						MCIRecursionChecker.End(MCI_HALLNAME);
						}

					break;
					}

				case MCI_NODENAME:
					{
					if (OC.UseMCI && MCIRecursionChecker.Start(MCI_NODENAME))
						{
						CopyStringToBuffer(string, special_deansi(cfg.nodeTitle, MCICodes[MCI_NODENAME]));
						MCI_DispOrAsgn(string, MCI_str);

						MCIRecursionChecker.End(MCI_NODENAME);
						}

					break;
					}

				case MCI_NODEPHONE:
					{
					if (OC.UseMCI && MCIRecursionChecker.Start(MCI_NODEPHONE))
						{
						CopyStringToBuffer(string, special_deansi(cfg.nodephone, MCICodes[MCI_NODEPHONE]));
						MCI_DispOrAsgn(string, MCI_str);

						MCIRecursionChecker.End(MCI_NODEPHONE);
						}

					break;
					}

				case MCI_DISKFREE:
					{
					break;
					}

				case MCI_NUMMSGS:
					{
					if (OC.UseMCI && MCIRecursionChecker.Start(MCI_NUMMSGS))
						{
						sprintf(string, pctld, MS.Read);
						MCI_DispOrAsgn(string, MCI_str);

						MCIRecursionChecker.End(MCI_NUMMSGS);
						}

					break;
					}

				case MCI_CLS:
					{
					break;
					}

				case MCI_CALLNUM:
					{
					break;
					}

				case MCI_SYSOPNAME:
					{
					if (OC.UseMCI && MCIRecursionChecker.Start(MCI_SYSOPNAME))
						{
						CopyStringToBuffer(string, special_deansi(cfg.sysop, MCICodes[MCI_SYSOPNAME]));
						MCI_DispOrAsgn(string, MCI_str);

						MCIRecursionChecker.End(MCI_SYSOPNAME);
						}

					break;
					}

				case MCI_CONNRATE:
					{
					if (OC.UseMCI && MCIRecursionChecker.Start(MCI_CONNRATE))
						{
						sprintf(string, pctlu, connectbauds[CommPort->GetModemSpeed()]);
						MCI_DispOrAsgn(string, MCI_str);
						MCIRecursionChecker.End(MCI_CONNRATE);
						}

					break;
					}

				case MCI_PORTRATE:
					{
					if (OC.UseMCI && MCIRecursionChecker.Start(MCI_PORTRATE))
						{
						sprintf(string, pctlu, bauds[CommPort->GetSpeed()]);
						MCI_DispOrAsgn(string, MCI_str);
						MCIRecursionChecker.End(MCI_PORTRATE);
						}

					break;
					}

				case MCI_MSGROOM:
					{
					if (OC.UseMCI && MCIRecursionChecker.Start(MCI_MSGROOM))
						{
						sprintf(string, pctlu, Talley->MessagesInRoom(thisRoom));
						MCI_DispOrAsgn(string, MCI_str);
						MCIRecursionChecker.End(MCI_MSGROOM);
						}

					break;
					}

				case MCI_BEEPNUM:
					{
					if (OC.UseMCI && isdigit(OurCopy[2]) && MCIRecursionChecker.Start(MCI_BEEPNUM))
						{
						int num;

						for (num = OurCopy[2] - '0'; num; num--)
							{
							oChar(BELL);
							}

						MCIRecursionChecker.End(MCI_BEEPNUM);
						}

					break;
					}

				case MCI_GETCHAR:
					{
					if (OC.UseMCI && isdigit(OurCopy[2]) && MCIRecursionChecker.Start(MCI_GETCHAR))
						{
						MCI_char[OurCopy[2] - '0'] = (char) iCharNE();
						MCIRecursionChecker.End(MCI_GETCHAR);
						}

					break;
					}

				case MCI_GETSTR:
					{
					if (OC.UseMCI && isdigit(OurCopy[2]) && MCIRecursionChecker.Start(MCI_GETSTR))
						{
						UserControlType oldOF = OC.User.GetOutFlag();

						getString(ns, MCI_str[OurCopy[2] - '0'], LABELSIZE, FALSE, ns);

						OC.SetOutFlag(oldOF);

						MCIRecursionChecker.End(MCI_GETSTR);
						}

					break;
					}

				case MCI_PUTCHAR:
					{
					if (OC.UseMCI && isdigit(OurCopy[2]) && MCIRecursionChecker.Start(MCI_PUTCHAR))
						{
						oChar(MCI_char[OurCopy[2] - '0']);
						MCIRecursionChecker.End(MCI_PUTCHAR);
						}

					break;
					}

				case MCI_PUTSTR:
					{
					if (OC.UseMCI && isdigit(OurCopy[2]) && MCIRecursionChecker.Start(MCI_PUTSTR))
						{
						MCI_DispOrAsgn(MCI_str[OurCopy[2] - '0'], MCI_str);

						MCIRecursionChecker.End(MCI_PUTSTR);
						}

					break;
					}

				case MCI_BSNUM:
					{
					if (OC.UseMCI && isdigit(OurCopy[2]) && MCIRecursionChecker.Start(MCI_BSNUM))
						{
						int num;

						for (num = OurCopy[2] - '0'; num; num--)
							{
							doBS();
							}

						MCIRecursionChecker.End(MCI_BSNUM);
						}

					break;
					}

				case MCI_OUTSPEC:
					{
					break;
					}

				case MCI_SLOW:
					{
					break;
					}

				case MCI_PASSWORD:
					{
					if (OC.UseMCI &&
							MCIRecursionChecker.Start(MCI_PASSWORD))
						{
						if (loggedIn)
							{
							CurrentUser->GetPassword(string, sizeof(string));
							CopyStringToBuffer(string, special_deansi(string, MCICodes[MCI_PASSWORD]));
							MCI_DispOrAsgn(string, MCI_str);
							}
						else
							{
							MCI_DispOrAsgn(getmsg(100), MCI_str);
							}

						MCIRecursionChecker.End(MCI_PASSWORD);
						}

					break;
					}

				case MCI_INITIALS:
					{
					if (OC.UseMCI &&
							MCIRecursionChecker.Start(MCI_INITIALS))
						{
						if (loggedIn)
							{
							CurrentUser->GetInitials(string, sizeof(string));
							CopyStringToBuffer(string, special_deansi(string, MCICodes[MCI_INITIALS]));
							MCI_DispOrAsgn(string, MCI_str);
							}
						else
							{
							MCI_DispOrAsgn(getmsg(100), MCI_str);
							}

						MCIRecursionChecker.End(MCI_INITIALS);
						}

					break;
					}

				case MCI_GOTO:
					{
					if (OC.UseMCI && OC.User.GetOutFlag() != IMPERVIOUS && MCIRecursionChecker.Start(MCI_GOTO))
						{
						OC.MCI_goto = TRUE;
						strncpy(OC.MCI_label, OurCopy + 2, LABELSIZE);
						OC.MCI_label[LABELSIZE] = 0;

						if (OC.User.GetOutFlag() == IMPERVIOUS)
							{
							OC.SetOutFlag(OUTOK);
							}

						MCIRecursionChecker.End(MCI_GOTO);
						}

					break;
					}

				case MCI_COMPARE:
					{
					if (OC.UseMCI && isdigit(OurCopy[2]) && MCIRecursionChecker.Start(MCI_COMPARE))
						{
						if (toupper(OurCopy[3]) == 'V' && isdigit(OurCopy[5]))
							{
							// variable comparison
							if (toupper(OurCopy[4]) == 'N')
								{
								// numeric
								MCI_result = atoi(MCI_str[OurCopy[2] - '0']) - atoi(MCI_str[OurCopy[5] - '0']);
								}
							else if (toupper(OurCopy[4]) == 'C')
								{
								// character
								MCI_result = toupper(MCI_char[OurCopy[2] - '0']) - (MCI_char[OurCopy[5] - '0']);
								}
							else
								{
								// string
								MCI_result = strcmpi(MCI_str[OurCopy[2] - '0'], MCI_str[OurCopy[5] - '0']);
								}
							}
						else if (toupper(OurCopy[3]) == 'N')
							{
							// numeric
							MCI_result = atoi(MCI_str[OurCopy[2] - '0']) - atoi(OurCopy + 4);
							}
						else if (toupper(OurCopy[3]) == 'C')
							{
							// character
							MCI_result = toupper(MCI_char[OurCopy[2] - '0']) - toupper(OurCopy[4]);
							}
						else
							{
							// string
							MCI_result = strcmpi(MCI_str[OurCopy[2] - '0'], OurCopy + 4);
							}

						MCIRecursionChecker.End(MCI_COMPARE);
						}

					break;
					}

				case MCI_GT:
					{
					if (OC.UseMCI && MCIRecursionChecker.Start(MCI_GT))
						{
						if (MCI_result > 0)
							{
							label temp;

							temp[0] = 'X';
							strncpy(temp + 1, OurCopy + 2, LABELSIZE - 1);
							temp[LABELSIZE] = 0;
							termCap(temp);
							}

						MCIRecursionChecker.End(MCI_GT);
						}

					break;
					}

				case MCI_LT:
					{
					if (OC.UseMCI && MCIRecursionChecker.Start(MCI_LT))
						{
						if (MCI_result < 0)
							{
							label temp;

							temp[0] = 'X';
							strncpy(temp + 1, OurCopy + 2, LABELSIZE - 1);
							temp[LABELSIZE] = 0;
							termCap(temp);
							}

						MCIRecursionChecker.End(MCI_LT);
						}

					break;
					}

				case MCI_EQ:
					{
					if (OC.UseMCI && MCIRecursionChecker.Start(MCI_EQ))
						{
						if (MCI_result == 0)
							{
							label temp;

							temp[0] = 'X';
							strncpy(temp + 1, OurCopy + 2, LABELSIZE - 1);
							temp[LABELSIZE] = 0;
							termCap(temp);
							}

						MCIRecursionChecker.End(MCI_EQ);
						}

					break;
					}

				case MCI_RANDOM:
					{
					if (OC.UseMCI && isdigit(OurCopy[2]) && MCIRecursionChecker.Start(MCI_RANDOM))
						{
						sprintf(MCI_str[OurCopy[2] - '0'], pctd, random(atoi(OurCopy + 3)) + 1);
						MCIRecursionChecker.End(MCI_RANDOM);
						}

					break;
					}

				case MCI_ASSIGN:
					{
					if (OC.UseMCI && isdigit(OurCopy[2]) && MCIRecursionChecker.Start(MCI_ASSIGN))
						{
						strncpy(MCI_str[OurCopy[2] - '0'], OurCopy + 3, LABELSIZE);
						MCI_str[OurCopy[2] - '0'][LABELSIZE] = 0;
						MCIRecursionChecker.End(MCI_ASSIGN);
						}

					break;
					}

				case MCI_ADD:
					{
					if (OC.UseMCI && isdigit(OurCopy[2]) && MCIRecursionChecker.Start(MCI_ADD))
						{
						int i = 3, sum = 0;

						while (isdigit(OurCopy[i]))
							{
							sum += atoi(MCI_str[OurCopy[i++] - '0']);
							}
						sprintf(MCI_str[OurCopy[2] - '0'], pctd, sum);
						MCIRecursionChecker.End(MCI_ADD);
						}

					break;
					}

				case MCI_SUBTRACT:
					{
					if (OC.UseMCI && isdigit(OurCopy[2]) && isdigit(OurCopy[3]) && MCIRecursionChecker.Start(MCI_SUBTRACT))
						{
						int i = 4, dif = atoi(MCI_str[OurCopy[3] - '0']);

						while (isdigit(OurCopy[i]))
							{
							dif -= atoi(MCI_str[OurCopy[i++] - '0']);
							}

						sprintf(MCI_str[OurCopy[2] - '0'], pctd, dif);
						MCIRecursionChecker.End(MCI_SUBTRACT);
						}

					break;
					}

				case MCI_TIMES:
					{
					if (OC.UseMCI && isdigit(OurCopy[2]) && isdigit(OurCopy[3]) && MCIRecursionChecker.Start(MCI_TIMES))
						{
						int i = 4, prd = atoi(MCI_str[OurCopy[3] - '0']);

						while (isdigit(OurCopy[i]))
							{
							prd *= atoi(MCI_str[OurCopy[i++] - '0']);
							}
						sprintf(MCI_str[OurCopy[2] - '0'], pctd, prd);
						MCIRecursionChecker.End(MCI_TIMES);
						}

					break;
					}

				case MCI_DIVIDE:
					{
					if (OC.UseMCI && isdigit(OurCopy[2]) && isdigit(OurCopy[3]) && MCIRecursionChecker.Start(MCI_DIVIDE))
						{
						int i = 4, quo = atoi(MCI_str[OurCopy[3] - '0']);

						while (isdigit(OurCopy[i]))
							{
							int divisor = atoi(MCI_str[OurCopy[i++] - '0']);

							if (divisor)
								{
								quo /= divisor;
								}
							else
								{
								quo = INT_MAX;	// cheezy, huh?
								}
							}

						sprintf(MCI_str[OurCopy[2] - '0'], pctd, quo);
						MCIRecursionChecker.End(MCI_DIVIDE);
						}

					break;
					}

				case MCI_ASGNNEXT:
					{
					if (OC.UseMCI && isdigit(OurCopy[2]) && MCIRecursionChecker.Start(MCI_ASGNNEXT))
						{
						MCI_asgnnext = TRUE;
						MCI_asgnvar = OurCopy[2] - '0';
						MCIRecursionChecker.End(MCI_ASGNNEXT);
						}

					break;
					}

				case MCI_HANGINGINDENT:
					{
					if (OC.UseMCI && isdigit(OurCopy[2]) && MCIRecursionChecker.Start(MCI_HANGINGINDENT))
						{
						OC.HangingIndent = atoi(OurCopy + 2);
						MCIRecursionChecker.End(MCI_HANGINGINDENT);
						}

					break;
					}

				case MCI_HELPFILE:
					{
					if (OC.UseMCI && isdigit(OurCopy[2]) && MCIRecursionChecker.Start(MCI_HELPFILE))
						{
						const int HF = atoi(OurCopy + 2);

//                        if (HF >= 0 && HF <= MAXHLP)
//                            {
//                            char *dude;
//                            hlpnames[HF] = dude;
//                            if (*dude == NULL)
//                                {
//                                MCI_DispOrAsgn(ns, MCI_str);
//                                }
//                                else
//                                {
//                                MCI_DispOrAsgn(dude, MCI_str);
//                                }
//                            }
//                        else
//                            {
//                            MCI_DispOrAsgn(ns, MCI_str);
//                            }

						MCI_DispOrAsgn(HF >= 0 && HF <= MAXHLP ? hlpnames[HF] : ns, MCI_str);
						MCIRecursionChecker.End(MCI_HELPFILE);
						}

					break;
					}
				}
			}

		delete [] xOurCopy;
		}
	}
