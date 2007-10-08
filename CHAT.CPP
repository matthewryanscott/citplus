// --------------------------------------------------------------------------
// Citadel: Chat.CPP
//
// Chat stuff.

#include "ctdl.h"
#pragma hdrstop

#include "infilt.h"
#include "account.h"
#include "log.h"
#include "miscovl.h"
#include "term.h"
#include "domenu.h"
#include "termwndw.h"
#ifdef WINCIT
#include "net6969.h"
#endif

#define MAXCHATTEXT 256

// --------------------------------------------------------------------------
// Contents
//
// doChat()     Menu-level interface. Returns TRUE if chat happend; else FALSE
// ringSysop()  ring the sysop
// chat()       This is the chat mode. used if sysop answers.


// --------------------------------------------------------------------------
// chat(): This is the chat mode. used if sysop answers.

void TERMWINDOWMEMBER chat(void)
{
	int c, from, lastfrom = 2, wsize = 0, i;
	char word[50];

	chatkey = FALSE;
	chatReq = FALSE;

	if (!CommPort->HaveConnection())
		{
		dial_out();
		return;
		}

	SetDoWhat(DOCHAT);

	cometochat();

	OC.SetOutFlag(IMPERVIOUS);

	do
		{
		c = 0;

		if (KBReady())
			{
			c = ciChar();
			from = 0;
			}
		else if (CommPort->IsInputReady())
			{
			if (!onConsole)
				{
				c = CommPort->Input();
				from = 1;
				}
			else
				{
				CommPort->Input();
				}
			}

		if (c < 128)
			{
			c = filt_in[c];
			}

		if (c && c != 26)   // Control+Z
			{
			if (from != lastfrom)
				{
				if (from)
					{
					termCap(TERM_NORMAL);
					OC.ansiattr = cfg.attr;
					}
				else
					{
					termCap(TERM_BOLD);
					OC.ansiattr = cfg.cattr;
					}

				lastfrom = from;
				}

			if (c == '\r' || c == '\n' || c == ' ' || c == '\t' || wsize == 50)
				{
				wsize = 0;
				}
			else
				{
				if (OC.CrtColumn >= CurrentUser->GetWidth() - 1)
					{
					if (wsize)
						{
						for (i = 0; i < wsize; i++)
							{
							doBS();
							}

						doCR();

						for (i = 0; i < wsize; i++)
							{
							echocharacter(word[i]);
							}
						}
					else
						{
						doCR();
						}

					wsize = 0;
					}
				else
					{
					// fix from maven

					if (c == '\b')
						{
						if (wsize) wsize--;
						}
					else
						{
						word[wsize] = (char) c;
						wsize++;
						}
					}
				}

			echocharacter((char) c);

			// make sure output gets back to remote user quickly if he's
			// typing. if input is already ready, it's probably a buffer
			// dump, so we shouldn't waste time making sure he's updated
			// now... his time will come.
			if (from && !CommPort->IsInputReady())
				{
				CommPort->FlushOutput();
				}
			}
		else
			{
#ifdef WINCIT
			// If we have no pending input to process, wait half a second
			if (KeyboardBuffer.IsEmpty() && !CommPort->IsInputReady())
				{
				HANDLE ToWaitFor[2];
				int WaitingFor = 0;

				// keyboard is worth looking at if we are focused
				if (TermWindowCollection.InFocus(this))
					{
					ToWaitFor[WaitingFor++] = hConsoleInput;
					}

				// comm port is worth looking at if we could create the event
				if (CommPort->InputWaiting)
					{
					ToWaitFor[WaitingFor++] = CommPort->InputWaiting;
					}

				if (WaitingFor)
					{
					WaitForMultipleObjects(WaitingFor, ToWaitFor, FALSE, 500);
					}
				}
#else
			CitIsIdle();    // None of the above occurred
#endif
			}
		} while ((c != 26) && CommPort->HaveConnection());  // Control+Z

	// don't account chat.
	if (cfg.accounting)
		{
		CurrentUserAccount->SetLastTimeAccounted(time(NULL));
		}

	termCap(TERM_NORMAL);
	OC.ansiattr = cfg.attr;
	time(&LastActiveTime);

	//jrs doCR();
}


// --------------------------------------------------------------------------
// ringSysop(): Ring the sysop.

