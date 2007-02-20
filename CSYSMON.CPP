// --------------------------------------------------------------------------
// Citadel: CSysMon.CPP
//
// Console Sysop system Monitor

#ifndef WINCIT
#include "ctdl.h"
#pragma hdrstop

#include "cwindows.h"
#include "consysop.h"
#include "config.h"
#include "extmsg.h"


static Bool kbmHandler(EVENT evt, long param, int more, CITWINDOW *wnd)
	{
	switch (evt)
		{
		case EVT_DESTROY:
			{
			if (wnd == KeyboardBufferMonitor)
				{
				KeyboardBufferMonitor = NULL;
				}

			break;
			}

		case EVT_CHECKNEWEXT:
			{
			if (defaultHandler(evt, param, more, wnd))
				{
				SRECT *rect = (SRECT *) param;

				if (rect->bottom - rect->top != 3)
					{
					rect->bottom = rect->top + 3;
					}
				}
			else
				{
				return (FALSE);
				}

			break;
			}

		case EVT_DRAWINT:
			{
			if (buildClipArray(wnd))
				{
				char Prompt[80];
				const int WindowWidth = wnd->extents.right -
						wnd->extents.left - 1;
				int i;

				if (KeyboardBuffer.HasInsertedStrings())
					{
					CitWindowOutStr(wnd, 1, 1, getmsg(26), cfg.attr);

					for (i = strlen(getmsg(26)) + 1; i <= WindowWidth; i++)
						{
						CitWindowOutChr(wnd, i, 2, ' ', cfg.attr);
						}

					strcpy(Prompt, getmsg(27));
					}
				else
					{
					char SaveBuffer[KEYBUFSIZE];
					int sbCounter = 0;

					while (!KeyboardBuffer.IsEmpty())
						{
						SaveBuffer[sbCounter++] =
								KeyboardBuffer.Retrieve();
						}

					for (i = 0; i < sbCounter; i++)
						{
						KeyboardBuffer.Add(SaveBuffer[i]);
						}

					sprintf(Prompt, getmsg(267), ltoac(sbCounter));

					for (i = 1; i <= WindowWidth; i++)
						{
						CitWindowOutChr(wnd, i, 1,
								i <= sbCounter ? SaveBuffer[i - 1] : ' ',
								cfg.attr);
						}
					}

				CitWindowOutStr(wnd, 1, 2, Prompt, cfg.attr);

				for (i = strlen(Prompt) + 1; i <= WindowWidth; i++)
					{
					CitWindowOutChr(wnd, i, 2, ' ', cfg.attr);
					}

				freeClipArray();
				}

			break;
			}

		default:
			{
			return (defaultHandler(evt, param, more, wnd));
			}
		}

	return (TRUE);
	}

static Bool spmHandler(EVENT evt, long param, int more, CITWINDOW *wnd)
	{
	switch (evt)
		{
		case EVT_DESTROY:
			{
			if (wnd == SerialPortMonitor)
				{
				SerialPortMonitor = NULL;
				}

			break;
			}

		case EVT_CHECKNEWEXT:
			{
			if (defaultHandler(evt, param, more, wnd))
				{
				SRECT *rect = (SRECT *) param;

				if (rect->bottom - rect->top < 3)
					{
					rect->bottom = rect->top + 3;
					}

				const int i = strlen(getmsg(265)) + 2;

				if (rect->right - rect->left < i)
					{
					rect->right = rect->left + i;
					}
				}
			else
				{
				return (FALSE);
				}

			break;
			}

		case EVT_DRAWINT:
			{
			if (buildClipArray(wnd))
				{
				const int WindowHeight = wnd->extents.bottom - wnd->extents.top - 1;
				const int WindowWidth = wnd->extents.right - wnd->extents.left - 1;

				const int SentHalf = WindowHeight / 2;
				const int RcvdHalf = WindowHeight - SentHalf;

				// Sent...
				int OnLine = 1;
				CitWindowOutStr(wnd, 1, OnLine, getmsg(265), cfg.attr);

				int i = strlen(getmsg(265));

				int BDispChars = (WindowWidth - i) * SentHalf;

				i++;

				int BufferPos = CommPort->GetNumInSentBuffer() -
						BDispChars + 1;

				BufferPos = max(BufferPos, 1);

				for (int j = 0; j < BDispChars; i++, BufferPos++, j++)
					{
					CitWindowOutChr(wnd, i, OnLine, 
							BufferPos <= CommPort->GetNumInSentBuffer() ?
							CommPort->GetFromSentBuffer(BufferPos) : ' ',
							cfg.attr);

					if (i >= WindowWidth)
						{
						OnLine++;

						i = strlen(getmsg(265));

						for (int k = 1; k <= i; k++)
							{
							CitWindowOutChr(wnd, k, OnLine, ' ', cfg.attr);
							}
						}
					}

				// Received...
				OnLine = 1 + SentHalf;
				CitWindowOutStr(wnd, 1, OnLine, getmsg(259), cfg.attr);

				i = strlen(getmsg(259));

				BDispChars = (WindowWidth - i) * RcvdHalf;

				i++;

				BufferPos = CommPort->GetNumInReceivedBuffer() -
						BDispChars + 1;

				BufferPos = max(BufferPos, 1);

				for (int j = 0; j < BDispChars; i++, BufferPos++, j++)
					{
					CitWindowOutChr(wnd, i, OnLine,
							BufferPos <= CommPort->GetNumInReceivedBuffer() ?
							CommPort->GetFromReceivedBuffer(BufferPos) : ' ',
							cfg.attr);

					if (i >= WindowWidth && j < BDispChars - 1)
						{
						OnLine++;

						i = strlen(getmsg(259));

						for (int k = 1; k <= i; k++)
							{
							CitWindowOutChr(wnd, k, OnLine, ' ', cfg.attr);
							}
						}
					}

				freeClipArray();
				}

			break;
			}

		default:
			{
			return (defaultHandler(evt, param, more, wnd));
			}
		}

	return (TRUE);
	}

