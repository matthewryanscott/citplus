// --------------------------------------------------------------------------
// Citadel: CronFile.CPP
//
// This file contains code to deal with the cron events.

#include "ctdl.h"
#pragma hdrstop

#include "cfgfiles.h"
#include "extmsg.h"
#include "config.h"


#ifdef WINCIT
static const char *ltoac(int wow)
	{
	static label Buffer;
	sprintf(Buffer, "%d", wow);
	return (Buffer);
	}
#endif


// --------------------------------------------------------------------------
// Contents
//
// readcron()		reads CRON.CIT values into Cron class
// writeCrontab()	write cron events to a file
// readCrontab()	read cron events from file

inline void CronC::DumpEvents(void)
	{
	WAITFORm(CronMutex);

	disposeLL((void **) &Events);
	NumEvents = 0;
	OnEvent = NULL;

	RELEASEm(CronMutex);
	}

inline CronEventListS *CronC::AddEvent(void)
	{
	WAITFORm(CronMutex);

	CronEventListS *ToRet = (CronEventListS *) addLL((void **) &Events,
			sizeof(*Events));

	if (ToRet)
		{
		NumEvents++;
		}

	RELEASEm(CronMutex);

	return (ToRet);
	}


// --------------------------------------------------------------------------
// readcron(): Reads CRON.CIT values into events structure 
Bool CronC::ReadCronCit(WC_TW)
	{
	WAITFORm(CronMutex);

	discardable *crdd;
	if ((crdd = readData(9)) == NULL)
		{
		mPrintfCROrUpdateCfgDlg(TW, getmsg(59));

		RELEASEm(CronMutex);
		return (FALSE);
		}

	if (!read_cfg_messages())
		{
		mPrintfCROrUpdateCfgDlg(TW, getmsg(59));
		discardData(crdd);
		
		RELEASEm(CronMutex);
		return (FALSE);
		}

	DumpEvents();

	Bool ToRet;

	char FullPathToCronCit[128];
	sprintf(FullPathToCronCit, sbs, cfg.homepath, citfiles[C_CRON_CIT]);

	FILE *fBuf;
	if ((fBuf = fopen(FullPathToCronCit, FO_R)) != NULL)	// ASCII mode
		{
		const char **crontypes = (const char **) crdd->aux;
		const char **cronkeywords = (const char **) crdd->next->aux;
		char line[90];

		CronEventListS *thisCron = NULL;
		int lineno = 0;

		while (fgets(line, sizeof(line), fBuf))
			{
			lineno++;
			if (line[0] != '#')
				{
				continue;
				}

			char *words[256];
			const int count = parse_it(words, line);

			if (count)
				{
				int i;
				for (i = 0; i < CR_NUMCRON; i++)
					{
					if (SameString(words[0] + 1, cronkeywords[i]))
						{
						break;
						}
					}

				switch (i)
					{
					case CR_DAYS:
						{
						if (thisCron)
							{
							// init days
							for (int j = 0; j < NUM_DAYS; j++)
								{
								thisCron->Event.SetDay((DaysE) j, FALSE);
								}
EXTRA_OPEN_BRACE
							for (int j = 1; j < count; j++)
								{
								if (SameString(words[j], getcfgmsg(76))) // ANY
									{
									for (int l = 0; l < NUM_DAYS; l++)
										{
										thisCron->Event.SetDay((DaysE) l, TRUE);
										}
									}
								else
									{
									DaysE theDay = EvaluateDay(words[j]);

									if (theDay != NUM_DAYS)
										{
										thisCron->Event.SetDay(theDay, TRUE);
										}
									else if (words[j][0] == '!')
										{
										theDay = EvaluateDay(words[j] + 1);

										if (theDay != NUM_DAYS)
											{
											thisCron->Event.SetDay(theDay,
													FALSE);
											}
										else
											{
											CRmPrintfCROrUpdateCfgDlg(TW, 
													getcfgmsg(77),
													citfiles[C_CRON_CIT],
													getcfgmsg(78), words[j]);
											}
										}
									}
								}
EXTRA_CLOSE_BRACE
							}

						break;
						}

					case CR_MONTHS:
						{
						if (thisCron)
							{
							// init months
							for (int j = 0; j < NUM_MONTHS; j++)
								{
								thisCron->Event.SetMonth((MonthsE) j, FALSE);
								}
EXTRA_OPEN_BRACE							
							for (int j = 1; j < count; j++)
								{
								if (SameString(words[j], getcfgmsg(76))) // ANY
									{
									for (int l = 0; l < NUM_MONTHS; l++)
										{
										thisCron->Event.SetMonth((MonthsE) l,
												TRUE);
										}
									}
								else
									{
									MonthsE theMonth = EvaluateMonth(words[j]);

									if (theMonth != NUM_MONTHS)
										{
										thisCron->Event.SetMonth(theMonth,
												TRUE);
										}
									else if (words[j][0] == '!')
										{
										theMonth = EvaluateMonth(words[j] + 1);

										if (theMonth != NUM_MONTHS)
											{
											thisCron->Event.SetMonth(theMonth,
													FALSE);
											}
										else
											{
											CRmPrintfCROrUpdateCfgDlg(TW, 
													getcfgmsg(77),
													citfiles[C_CRON_CIT],
													getmsg(585), words[j]);
											}
										}
									}
								}
EXTRA_CLOSE_BRACE							
							}

						break;
						}

					case CR_DO:
						{
						thisCron = AddEvent();

						if (thisCron)
							{
							int k;

							for (k = 0; k < CR_NUMTYPES; k++)
								{
								if (SameString(words[1], crontypes[k]))
									{
									thisCron->Event.SetType((CronTypesE) k);
									break;
									}
								}

							if (k == CR_NUMTYPES)
								{
								mPrintfCROrUpdateCfgDlg(TW, getcfgmsg(109),
										citfiles[C_CRON_CIT], ltoac(lineno),
										words[1]);
								break;
								}

							if (count > 2)
								{
								thisCron->Event.SetString(words[2]);
								}

							thisCron->Event.SetRedo(180);	// 3 hours
							thisCron->Event.SetRetry(10);

							// init hours
							for (k = 0; k < 24; k++ )
								{
								thisCron->Event.SetHour(k, TRUE);
								}

							// init dates
							for (k = 1; k < 32; k++ )
								{
								thisCron->Event.SetDate(k, TRUE);
								}

							// init days
							for (k = 0; k < NUM_DAYS; k++ )
								{
								thisCron->Event.SetDay((DaysE) k, TRUE);
								}

							// init months
							for (k = 0; k < NUM_MONTHS; k++ )
								{
								thisCron->Event.SetMonth((MonthsE) k, TRUE);
								}
							}
						else
							{
							CRmPrintfOrUpdateCfgDlg(TW, getmsg(188),
									citfiles[C_CRON_CIT]);
							}

						break;
						}

					case CR_HOURS:
						{
						if (thisCron)
							{
							// init hours
							for (int j = 0; j < 24; j++)
								{
								thisCron->Event.SetHour(j, FALSE);
								}
EXTRA_OPEN_BRACE							
							for (int j = 1; j < count; j++)
								{
								if (SameString(words[j], getcfgmsg(76))) // ANY
									{
									for (int l = 0; l < 24; l++)
										{
										thisCron->Event.SetHour(l, TRUE);
										}
									}
								else
									{
									const Bool Not = words[j][0] == '!';

									// This "+ Not" looks gross, but is
									// perfectly valid and portable.
									const int hour = atoi(words[j] + Not);

									if (hour > 23 || hour < 0)
										{
										CRmPrintfCROrUpdateCfgDlg(TW, 
												getcfgmsg(82),
												citfiles[C_CRON_CIT], ns,
												hour);
										}
									else
										{
										thisCron->Event.SetHour(hour, !Not);
										}
									}
								}
EXTRA_CLOSE_BRACE
							}

						break;
						}

					case CR_DATES:
						{
						if (thisCron)
							{
							// init dates
							for (int j = 1; j < 32; j++)
								{
								thisCron->Event.SetDate(j, FALSE);
								}
EXTRA_OPEN_BRACE							
							for (int j = 1; j < count; j++)
								{
								if (SameString(words[j], getcfgmsg(76))) // ANY
									{
									for (int l = 1; l < 32; ++l)
										{
										thisCron->Event.SetDate(l, TRUE);
										}
									}
								else
									{
									const Bool Not = words[j][0] == '!';

									// This "+ Not" looks gross, but is
									// perfectly valid and portable.
									const int date = atoi(words[j] + Not);

									if (date > 31 || date < 1)
										{
										CRmPrintfCROrUpdateCfgDlg(TW, 
												getcfgmsg(110),
												citfiles[C_CRON_CIT], ns,
												date);
										}
									else
										{
										thisCron->Event.SetDate(date, !Not);
										}
									}
								}
EXTRA_CLOSE_BRACE							
							}

						break;
						}

					case CR_REDO:
						{
						if (thisCron)
							{
							thisCron->Event.SetRedo(atoi(words[1]));
							}

						break;
						}

					case CR_RETRY:
						{
						if (thisCron)
							{
							thisCron->Event.SetRetry(atoi(words[1]));
							}

						break;
						}

					default:
						{
						mPrintfCROrUpdateCfgDlg(TW, getcfgmsg(109),
								citfiles[C_CRON_CIT], ltoac(lineno),
								words[0]);
						break;
						}
					}
				}
			}

		fclose(fBuf);
		ToRet = TRUE;
		}
	else
		{
		mPrintfCROrUpdateCfgDlg(TW, getmsg(15), FullPathToCronCit);
		ToRet = FALSE;
		}

	SetOnEvent((CronEventListS *) NULL);

	discardData(crdd);
	dump_cfg_messages();
	compactMemory(1);

	RELEASEm(CronMutex);
	return (ToRet);
	}