Bool TERMWINDOWMEMBER ringSysop(void)
{

#define RINGLIMIT cfg.ringlimit

	static const char shave[] = { 10, 5, 5, 10, 20, 10, 75 };
	int j = 0;
	int special = 0;
	Bool oldnobells = cfg.noBells;

	CRmPrintf(getmenumsg(MENU_CHAT, 8));

	if (cfg.noBells == 1)
		{
		cfg.noBells = 0;
		}

	Bool answered = FALSE;

	int i;
	for (i = 0; (i < RINGLIMIT) && !answered && (HaveConnectionToUser()); i++)
		{
		if (cfg.LocalChatLen && !cfg.noBells)
			{
			int Freq = 1024;
			int Cur;

			for (Cur = 0; Cur < cfg.LocalChatLen && !answered; Cur++)
				{
				int TimeLeft = 0;

				if (cfg.LocalChat[Cur] > 0)
					{
					Freq = cfg.LocalChat[Cur];
					}
				else if (cfg.LocalChat[Cur] == INT_MIN)
					{
					CommPort->Output(BELL);
					}
				else if (cfg.LocalChat[Cur] == INT_MIN + 1)
					{
					outCon(BELL);
					}
				else if (cfg.LocalChat[Cur] == INT_MIN + 2)
					{
					oChar(BELL);
					}
				else if (cfg.LocalChat[Cur] < -5000)
					{
					// pause
					TimeLeft = -cfg.LocalChat[Cur] - 5000;
					}
				else
					{
					// duration
					TimeLeft = -cfg.LocalChat[Cur];
#ifdef WINCIT
					Beep(Freq, 0xFFFFFFFF);
#else
					sound(Freq);
#endif
					}


				// this is probably icky in the Windows verion: in the
				// Sleep(10), some other thread could take over for longer
				// then 10 milliseconds.
				while (TimeLeft && !answered)
					{
#ifdef WINCIT
					Sleep(10);
#else
					delay(10);
#endif

					TimeLeft -= 10;

					if (BBSCharReady() || KBReady())
						{
						answered = TRUE;
						}

					if (TimeLeft < 0)
						{
						TimeLeft = 0;
						}
					}

#ifdef WINCIT
				// If this is not Windows 95, we want to turn off the sound

				OSVERSIONINFO ovi;
				ovi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);

				if (!GetVersionEx(&ovi) || (ovi.dwPlatformId != VER_PLATFORM_WIN32_WINDOWS))
					{
					Beep(1024, 0);
					}
#else
				nosound();
#endif
				}
			}
		else
			{
			oChar(BELL);

			for (int k = 0; k < shave[j] && !answered; k++, special++)
				{
				if (BBSCharReady() || KBReady())
					{
					answered = TRUE;
					}

				twirlypause(1);
				if (cfg.chatflash && !(special % cfg.chatflash) )
					{
					setborder(special / cfg.chatflash);
					}
				}

			if (++j == 7)
				{
				j = 0;
				}
			}

		if (BBSCharReady() || KBReady())
			{
			answered = TRUE;
			}
		}

	cfg.noBells = oldnobells;

	if (cfg.chatflash)
		{
		setborder(cfg.battr);
		}

	if (KBReady())
		{
		iCharNE();

		chat();

		return (TRUE);
		}
	else if (i >= RINGLIMIT)
		{
		dispBlb(B_NOANSWER);

		return (FALSE);
		}
	else
		{
		iCharNE();

		return (FALSE);
		}
}


// --------------------------------------------------------------------------
// doChat()
//
//  expand: TRUE if dot command

void TERMWINDOWMEMBER doChat(Bool expand)
{
	SetDoWhat(DOCHAT);

	if (!LockMenu(MENU_CHAT))
		{
		OutOfMemory(37);
		return;
		}

	mPrintf(getmenumsg(MENU_CHAT, 0));

	const int ich = expand ? iCharNE() : 0;


	switch (expand ? DoMenu(MENU_CHAT, ich) : 1 /*LastChat cfg.chatmode*/)
		{
		case 1: // All
			{
			if (!expand)
				{
				mPrintf(getmenumsg(MENU_CHAT, 1));
				}
			if (CurrentUser->IsExpert())
				{
				oChar(':');
				}
			oChar(' ');
			DoCommand(UC_CHATALL);
			LastChat = 1;
			break;
			}

		case 2: // Exclusive
			{
			if (CurrentUser->IsExpert())
				{
				oChar(':');
				}
			oChar(' ');
			DoCommand(UC_CHATUSER);
			LastChat = 2;
			break;
			}

		case 3: // Group
			{
			if (CurrentUser->IsExpert())
				{
				oChar(':');
				}
			oChar(' ');
			DoCommand(UC_CHATGROUP);
			LastChat = 3;
			break;
			}

		case 4: // Room
			{
			if (CurrentUser->IsExpert())
				{
				oChar(':');
				}
			oChar(' ');
			DoCommand(UC_CHATROOM);
			LastChat = 4;
			break;
			}

		case 5: // Console
			{
			DoCommand(UC_CHATCONSOLE);
			LastChat = 5;
			break;
			}

		case -1: // ESC
			{
			break;
			}

		default: // Unknown
			{
			oChar((char) ich);
			mPrintf(sqst);
            if (CurrentUser->IsExpert())
				{
				break;
				}
			}

		case -2: // Menu
			{
			if (ich == '?')
				{
				oChar('?');
				}

			SetDoWhat(CHATMENU);

			showMenu(M_CHAT);
			break;
			}
		}

	UnlockMenu(MENU_CHAT);
	return;
}

