// --------------------------------------------------------------------------
// Citadel: DoSysop.CPP
//
// The root of the F6/.S menu.

#include "ctdl.h"
#pragma hdrstop

#include "config.h"
#include "infofile.h"
#include "filecmd.h"
#include "room.h"
#include "pathdat.h"
#include "log.h"
#include "auxtab.h"
#include "account.h"
#include "hall.h"
#include "group.h"
#include "cfgfiles.h"
#include "tallybuf.h"
#include "miscovl.h"
#include "extmsg.h"
#include "statline.h"
#ifdef WINCIT
#include "net6969.h"
#else
#include "msg.h"
#endif


// --------------------------------------------------------------------------
// Contents
//
// doSysop()		Privileged Fn: menu breakdown
// do_SysopGroup()	handles doSysop() Group functions
// do_SysopHall()	handles the doSysop hall functions
// globalverify()	does global sweep to verify any un-verified

#ifdef AUXMEM
static void ShowAuxmemUsage(auxTabList *List, const char *Name, int PageSlots);
#endif


// --------------------------------------------------------------------------
// doSysop() Privileged Fn: menu breakdown.

void TERMWINDOWMEMBER doSysop(void)
	{
	label scrFile;
	Bool saveSysop;

	if (!read_sys_messages())
		{
		cPrintf(getmsg(59));
		return;
		}

	MRO.Reverse = FALSE;

	saveSysop = CurrentUser->IsSysop();
	CurrentUser->SetSysop(TRUE);

	StatusLine.Update(WC_TWp);

	while (!ExitToMsdos)
		{
		amZap();

		SetDoWhat(SYSOPMENU);

		const int c = DoMenuPrompt(getsysmsg(1), NULL);

		SetDoWhat(DUNO);

		switch (toupper(c))
			{
			case '1':
				{
				SetDoWhat(SYSMAINT);

				char prompt[128];

				label Buffer;
				mPrintfCR(getsysmsg(2), HallData[MAINTENANCE].GetName(Buffer, sizeof(Buffer)), cfg.Lhall_nym);

				sprintf(prompt, getsysmsg(196), Buffer, cfg.Lhall_nym);

				if (getYesNo(prompt, 0))
					{
					for (r_slot i = 0; i < cfg.maxrooms; i++)
						{
						if (i != DUMP && RoomTab[i].IsInuse() && !HallData[MAINTENANCE].IsRoomInHall(i))
							{
							mPrintfCR(pcts, RoomTab[i].GetName(Buffer, sizeof(Buffer)));
							HallData[MAINTENANCE].SetRoomInHall(i, TRUE);
							}
						}

					HallData.Save();
					}

				break;
				}

			case '6':
				{
				mPrintf(getsysmsg(25));
				net69_menu();
				break;
				}

#ifdef WINCIT
			case '9':
				{
				mPrintf("net6969 debug");
				Net6969.Debug(this);
				break;
				}
#endif

			case ESC:
			case 'A':
				{
				mPrintfCR(getmsg(653));
				}

			case 0:
				{
				// restore old mode
				CurrentUser->SetSysop(saveSysop);
				StatusLine.Update(WC_TWp);
				dump_sys_messages();
				return;
				}

			case 'C':
				{
				mPrintf(getsysmsg(3));
				cron_commands();
				break;
				}

			case 'D':
				{
				mPrintfCR(getsysmsg(5));
				changeDate();
				break;
				}

			case 'E':
				{
				mPrintf(getsysmsg(6));
				do_SysopEnter();
				break;
				}

			case 'F':
				{
				doAide(1, 'E');
				break;
				}

			case 'G':
				{
				mPrintf(getsysmsg(7), cfg.Ugroup_nym);
				do_SysopGroup();
				break;
				}

			case 'H':
				{
				mPrintf(getsysmsg(7), cfg.Uhall_nym);
				do_SysopHall();
				break;
				}

			case 'I':
				{
				SetDoWhat(SYSINFO);

				mPrintfCR(getsysmsg(9));
				ReIndexFileInfo();
				break;
				}

			case 'J':
				{
				mPrintf(getsysmsg(10));

				if (onConsole || CurrentUser->IsSuperSysop())
					{
					roomJournal();
					}
				else
					{
					CRCRmPrintfCR(getsysmsg(359));
					}
				break;
				}

			case 'K':
				{
				mPrintfCR(getsysmsg(11));
				killuser(NULL);
				break;
				}

			case 'L':
				{
				mPrintfCR(getsysmsg(12));
				sysopNew = TRUE;
				break;
				}

			case 'M':
				{
				mPrintfCR(getsysmsg(13));
				massdelete();
				break;
				}

			case 'N':
				{
				mPrintfCR(getsysmsg(14));
				globalverify();
				break;
				}

			case 'O':
				{
				SetDoWhat(SYSOFFHK);

				mPrintfCR(getsysmsg(4));

				if (onConsole)
					{
					offhook();
					}
				else
					{
					CRmPrintf(getsysmsg(44));
					}

				break;
				}

			case 'P':
				{
				mPrintfCR(getsysmsg(15));
				purgeuserlog();
				break;
				}

			case 'R':
				{
				SetDoWhat(SYSSCR);

				mPrintfCR(getsysmsg(16));
				getNormStr(getsysmsg(17), scrFile, LABELSIZE);

				if (*scrFile)
					{
#ifdef WINCIT
					runScript(scrFile, this);
#else
					runScript(scrFile);
#endif
					}

				break;
				}

			case 'S':
				{
				mPrintfCR(getsysmsg(18));
				CurrentUser->SetSysop(saveSysop);
				storeLog(); 	// in case we look at ourselves
				CurrentUser->SetSysop(TRUE);
				SysopShowUser();
				break;
				}

			case 'U':
				{
				mPrintfCR(getsysmsg(19));
				CurrentUser->SetSysop(saveSysop);
				userEdit();
				saveSysop = CurrentUser->IsSysop();
				break;
				}

			case 'X':
				{
				SetDoWhat(SYSEXIT);

				mPrintfCR(getsysmsg(21));

				if (onConsole || CurrentUser->IsSuperSysop())
					{
					if (getYesNo(getmsg(57), 0))
						{
						// restore old mode
						CurrentUser->SetSysop(saveSysop);
						ExitToMsdos = TRUE;
						}
					}
				else
					{
					CRmPrintf(getsysmsg(359));
					}

				break;
				}

			case 'Z':
				{
				mPrintfCR(getsysmsg(22));
				killempties();
				break;
				}

			case '!':
				{
				SetDoWhat(SYSSHELL);

				mPrintfCR(getsysmsg(23));

				if (onConsole)
					{
					shellescape(FALSE);
					}
				else
					{
					CRmPrintf(getsysmsg(44));
					}
				break;
				}

			case '@':
				{
				SetDoWhat(SYSSHELL);

				mPrintfCR(getsysmsg(24));

				if (onConsole)
					{
					shellescape(TRUE);
					}
				else
					{
					CRmPrintf(getsysmsg(44));
					}
				break;
				}

#ifdef AUXMEM
			case '^':
				{
				SetDoWhat(SYSAUXDEB);

				OC.SetOutFlag(OUTOK);

				mPrintfCR(getsysmsg(26));

				CRmPrintf(getsysmsg(27), usexms, xmshand, xmssize);
				CRmPrintf(getsysmsg(28), emsframe, emshand, emssize * (AUXPAGESIZE / 1024));
				CRmPrintf(getsysmsg(29), vrthand, vrtpags * (AUXPAGESIZE / 1024));

extern auxTabList *scrollBuf;
extern int LinesPerPage;

#ifdef GOODBYE
				ShowAuxmemUsage(mtList, getsysmsg(30), MSGTABPERPAGE);
				ShowAuxmemUsage(rtList, getsysmsg(36), ROOMTABPERPAGE);
				ShowAuxmemUsage(((talleyTaskInfo *) (talleyInfo))->tbList, getsysmsg(47), TALLEYBUFPERPAGE);
#endif

				if (LinesPerPage)
					{
					ShowAuxmemUsage(scrollBuf, getsysmsg(183), LinesPerPage);
					}

extern auxTabList *freeBlock;
				ShowAuxmemUsage(freeBlock, getsysmsg(37), 1);

				if (xmserr)
					{
					int errcode = 0;

					switch (xmserr)
						{
						case 0x80: errcode = 91; break;
						case 0x81: errcode = 92; break;
						case 0x82: errcode = 93; break;
						case 0x8e: errcode = 94; break;
						case 0x8f: errcode = 95; break;
						case 0x90: errcode = 96; break;
						case 0x91: errcode = 97; break;
						case 0x92: errcode = 98; break;
						case 0x93: errcode = 99; break;
						case 0x94: errcode = 100; break;
						case 0xa0: errcode = 101; break;
						case 0xa1: errcode = 102; break;
						case 0xa2: errcode = 103; break;
						case 0xa3: errcode = 104; break;
						case 0xa4: errcode = 105; break;
						case 0xa5: errcode = 106; break;
						case 0xa6: errcode = 107; break;
						case 0xa7: errcode = 108; break;
						case 0xa8: errcode = 109; break;
						case 0xa9: errcode = 110; break;
						case 0xaa: errcode = 111; break;
						case 0xab: errcode = 112; break;
						case 0xac: errcode = 113; break;
						case 0xad: errcode = 114; break;
						case 0xb0: errcode = 115; break;
						case 0xb1: errcode = 116; break;
						case 0xb2: errcode = 269; break;
						}

					if (errcode)
						{
						CRmPrintf(getecmsg(167), getsysmsg(errcode), xmserr);
						}
					else
						{
						CRmPrintf(getsysmsg(222), xmserr);
						}
					}

				if (emserr)
					{
					if ((emserr >= 0x80) && (emserr <= 0xa4))
						{
						CRmPrintf(getsysmsg(267), getsysmsg(emserr - 0x80 + 117), emserr);
						}
					else
						{
						CRmPrintf(getsysmsg(270), emserr);
						}
					}

				doCR();
				break;
				}
#endif

			case '#':
				{
				SetDoWhat(SYSREADMSG);

				mPrintfCR(getsysmsg(39), cfg.Lmsg_nym);
				MRO.Clear(CurrentUser->IsAutoVerbose());
				readbymsgno();
				break;
				}

			case '%':
				{
				SetDoWhat(SYSFPDOWN);

				mPrintf(getsysmsg(43));
				doSysFPDown();
				break;
				}

			case '>':
				{
				CreatePathDat();
				break;
				}

			case '+':
				{
				ShowTables();
				break;
				}

			// Get rid of this later
//			case '}':
//				{
//				for (i = 0; i < 200; i++)
//					{
//					char string[10];
//					itoa(i, string, 10);
//					autoroom(string);
//					}
//
//				break;
//				}

			case '?':
				{
				SetDoWhat(SYSMENU);

				oChar('?');
				showMenu(M_SYSOP);
				break;
				}

			default:
				{
				BadMenuSelection(c);
				break;
				}
			}
		}

	// restore old mode
	CurrentUser->SetSysop(saveSysop);
	dump_sys_messages();
	}