static Bool cmHandler(EVENT evt, long param, int more, CITWINDOW *wnd)
	{
	switch (evt)
		{
		case EVT_DESTROY:
			{
			if (wnd == CronMonitor)
				{
				CronMonitor = NULL;
				}

			break;
			}

		case EVT_CHECKNEWEXT:
			{
			if (defaultHandler(evt, param, more, wnd))
				{
				SRECT *rect = (SRECT *) param;

				if (rect->bottom - rect->top != 2)
					{
					rect->bottom = rect->top + 2;
					}
				}
			else
				{
				return (FALSE);
				}

			break;
			}

		case EVT_DRAWINT:
			{
			if (buildClipArray(wnd))
				{
				char String[80];

				if (Cron.GetExecutingEvent())
					{
					sprintf(String, getmsg(257),
							Cron.GetExecutingEvent()->Event.GetString());
					}
				else if (Cron.IsReady())
					{
					CopyStringToBuffer(String, getmsg(525));
					}
				else
					{
					time_t Timeout = ((time_t) cfg.idle * 60l) -
							(time(NULL) - Cron.GetStartTime());

					const long Minutes = Timeout / 60;
					const long Seconds = Timeout - (Minutes * 60);

					sprintf(String, getmsg(258), Minutes, Seconds);
					}

				CitWindowOutStr(wnd, 1, 1, String, cfg.attr);

				const int WindowWidth = wnd->extents.right - wnd->extents.left - 1;

				for (int i = strlen(String) + 1; i <= WindowWidth; i++)
					{
					CitWindowOutChr(wnd, i, 1, ' ', cfg.attr);
					}

				freeClipArray();
				}

			break;
			}

		default:
			{
			return (defaultHandler(evt, param, more, wnd));
			}
		}

	return (TRUE);
	}

Bool SysMonKeyHandler(CITWINDOW *wnd, int key)
	{
	switch (key)
		{
		case CSSMM_EXIT:
		case MK_ESC:
			{
			// exit user menu stuff
			return (FALSE);
			}

		case CSSMM_KEYBUF:
			{
			if (KeyboardBufferMonitor)
				{
				destroyCitWindow(KeyboardBufferMonitor, FALSE);
				}
			else
				{
				KeyboardBuffer.SetDirty();

				WINDOWFLAGS flags;
				SRECT rect;

				memset(&flags, 0, sizeof(flags));

				flags.visible = TRUE;
				flags.showTitle = TRUE;
				flags.moveable = TRUE;
				flags.resize = TRUE;
				flags.minimize = TRUE;
				flags.close = TRUE;

				rect.top = 0;
				rect.left = 0;
				rect.bottom = 3;
				rect.right = conCols - 1;

				KeyboardBufferMonitor = makeCitWindow(kbmHandler, NULL,
						getsysmsg(53), flags, rect, NULL, FALSE);
				}

			bringToTop(wnd);

			break;
			}

		case CSSMM_SERPORT:
			{
			if (CitadelIsConfiguring)
				{
				WaitUntilDoneConfiguring();
				break;
				}

			if (SerialPortMonitor)
				{
				destroyCitWindow(SerialPortMonitor, FALSE);
				}
			else
				{
				CommPort->SetDirty();

				WINDOWFLAGS flags;
				SRECT rect;

				memset(&flags, 0, sizeof(flags));

				flags.visible = TRUE;
				flags.showTitle = TRUE;
				flags.moveable = TRUE;
				flags.resize = TRUE;
				flags.minimize = TRUE;
				flags.close = TRUE;

				rect.top = 0;
				rect.left = 0;
				rect.bottom = 3;
				rect.right = conCols - 1;

				SerialPortMonitor = makeCitWindow(spmHandler, NULL,
						getsysmsg(54), flags, rect, NULL, FALSE);
				}

			bringToTop(wnd);

			break;
			}

		case CSSMM_CRON:
			{
			if (CitadelIsConfiguring)
				{
				WaitUntilDoneConfiguring();
				break;
				}

			if (CronMonitor)
				{
				destroyCitWindow(CronMonitor, FALSE);
				}
			else
				{
				WINDOWFLAGS flags;
				SRECT rect;

				memset(&flags, 0, sizeof(flags));

				flags.visible = TRUE;
				flags.showTitle = TRUE;
				flags.moveable = TRUE;
				flags.resize = TRUE;
				flags.minimize = TRUE;
				flags.close = TRUE;

				rect.top = 0;
				rect.left = 0;
				rect.bottom = 2;
				rect.right = conCols - 1;

				CronMonitor = makeCitWindow(cmHandler, NULL,
						getsysmsg(56), flags, rect, NULL, FALSE);
				}

			bringToTop(wnd);

			break;
			}
		}

	return (TRUE);
	}
#endif