#ifdef WINCIT
void TERMWINDOWMEMBER ChatAll(void)
{
	char Text[MAXCHATTEXT+1];

    label prompt;
    *prompt = 0;

    if (!CurrentUser->IsExpert())
        {
        strcpy(prompt, getmenumsg(MENU_CHAT, 9));
        doCR();
        getNormStr(prompt, Text, MAXCHATTEXT);
        }
    else
        {
        getString(ns, Text, MAXCHATTEXT, FALSE, ns);
        }

    if (*Text)
        {
        label Name;
        char Event[MAXCHATTEXT+176];
        if (!cfg.chattype)
            {
            sprintf(Event, "4%s0> %s", CurrentUser->GetName(Name, sizeof(Name)), Text);
            }
        else
            {            
            sprintf(Event, getmenumsg(MENU_CHAT, 10), CurrentUser->GetName(Name, sizeof(Name)),
                getmenumsg(MENU_CHAT, 13), Text);
            }
        TermWindowCollection.SystemEvent(SE_CHATALL, 0, NULL, FALSE, Event);

        sprintf(Event, getmenumsg(MENU_CHAT, 20), CurrentUser->GetName(Name, sizeof(Name)), cfg.nodeTitle,
            getmenumsg(MENU_CHAT, 13), Text);
        Net6969.ChatAll(Event);
        if (!CurrentUser->IsSeeOwnChats())
            {
            mPrintfCR(pcts, getmenumsg(MENU_CHAT, 16));            
            }
        }
}

void TERMWINDOWMEMBER ChatUser(void)
{
	label who;
	Bool no_escape;

	OC.Echo = CALLER;
	OC.setio();

	label prompt;
	*prompt = 0;

	if (!CurrentUser->IsExpert())
		{
		strcpy(prompt, cfg.Luser_nym);
		doCR();
		}

	AskUserName(who, ns, prompt, LABELSIZE, &no_escape, TRUE);

	if (*who)
		{
		l_slot logNo = FindPersonByPartialName(who);

		if (logNo == CERROR)
			{
			mPrintfCR(getmsg(595), who);
			}
	//	else if (loggedIn && logNo == ThisSlot)
	//		{
	//		CRmPrintfCR(getmenumsg(MENU_CHAT, 11));
	//		}
		else
			{
			LTab(logNo).GetName(who, sizeof(who));

			if (TermWindowCollection.IsUserLoggedIn(who))
				{
				char Prompt[100], Text[MAXCHATTEXT+1];
				sprintf(Prompt, getmenumsg(MENU_CHAT, 12), cfg.Luser_nym, who);

				getNormStr(Prompt, Text, MAXCHATTEXT);

				if (*Text)
					{
					label Name;
					char Event[MAXCHATTEXT+176];
        			if (!cfg.chattype)
						{
						sprintf(Event, "4%s3 to 4%s0)3 %s", CurrentUser->GetName(Name, sizeof(Name)), who, Text);
						}
					else
						{
						sprintf(Event, getmenumsg(MENU_CHAT, 10), CurrentUser->GetName(Name, sizeof(Name)), who, Text);
						}        

					switch (TermWindowCollection.EventToUser(who, TRUE, Event))
						{
					case FALSE: 
					    mPrintfCR(getmenumsg(MENU_CHAT, 14), who);    
					    break;
					case TRUE:  
						if (CurrentUser->IsSeeOwnChats())
							{
							if (!SameString(who, Name))
								{
								TermWindowCollection.EventToUser(Name, TRUE, Event);
								}
							}
						else
							{               
							mPrintfCR(pcts, getmenumsg(MENU_CHAT, 16));                                       
							}
						break;
					default:    
					    mPrintfCR(getmenumsg(MENU_CHAT, 15), who);    
					    break;
						}
					}
				}
			else
				{
				mPrintfCR(getmenumsg(MENU_CHAT, 17), who);
				}
			}

		OC.Echo = BOTH;
		OC.setio();
		}
	else
		{
		OC.Echo = BOTH;
		OC.setio();

        /* jrs
        this is annoying, disabling
		if (no_escape)
			{
			mPrintf("Switching to .Chat All");
			if (CurrentUser->IsExpert())
				{
				doCR();
				CRmPrintf("2Enter text to send to all:0 ");
				}
			ChatAll();
			}
        */
		}
}