// --------------------------------------------------------------------------
// do_SysopGroup(): Handles doSysop() Group functions.

void TERMWINDOWMEMBER do_SysopGroup(void)
	{
	char ich;

	SetDoWhat(SYSGRP);

	switch (toupper(ich = (char) iCharNE()))
		{
		case 'E':
		case 'R':
			{
			mPrintf(getmsg(197));
			renamegroup();
			break;
			}

		case 'G':
			{
			mPrintf(getsysmsg(46), cfg.Ugroup_nym);
			globalgroup(NULL, FALSE);
			break;
			}

		case 'K':
			{
			mPrintf(getsysmsg(184));
			killgroup(NULL);
			break;
			}

		case 'L':
			{
			mPrintf(getmsg(332), cfg.Lgroup_nym);
			DoCommand(UC_AIDELISTGROUP);
			break;
			}

		case 'N':
			{
			mPrintf(getsysmsg(48));
			newgroup(NULL, NULL, FALSE, FALSE, FALSE);
			break;
			}

		case 'O':
			{
			mPrintf(getsysmsg(212));
			OperatorSpecificMembership();
			break;
			}

		case 'U':
			{
			mPrintfCR(getsysmsg(49));
			globaluser();
			break;
			}

		case '?':
			{
			oChar('?');
			showMenu(M_SYSGROUP);
			break;
			}

		case ESC:
			{
			break;
			}

		default:
			{
			BadMenuSelection(ich);
			break;
			}
		}
	}