// --------------------------------------------------------------------------
// writeCrontab(): Write cron events to a file.

Bool CronEventC::Store(FILE *File)
	{
	assert(File);
	assert(this);

	return (fwrite(this, sizeof(*this), 1, File) == 1);
	}

Bool CronC::WriteTable(void)
	{
	WAITFORm(CronMutex);

	char FullPathToCronTab[128];
	sprintf(FullPathToCronTab, sbs, cfg.homepath, cronTab);

	FILE *fp;
	if ((fp = fopen(FullPathToCronTab, FO_WB)) == NULL)
		{
		RELEASEm(CronMutex);
		return (FALSE);
//		crashout(getmsg(8), FullPathToCronTab);
		}

	fwrite(&Pause, sizeof(Pause), 1, fp);

	CronEventListS *theEvent;
	int i;

	for (theEvent = Events, i = 1; theEvent;
			theEvent = (CronEventListS *) getNextLL(theEvent), i++)
		{
		if (theEvent == OnEvent)
			{
			break;
			}
		}

	if (!theEvent)
		{
		i = 1;
		}

	fwrite(&i, sizeof(i), 1, fp);

	for (theEvent = Events; theEvent;
			theEvent = (CronEventListS *) getNextLL(theEvent))
		{
		if (!theEvent->Event.Store(fp))
			{
			fclose(fp);
			unlink(FullPathToCronTab);
			RELEASEm(CronMutex);
			return (FALSE);
			}
		}

	fclose(fp);
	RELEASEm(CronMutex);
	return (TRUE);
	}