void TERMWINDOWMEMBER ChatGroup(void)
{
	label GroupName;

    label prompt;
    *prompt = 0;

    if (!CurrentUser->IsExpert())
        {
        strcpy(prompt, cfg.Lgroup_nym);
        doCR();
        }

    AskGroupName(GroupName, LastChatGroup, prompt, LABELSIZE, TRUE);

    if (*GroupName)
        {
        g_slot GroupSlot = FindGroupByPartialName(GroupName, FALSE);

        if (GroupSlot == CERROR || !(CurrentUser->IsInGroup(GroupSlot)))
            {
            mPrintfCR(getmsg(584), cfg.Lgroup_nym);
            }
        else
            {
            GroupData[GroupSlot].GetName(GroupName, sizeof(GroupName));

            char Prompt[100], Text[MAXCHATTEXT+1];
            sprintf(Prompt, getmenumsg(MENU_CHAT, 12), cfg.Lgroup_nym, GroupName);

            getNormStr(Prompt, Text, MAXCHATTEXT);

            if (*Text)
                {
                label Name;
                char Event[MAXCHATTEXT+176];
                if (!cfg.chattype)
                    {
                    sprintf(Event, "4%s3 to 4%s0:3 %s", CurrentUser->GetName(Name, sizeof(Name)), GroupName, Text);
                    }
                else
                    {             
                    sprintf(Event, getmenumsg(MENU_CHAT, 10), CurrentUser->GetName(Name, sizeof(Name)), GroupName, Text);
                    }
                TermWindowCollection.SystemEvent(SE_CHATGROUP, GroupSlot, NULL, FALSE, Event);

                if (!CurrentUser->IsSeeOwnChats())
                    {                 
                    mPrintfCR(pcts, getmenumsg(MENU_CHAT, 16));
                    }
                }
            }
        }
}

void TERMWINDOWMEMBER ChatRoom(void)
{
	char Text[MAXCHATTEXT+1];


    char prompt[LABELSIZE+LABELSIZE+LABELSIZE+3];
    *prompt = 0;

    if (!CurrentUser->IsExpert())
        {
        sprintf(prompt, getmenumsg(MENU_CHAT, 18), cfg.Lusers_nym, cfg.Lroom_nym );
        doCR();
        }

    getNormStr(prompt, Text, MAXCHATTEXT);

    if (*Text)
        {
        label Name;
        label Room;
        char Event[MAXCHATTEXT+176];

        if (!cfg.chattype)
            {
            sprintf(Event, "4%s3 to 4%s0>>3 %s", CurrentUser->GetName(Name, sizeof(Name)), CurrentRoom->GetName(Room, sizeof(Room)), Text);
            }
        else
            {            
            char temp[LABELSIZE+LABELSIZE+LABELSIZE+3];
            sprintf(temp, getmenumsg(MENU_CHAT, 19), cfg.Lusers_nym, cfg.Lroom_nym );
            sprintf(Event, getmenumsg(MENU_CHAT, 10), CurrentUser->GetName(Name, sizeof(Name)),	temp, Text);
            }
        TermWindowCollection.SystemEvent(SE_CHATROOM, thisRoom, NULL, FALSE, Event);

        if (!CurrentUser->IsSeeOwnChats())
            {
            mPrintfCR(pcts, getmenumsg(MENU_CHAT, 16));         
            }   
        }
}
#endif

void TERMWINDOWMEMBER ChatConsole(void)
{
	if (!LockMenu(MENU_CHAT))
		{
		OutOfMemory(37);
		return;
		}

	chatReq = TRUE;
#ifdef WINCIT
    trap(T_CHAT, WindowCaption, getmenumsg(MENU_CHAT, 6));
#else
    trap(T_CHAT, getmenumsg(MENU_CHAT, 6));
#endif

	if (cfg.noChat || !CurrentUser->IsChat())
		{
		nochat(FALSE);

#ifdef WINCIT
        if (chatReq && ((cfg.chatmail == 1) || (cfg.chatmail == 4)))
            {
            Message *Msg = new Message;

            if (Msg)
                {
                msgtosysop(Msg);
				delete Msg;
				}
			else
				{
				OutOfMemory(37);
				}
			}
#endif
		}
	else
		{
		if (OC.whichIO == MODEM)
			{
			char ChatSubject[81];

			if (cfg.chatwhy)
				{
				mPrintfCR(getmenumsg(MENU_CHAT, 7));

				oChar('>');
				getNormStr(ns, ChatSubject, 80);
				}

			if (!ringSysop())
				{
#ifdef WINCIT
				if (chatReq && ((cfg.chatmail == 1) || (cfg.chatmail == 4)))
					{
					Message *Msg = new Message;

					if (Msg)
						{
						Msg->SetSubject(ChatSubject);
						msgtosysop(Msg);

						delete Msg;
						}
					else
						{
						OutOfMemory(37);
						}
					}
#endif
				}
			}
		else
			{
			chat();
			}
		}
	UnlockMenu(MENU_CHAT);
}