// --------------------------------------------------------------------------
// do_SysopHall(): Handles the doSysop hall functions.

void TERMWINDOWMEMBER do_SysopHall()
	{
	char ich;

	SetDoWhat(SYSHALL);

	switch (toupper(ich = (char) iCharNE()))
		{
		case 'E':
		case 'R':
			{
			mPrintf(getmsg(197));
			renamehall();
			break;
			}

		case 'F':
			{
			mPrintf(getsysmsg(50));
			force();
			break;
			}

		case 'G':
			{
			mPrintf(getsysmsg(55));
			globalhall();
			break;
			}

		case 'K':
			{
			mPrintf(getsysmsg(184));
			killhall(NULL);
			break;
			}

		case 'L':
			{
			mPrintfCR(getmsg(332), cfg.Lhalls_nym);
			ListHalls(FALSE);
			break;
			}

		case 'N':
			{
			mPrintf(getsysmsg(48));
			newhall(NULL, NULL, thisRoom);
			break;
			}

		case '+':
			{
			moveHall(1, thisHall, TRUE);
			break;
			}

		case '-':
			{
			moveHall(-1, thisHall, TRUE);
			break;
			}

		case '?':
			{
			oChar('?');
			showMenu(M_SYSHALL);
			break;
			}

		case ESC:
			{
			break;
			}

		default:
			{
			BadMenuSelection(ich);
			break;
			}
		}
	}