// --------------------------------------------------------------------------
// readCrontab(): Read cron events from file.

Bool CronEventC::Load(FILE *File)
	{
	assert(File);
	assert(this);

	return (fread(this, sizeof(*this), 1, File) == 1);
	}

Bool CronC::ReadTable(WC_TW)
	{
	WAITFORm(CronMutex);

	char FullPathToCronTab[128];
	sprintf(FullPathToCronTab, sbs, cfg.homepath, cronTab);

	DumpEvents();

	FILE *fp;
	if ((fp = fopen(FullPathToCronTab, FO_RB)) != NULL)
		{
		fread(&Pause, sizeof(Pause), 1, fp);

		int i;
		fread(&i, sizeof(i), 1, fp);

		CronEventC theEvent;

		while (theEvent.Load(fp))
			{
			CronEventListS *newEvent = AddEvent();

			if (newEvent)
				{
				newEvent->Event = theEvent;
				}
			else
				{
				crashout(getcfgmsg(111));
				}
			}

		fclose(fp);

		unlink(FullPathToCronTab);
		SetOnEvent(GetEventNum(i));
		}
	else
		{
#ifdef WINCIT
		ReadCronCit(TW);
#else
		ReadCronCit();
#endif
		}

	RELEASEm(CronMutex);
	return (TRUE);
	}