// --------------------------------------------------------------------------
// do_SysopEnter(): Handle .SE functions.

void TERMWINDOWMEMBER do_SysopEnter(void)
	{
	SetDoWhat(SYSENTER);

	if (read_cfg_messages())
		{
		Bool all = FALSE;
		const Bool oldAcct = cfg.accounting;

		const int ich = iCharNE();

		switch (toupper(ich))
			{
			case 'A':
				mPrintfCR(getsysmsg(63));
				all = TRUE;

			case 'C':
				{
				mPrintfCR(citfiles[C_CONFIG_CIT]);
				changedir(cfg.homepath);		// just in case

				readconfig(CitThis, 2);

				if (!oldAcct && cfg.accounting)
					{
					ReadGrpdataCit(WC_TWp);

					CurrentUserAccount = new UserAccountInfo;
					if (!CurrentUserAccount)
						{
						crashout(getsysmsg(357));
						}

					CurrentUserAccount->Negotiate(CurrentUser);
					if (CurrentUser->IsAccounting())
						{
						CurrentUserAccount->GiveDailyIncrement();
						}
					}

				if (oldAcct && !cfg.accounting)
					{
					delete [] AccountingData;
					AccountingData = NULL;
					delete CurrentUserAccount;
					CurrentUserAccount = NULL;
					}

				if (!all)
					{
					break;
					}
				}

			case 'E':
				{
				mPrintfCR(citfiles[C_EXTERNAL_CIT]);
				ReadExternalCit(WC_TWp);

				if (!all)
					{
					break;
					}
				}

			case 'G':
				{
				if (!(all && !cfg.accounting))
					{
					mPrintfCR(citfiles[C_GRPDATA_CIT]);

					if (cfg.accounting)
						{
						ReadGrpdataCit(WC_TWp);
						}
					else
						{
						mPrintfCR(getsysmsg(64));
						}
					}

				if (!all)
					{
					break;
					}
				}

			case 'I':
				{
				mPrintfCR(citfiles[C_MCI_CIT]);
#ifdef WINCIT
				ReadMCICit(TRUE, WC_TWp);
#else
				ReadMCICit(TRUE);
#endif

				if (!all)
					{
					break;
					}
				}

			case 'M':
				{
				mPrintfCR(citfiles[C_MDMRESLT_CIT]);
				ReadMdmresltCit(WC_TWp);

				if (!all)
					{
					break;
					}
				}

			case 'O':
				{
				mPrintfCR(citfiles[C_COMMANDS_CIT]);
				ReadCommandsCit(WC_TWp);

				if (!all)
					{
					break;
					}
				}

			case 'P':
				{
				mPrintfCR(citfiles[C_PROTOCOL_CIT]);
				ReadProtocolCit(WC_TWp);

				if (!all)
					{
					break;
					}
				}

			case 'T':
				{
				mPrintfCR(citfiles[C_CRON_CIT]);
				Cron.ReadCronCit(WC_TWp);
				break;
				}

			case '?':
				{
				oChar('?');
				showMenu(M_SYSENTER);
				break;
				}

			case ESC:
				{
				break;
				}

			default:
				{
				BadMenuSelection(ich);
				break;
				}
			}

		dump_cfg_messages();
		compactMemory(1);
		}
	else
		{
		OutOfMemory(102);
		}
	}


void TERMWINDOWMEMBER doSysFPDown(void)
	{
	char filePath[256], tmp[256], *filename;

	int ch = iCharNE();

	if (ch == '\n' || ch == '\r')
		{
		ch = CurrentUser->GetDefaultProtocol();
		}

	const protocols *theProt = GetProtocolByKey((char) ch);

	if (theProt)
		{
		mPrintfCR(pcts, theProt->name);

		getNormStr(getsysmsg(65), filePath, 63);

		if (strchr(filePath, ' '))
			{
			mPrintfCR(getsysmsg(66));
			}
		else
			{
			if (filePath[0])
				{
				if (!strchr(filePath, '\\'))
					{
					CopyStringToBuffer(tmp, filePath);
					sprintf(filePath, sbs, cfg.homepath, tmp);
					}

				CopyStringToBuffer(tmp, filePath);
				*strrchr(tmp, '\\') = 0;

				if (!onConsole && !CurrentUser->IsSuperSysop() && inExternal(getmsg(341), tmp))
					{
					securityViolation(tmp);
					return;
					}

				if (ambig(filePath) && !theProt->batch)
					{
					mPrintfCR(getsysmsg(67));
					}
				else
					{
					filename = strrchr(filePath, '\\') + 1;
					doTheDownload(filename, tmp, TRUE, theProt, FALSE);
					}
				}
			}
		}
	else
		{
		if (ch == '?')
			{
			oChar('?');
			upDownMnu('D', FALSE);
			}
		else
			{
			oChar((char) ch);
			mPrintf(sqst);
			if (!CurrentUser->IsExpert())
				{
				upDownMnu('D', FALSE);
				}
			}
		}
	}

void MessageDatC::FillDebugInfo(MessageDebugInfoS *MDI, m_index Index)
	{
	Lock();

	MDI->IsValid = IsValid(Index);

	if (MDI->IsValid)
		{
		MDI->IsMail			= IsMail(Index);
		MDI->IsReceived		= IsReceived(Index);
		MDI->IsReply		= IsReply(Index);
		MDI->IsProblem		= IsProblem(Index);
		MDI->IsMadevis		= IsMadevis(Index);
		MDI->IsLimited		= IsLimited(Index);
		MDI->IsModerated	= IsModerated(Index);
		MDI->IsCopy			= IsCopy(Index);
		MDI->IsNet			= IsNet(Index);
		MDI->IsForwarded	= IsForwarded(Index);
		MDI->IsNetworked	= IsNetworked(Index);
		MDI->IsMassemail	= IsMassemail(Index);
		MDI->IsCensored		= IsCensored(Index);
		MDI->IsLocal		= IsLocal(Index);
		MDI->IsInuse		= IsInuse(Index);
		
		MDI->RoomNum		= GetRoomNum(Index);
		MDI->Location		= GetLocation(Index);
		MDI->OriginID		= GetOriginID(Index);
		
		MDI->CopyOffset		= GetCopyOffset(Index);
		MDI->ToHash			= GetToHash(Index);
		MDI->AuthHash		= GetAuthHash(Index);
	
		#if defined(WINCIT) || defined(AUXMEM)
			MDI->PrevRoomMsg = GetPrevRoomMsg(Index);
			MDI->NextRoomMsg = GetNextRoomMsg(Index);
		#endif
		}

	Unlock();
	}

void TERMWINDOWMEMBER ShowTables(void)
	{
	SetDoWhat(SYSTAB);

	int ich;

	mPrintf(getsysmsg(273));

	switch (toupper(ich = iCharNE()))
		{
		case 'M':
			{
			label Newest;
			CopyStringToBuffer(Newest, ltoac(MessageDat.NewestMessage()));

			mPrintfCR(getsysmsg(274));
			CRmPrintfCR(getsysmsg(164), ltoac(MessageDat.OldestMessageInTable()), Newest);

			for (;;)
				{
				m_index msgno;
				char prompt[80];

				sprintf(prompt, getsysmsg(165), cfg.Lmsg_nym);
				msgno = getNumber(prompt, 0l, LONG_MAX, -1l, FALSE, NULL);

				if (!msgno || msgno == -1)
					{
					break;
					}

				MessageDebugInfoS MDI;

				MessageDat.FillDebugInfo(&MDI, msgno);

				if (!MDI.IsValid)
					{
					CRmPrintfCR(getsysmsg(166));
					}
				else
					{
					CRmPrintfCR(getsysmsg(275), MDI.IsMail ?	getmsg(626) : getmsg(627));
					mPrintfCR(getsysmsg(276), MDI.IsReceived ?	getmsg(626) : getmsg(627));
					mPrintfCR(getsysmsg(277), MDI.IsReply ?		getmsg(626) : getmsg(627));
					mPrintfCR(getsysmsg(278), MDI.IsProblem ?	getmsg(626) : getmsg(627));
					mPrintfCR(getsysmsg(279), MDI.IsMadevis ?	getmsg(626) : getmsg(627));
					mPrintfCR(getsysmsg(280), MDI.IsLimited ?	getmsg(626) : getmsg(627));
					mPrintfCR(getsysmsg(281), MDI.IsModerated ?	getmsg(626) : getmsg(627));
					mPrintfCR(getsysmsg(283), MDI.IsCopy ?		getmsg(626) : getmsg(627));
					mPrintfCR(getsysmsg(284), MDI.IsNet ?		getmsg(626) : getmsg(627));
					mPrintfCR(getsysmsg(285), MDI.IsForwarded ?	getmsg(626) : getmsg(627));
					mPrintfCR(getsysmsg(286), MDI.IsNetworked ?	getmsg(626) : getmsg(627));
					mPrintfCR(getsysmsg(287), MDI.IsMassemail ?	getmsg(626) : getmsg(627));
					mPrintfCR(getsysmsg(288), MDI.IsCensored ?	getmsg(626) : getmsg(627));
					mPrintfCR(getsysmsg(289), MDI.IsLocal ?		getmsg(626) : getmsg(627));
					mPrintfCR(getsysmsg(290), MDI.IsInuse ?		getmsg(626) : getmsg(627));
					mPrintfCR(getsysmsg(291), ltoac(MDI.RoomNum));
					mPrintfCR(getsysmsg(292), ltoac(MDI.Location));
					mPrintfCR(getsysmsg(295), ltoac(MDI.OriginID));

					if (MDI.IsCopy)
						{
						mPrintfCR(getsysmsg(296), ltoac(MDI.CopyOffset));
						}
					else
						{
						mPrintfCR(getsysmsg(293), ltoac(MDI.ToHash));
						mPrintfCR(getsysmsg(294), ltoac(MDI.AuthHash));
						}

					#if defined(WINCIT) || defined(AUXMEM)
						mPrintfCR(getsysmsg(323), ltoac(MDI.PrevRoomMsg));
						mPrintfCR(getsysmsg(324), ltoac(MDI.NextRoomMsg));
					#endif
					}
				}

			break;
			}

		case 'R':
			{
			mPrintfCR(getsysmsg(297));

			for (;;)
				{
				r_slot RoomNumber;

				RoomNumber = (r_slot) getNumber(getsysmsg(298), 0l,
						cfg.maxrooms - 1, -1l, FALSE, NULL);

				if (RoomNumber < 0)
					{
					break;
					}

				CRmPrintfCR(getsysmsg(299), RoomTab[RoomNumber].IsInuse() ? getmsg(626) : getmsg(627));

				if (RoomTab[RoomNumber].IsInuse())
					{
					mPrintfCR(getsysmsg(300), RoomTab[RoomNumber].IsPermanent() ?		getmsg(626) : getmsg(627));
					mPrintfCR(getsysmsg(301), RoomTab[RoomNumber].IsGroupOnly() ?		getmsg(626) : getmsg(627));
					mPrintfCR(getsysmsg(302), RoomTab[RoomNumber].IsBooleanGroup() ?	getmsg(626) : getmsg(627));
					mPrintfCR(getsysmsg(303), RoomTab[RoomNumber].IsPrivilegedGroup() ?	getmsg(626) : getmsg(627));
					mPrintfCR(getsysmsg(304), RoomTab[RoomNumber].IsReadOnly() ?		getmsg(626) : getmsg(627));
					mPrintfCR(getsysmsg(305), RoomTab[RoomNumber].IsModerated() ?		getmsg(626) : getmsg(627));
					mPrintfCR(getsysmsg(306), RoomTab[RoomNumber].IsDownloadOnly() ?	getmsg(626) : getmsg(627));
					mPrintfCR(getsysmsg(307), RoomTab[RoomNumber].IsUploadOnly() ?		getmsg(626) : getmsg(627));
					mPrintfCR(getsysmsg(308), RoomTab[RoomNumber].IsShared() ?			getmsg(626) : getmsg(627));
					mPrintfCR(getsysmsg(309), RoomTab[RoomNumber].IsMsDOSdir() ?		getmsg(626) : getmsg(627));
					mPrintfCR(getsysmsg(310), RoomTab[RoomNumber].IsHidden() ?			getmsg(626) : getmsg(627));
					mPrintfCR(getsysmsg(311), RoomTab[RoomNumber].IsBIO() ?				getmsg(626) : getmsg(627));
					mPrintfCR(getsysmsg(312), RoomTab[RoomNumber].IsSubject() ?			getmsg(626) : getmsg(627));
					mPrintfCR(getsysmsg(313), RoomTab[RoomNumber].IsAnonymous() ?		getmsg(626) : getmsg(627));
					mPrintfCR(getsysmsg(314), RoomTab[RoomNumber].IsArchive() ?			getmsg(626) : getmsg(627));
					mPrintfCR(getsysmsg(315), RoomTab[RoomNumber].IsApplication() ?		getmsg(626) : getmsg(627));

					if (RoomTab[RoomNumber].IsPrivilegedGroup())
						{
						mPrintfCR(getsysmsg(316), RoomTab[RoomNumber].IsGroupModerates() ? getmsg(626) : getmsg(627));
						}

					label Buffer;
					mPrintfCR(getsysmsg(317), RoomTab[RoomNumber].GetName(Buffer, sizeof(Buffer)));
					mPrintfCR(getsysmsg(318), RoomTab[RoomNumber].GetNetID(Buffer, sizeof(Buffer)));

					if (RoomTab[RoomNumber].IsGroupOnly())
						{
						if (RoomTab[RoomNumber].IsBooleanGroup())
							{
							mPrintf(getsysmsg(319), ns);
#ifndef WINCIT
							BoolExpr Buffer;
							ShowBooleanExpression(RoomTab[RoomNumber].GetGroupExpression(Buffer), ShowGroup);
#endif
							}
						else
							{
							mPrintf(getsysmsg(319),
									GroupData[RoomTab[RoomNumber].GetGroupNumber()].GetName(Buffer, sizeof(Buffer)));
							}

						doCR();
						}

					if (RoomTab[RoomNumber].IsPrivilegedGroup())
						{
						label Buffer;
						mPrintfCR(getsysmsg(320),
								GroupData[RoomTab[RoomNumber].GetPGroupNumber()].GetName(Buffer, sizeof(Buffer)));
						}
					}
				}

			break;
			}

		case '?':
			{
			oChar('?');
			showMenu(M_SYSTABLE);
			break;
			}

		case ESC:
			{
			break;
			}

		default:
			{
			BadMenuSelection(ich);
			break;
			}
		}
	}

#ifdef AUXMEM
static void ShowAuxmemUsage(auxTabList *List, const char *Name, int PageSlots)
	{
	for (int j = 1; List; List = List->next, j++)
		{
		CRmPrintf(Name, j);
		if (List->whatMem == atHEAP) mPrintf(getsysmsg(31));
		if (List->whatMem == atXMS) mPrintf(getsysmsg(32));
		if (List->whatMem == atEMS) mPrintf(getsysmsg(33));
		if (List->whatMem == atVIRT) mPrintf(getsysmsg(34));
		mPrintf(getsysmsg(35), MkPtr(List->Where), List->first, List->first + PageSlots - 1);
		}
	}
#endif
